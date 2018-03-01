/*#############################################################################
 * �ļ���   : tcp_conn_reorder.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��09��
 * �ļ����� : TcpConnReorder��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_TCP_CONN_REORDER_INL
#define BROADINTER_TCP_CONN_REORDER_INL

#include <glog/logging.h>

#include "tcp_conn_reorder.hpp"
#include "tmas_assert.hpp"
#include "pkt_resolver.hpp"
#include "tcp_monitor.hpp"
#include "tmas_cfg.hpp"
#include "tmas_config_parser.hpp"
#include "pkt_dispatcher.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] tcp_monitor ����TcpMonitor
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
TcpConnReorder<Next, Succ>::TcpConnReorder(TcpMonitorType* tcp_monitor)
	: tcp_monitor_(tcp_monitor), conn_closed_(false)
{
	GET_TMAS_CONFIG_INT("global.tcp.max-cached-unordered-pkt", 
		                max_cached_unordered_pkt_);

	if (max_cached_unordered_pkt_ > 1024 * 1024)
	{
		max_cached_unordered_pkt_ = 16;

		LOG(WARNING) << "Invalid max-cached-unordered-pkt value " 
			         << max_cached_unordered_pkt_;
	}

	GET_TMAS_CONFIG_INT("global.tcp.max-cached-pkt-before-handshake", 
		                max_cached_pkt_before_handshake_);

	if (max_cached_pkt_before_handshake_ > 1024 * 1024)
	{
		max_cached_pkt_before_handshake_ = 16;

		LOG(WARNING) << "Invalid max_cached_pkt_before_handshake_ value " 
			         << max_cached_pkt_before_handshake_;
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
TcpConnReorder<Next, Succ>::~TcpConnReorder()
{
	// �ͷ����򻺴�ı���
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���³�ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::Reinitialize()
{
	conn_closed_ = false;
	conn_info_.started = false;

	ReInitHalfConn(conn_info_.half_conn[PKT_DIR_S2B]);
	ReInitHalfConn(conn_info_.half_conn[PKT_DIR_B2S]);	
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���³�ʼ����������Ϣ
 * ��  ��: [in] half_conn ������
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::ReInitHalfConn(HalfConnInfo& half_conn)
{
	half_conn.next_ack_num = 0;
	half_conn.next_seq_num = 0;
	half_conn.sent_ack_num = 0;
	half_conn.sent_seq_num = 0;
	half_conn.send_pkt_num = 0;

	half_conn.cached_pkt.clear();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��Ϣ����
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo TcpConnReorder<Next, Succ>::DoProcessMsg(MsgType msg_type, void* data)
{
	if (msg_type == MSG_REINITIALIZE)
	{
		Reinitialize();
	}

	// ��������Analyzer����
	this->PassMsgToSuccProcessor(msg_type, data);

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĵ���
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo TcpConnReorder<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	boost::mutex::scoped_lock lock(reorder_mutex_);

	// ���̳߳�����һ���̹߳ر������ӣ������̻߳����ܳ�������
	if (conn_closed_) return PI_HAS_PROCESSED;

	PktOrderType order_type = GetPktOrderType(pkt_msg);
	if (order_type == POT_IN_ORDER)
	{
		TryProcOrderedPkt(pkt_msg);
	}
	else if (order_type == POT_OUT_OF_ORDER)
	{
		AddUnorderedPkt(pkt_msg);
	}
	else if (order_type == POT_RETRANSMIT)
	{
		DLOG(WARNING) << "Tcp retransmission | seq: " << SEQ_NUM(pkt_msg);
	}
	else // POT_IGNORE
	{
		DLOG(WARNING) << "Ignored tcp packet | seq: " << SEQ_NUM(pkt_msg);
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ա��Ľ���Ԥ����ȷ�����ĵ�״̬���Ƿ������Լ��Ƿ�Ϸ�
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ����״̬
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
typename TcpConnReorder<Next, Succ>::PktOrderType 
inline TcpConnReorder<Next, Succ>::GetPktOrderType(const PktMsgSP& pkt_msg)
{
	uint8 direction = pkt_msg->l3_pkt_info.direction;
	TMAS_ASSERT(direction < PKT_DIR_BUTT);

	HalfConnInfo& send_half_conn = conn_info_.half_conn[direction];

	// ���ӵĽ�����δ��ʼ������һ�������ı���Ϊ�������ֵĵ�һ������
	if (!conn_info_.started)
	{
		if (IsFirstPktOf3WHS(pkt_msg))
		{
			return POT_IN_ORDER;
		}
		else
		{
			return POT_OUT_OF_ORDER;
		}
	}

	// �ߵ���˵���������ֵ�һ�������Ѿ����������Ͷ˻�û�з��͹����ģ�
	// ��˵�����Ͷ���server�ˣ���һ�������ı���Ӧ�����������ֵڶ�������
	if (send_half_conn.send_pkt_num == 0)
	{
		return IsSecondPktOf3WHS(pkt_msg) ? POT_IN_ORDER : POT_OUT_OF_ORDER;
	}

	//--- �ߵ��⣬˵���������ֵ�ǰ���α��Ľ������Ѿ����

	// TODO: ��ʵ�ϣ�seq_num�ǿ��ܺ��˵ģ������ڹر�nagle�㷨�������ش�ʱ��
	// һ����˵���ش����¾ɱ���֮������Ӧ����һ�µģ�����Ҳ���ܾ��Ա�֤��
	// ����Ϊ�˱��ִ���ļ򵥣��������ش����ģ����������Ż���
	if (SEQ_NUM(pkt_msg) < send_half_conn.next_seq_num)
	{
		// ��ACK���ģ�������򣬲�����Ӱ�����ӵ��ؽ�
		if (L4_DATA_LEN(pkt_msg) == 0 && ACK_FLAG(pkt_msg) 
			&& !SYN_FLAG(pkt_msg) && !FIN_FLAG(pkt_msg) && !RST_FLAG(pkt_msg))
		{
			return POT_IGNORE;
		}
		else
		{
			return POT_RETRANSMIT;
		}
	}
	else if (SEQ_NUM(pkt_msg) > send_half_conn.next_seq_num)
	{
		return POT_OUT_OF_ORDER;
	}
	else // SEQ_NUM(pkt_msg) == send_conn.next_seq_num
	{
		if (ACK_NUM(pkt_msg) > send_half_conn.next_ack_num)
		{
			return POT_OUT_OF_ORDER;
		}
		else
		{
			return POT_IN_ORDER;
		}
	}
} 

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ����������ֵĵ�һ�����ġ�3WHS: 3 way handshake
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline bool TcpConnReorder<Next, Succ>::IsFirstPktOf3WHS(const PktMsgSP& pkt_msg)
{
	return pkt_msg->l4_pkt_info.l4_data_len == 0
		&& pkt_msg->l4_pkt_info.syn_flag
		&& !pkt_msg->l4_pkt_info.ack_flag;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ����������ֵĵڶ������ġ�3WHS: 3 way handshake
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline bool TcpConnReorder<Next, Succ>::IsSecondPktOf3WHS(const PktMsgSP& pkt_msg)
{
	return pkt_msg->l4_pkt_info.l4_data_len == 0
		&& pkt_msg->l4_pkt_info.syn_flag
		&& pkt_msg->l4_pkt_info.ack_flag;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������
 * ��  ��: [in] conn_info ������Ϣ
 *         [in] pkt_msg ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::AddUnorderedPkt(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Add unordered tcp packet";

	TMAS_ASSERT(SEND_DIR < PKT_DIR_BUTT);
	HalfConnInfo& send_half_conn = conn_info_.half_conn[SEND_DIR];

	// ������ô�౨�Ļ�δ��ʼ���֣�ɾ������
	if (!conn_info_.started 
		&& (GetTotalCachedPktNum() > max_cached_pkt_before_handshake_))
	{
		LOG(WARNING) << "Cached too many packets before handshake";
		return HandshakeFailed(pkt_msg);
	}
	
	// �����Ĳ��뵽���汨���б�
	InsertUnorderedPktToList(pkt_msg, send_half_conn.cached_pkt);

	// �������������ĳ������ƣ������Ƕ������£�����ʱ��Ҫ�˹���Ԥ
	// ���ӵ����У�ͨ����Ϊ��������״̬������ʹ�û��汨�ı�Ϊ����
	if (send_half_conn.cached_pkt.size() > max_cached_unordered_pkt_)
	{
		LOG(WARNING) << "Refactor tcp connection ";
		return RefactorTcpConn();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ�ܻ��汨�ĸ���
 * ��  ��: 
 * ����ֵ: ���汨�ĸ���
 * ��  ��:
 *   ʱ�� 2014��03��28��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline uint32 TcpConnReorder<Next, Succ>::GetTotalCachedPktNum()
{
	HalfConnInfo& s2b_half = conn_info_.half_conn[PKT_DIR_S2B];
	HalfConnInfo& b2s_half = conn_info_.half_conn[PKT_DIR_B2S];

	return s2b_half.cached_pkt.size() + b2s_half.cached_pkt.size();
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ع�TCP���ӡ�ż���Զ������ܵ���TCP�����������жϣ�Ϊ�����ϵͳ�ݴ�
 *         �ԣ���Ҫ�Զ��ع����ӣ�����ż���Զ������µ������жϡ�
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��03��28��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::RefactorTcpConn()
{
	MakeChacedPktOrdered();

	TryProcCachedPkt();
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ع�TCP���ӡ�ż���Զ������ܵ���TCP�����������жϣ�Ϊ�����ϵͳ�ݴ�
 *         �ԣ���Ҫ�Զ��ع����ӣ�����ż���Զ������µ������жϡ�
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��03��28��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::MakeChacedPktOrdered()
{
	HalfConnInfo& s2b_half = conn_info_.half_conn[PKT_DIR_S2B];
	HalfConnInfo& b2s_half = conn_info_.half_conn[PKT_DIR_B2S];

	if (!s2b_half.cached_pkt.empty())
	{
		if (s2b_half.next_seq_num < SEQ_NUM(s2b_half.cached_pkt.front()))
		{		
			LOG(WARNING) << "S2B missing tcp data length " 
				<< SEQ_NUM(s2b_half.cached_pkt.front()) - s2b_half.next_seq_num;

			s2b_half.next_seq_num = SEQ_NUM(s2b_half.cached_pkt.front());
		}
		
		if (ACK_NUM(s2b_half.cached_pkt.front()) > s2b_half.next_ack_num)
		{
			s2b_half.next_ack_num = ACK_NUM(s2b_half.cached_pkt.front());
		}
	}

	if (!b2s_half.cached_pkt.empty())
	{
		if (b2s_half.next_seq_num < SEQ_NUM(b2s_half.cached_pkt.front()))
		{
			LOG(WARNING) << "B2S missing tcp data length " 
				<< SEQ_NUM(b2s_half.cached_pkt.front()) - b2s_half.next_seq_num;

			b2s_half.next_seq_num = SEQ_NUM(b2s_half.cached_pkt.front());	
		}

		if (SEQ_NUM(b2s_half.cached_pkt.front()) > b2s_half.next_ack_num)
		{
			b2s_half.next_ack_num = SEQ_NUM(b2s_half.cached_pkt.front());
		}
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ĳ��뵽����
 * ��  ��: [in] pkt_msg ������Ϣ
 *         [out] pkt_list ���Ķ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::InsertUnorderedPktToList(
	const PktMsgSP& pkt_msg, PktMsgList& pkt_list)
{
	auto iter = pkt_list.begin();
	for (; iter != pkt_list.end(); ++iter)
	{
		// ���ȸ��ݷ����������
		if (SEQ_NUM(pkt_msg) < SEQ_NUM(*iter))
		{
			break;
		}
		else if (SEQ_NUM(pkt_msg) == SEQ_NUM(*iter))
		{
			// ���������ȣ������ȷ���������
			if (ACK_NUM(pkt_msg) < ACK_NUM(*iter))
			{
				break;
			}
			else if (ACK_NUM(pkt_msg) == ACK_NUM(*iter))
			{
				// ����������Ķ�Я�����ݣ���seq_num��ack_num��ȣ���϶����ش���
				if ((L4_DATA_LEN(pkt_msg) > 0 && L4_DATA_LEN(*iter) > 0))
				{
					DLOG(WARNING) << "Unordered tcp retransmission with data | "
						<< (*iter)->l4_pkt_info << "->" << pkt_msg->l4_pkt_info;
					return;
				}
				// �����ûЯ�����ݣ���FIN��SYN��־Ҳ��ͬ����Ҳ�������ش���
				else if (L4_DATA_LEN(pkt_msg) == 0 && L4_DATA_LEN(*iter) == 0)
				{
					if (FIN_FLAG(pkt_msg) == FIN_FLAG(*iter) 
						&& SYN_FLAG(pkt_msg) == SYN_FLAG(*iter))
					{
						DLOG(WARNING) << "Unordered tcp retransmission without data | "
							<< (*iter)->l4_pkt_info << "->" << pkt_msg->l4_pkt_info;
						return;
					}
					else
					{
						// δ�ñ�־λ������ǰ��(SYN��FINֻ������һ)
						if (!SYN_FLAG(pkt_msg) && !FIN_FLAG(pkt_msg))
						{
							break;
						}
					}
				}
				else // �ߵ��⣬��϶���һ�����ĵĳ���Ϊ0����һ�����ĳ��Ȳ�Ϊ0
				{
					// ���Ȱ������ݳ�������payloadΪ0�ı�������ǰ��
					if (L4_DATA_LEN(pkt_msg) < L4_DATA_LEN(*iter))
					{
						TMAS_ASSERT(L4_DATA_LEN(pkt_msg) == 0);
						break;
					}
					// ���ݳ���һ�£�����SYN/FIN��־λ����
					else
					{
						; // ���򣬼���Ѱ�ҷ��ͺź�ȷ�ϺŶ���ȵı���
					}
				}
			}
			else
			{
				; // ����Ѱ�ҷ��ͺ��������£�
				  // ��һ��ȷ�ϺŴ��ڵ�ǰȷ�Ϻŵı���
			}
		}
		else
		{
			; // ����Ѱ�ҵ�һ��������Ŵ��ڵ�ǰ������ŵı���λ
		}
	}

	pkt_list.insert(iter, pkt_msg);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������ģ�������󣬿��»��汨���Ƿ�Ҳ������
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::TryProcOrderedPkt(PktMsgSP& pkt_msg)
{
	ProcessOrderedPkt(pkt_msg);

	TryProcCachedPkt();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����������
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::ProcessOrderedPkt(PktMsgSP& pkt_msg)
{
	RefreshTcpConn(pkt_msg);

	// �����Ĵ��ݸ�TcpConnAnalyzer����
	this->PassPktToSuccProcessor(pkt_msg);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������ӵĻ��汨��
 * ��  ��: [in][out] pkt_list ���汨�Ķ���
 *         [in][out] pkt_iter ������������λ��
 *         [in][out] local_half_flag �������ӿ��Ʊ�־
 *         [out] other_half_flag ������ӿ��Ʊ�־
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��28��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::ProcHalfConnCachedPkt(PktMsgList& pkt_list, 
	PktMsgIter& pkt_iter, bool& local_half_flag, bool& other_half_flag)
{
	if (pkt_iter == pkt_list.end())
	{
		local_half_flag = false;
	}
	else
	{
		if (!ProcOneCachedPkt(pkt_list, pkt_iter))
		{
			local_half_flag = false;
		}
		else
		{
			// ���˴�����һ�������ĺ󣬿�����һ��
			// ���汨�ı��������Ҫ������һ�˶���
			other_half_flag = true;
		}
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����汨��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::TryProcCachedPkt()
{
	PktMsgList& s2b_list = conn_info_.half_conn[PKT_DIR_S2B].cached_pkt;
	PktMsgList& b2s_list = conn_info_.half_conn[PKT_DIR_B2S].cached_pkt;

	auto s2b_iter = s2b_list.begin(); 
	auto b2s_iter = b2s_list.begin();

	bool s2b_proc_flag = true;
	bool b2s_proc_flag = true;

	// �����л����д��������ģ�����������ж��Ѿ�û�����������˳�
	while (s2b_proc_flag || b2s_proc_flag)
	{
		if (s2b_proc_flag)
		{
			ProcHalfConnCachedPkt(s2b_list, s2b_iter, s2b_proc_flag, b2s_proc_flag);
		}

		if (b2s_proc_flag)
		{
			ProcHalfConnCachedPkt(b2s_list, b2s_iter, b2s_proc_flag, s2b_proc_flag);
		}
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����һ�����汨��
 * ��  ��: [in] pkt_list �����б�
 *         [in][out] pkt_iter �����α�
 * ����ֵ: true: ������߷Ƿ�
 *         false: ����
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline bool TcpConnReorder<Next, Succ>::ProcOneCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter)
{
	PktOrderType order_type = GetPktOrderType(*pkt_iter);
	if (order_type == POT_IN_ORDER)
	{
		ProcessOrderedPkt(*pkt_iter);

		pkt_iter = pkt_list.erase(pkt_iter);
	}
	else if (order_type == POT_OUT_OF_ORDER)
	{
		return false;
	}
	else // retransmit or ignore
	{
		DLOG(WARNING) << "Invalid cached packet | " << order_type;

		pkt_iter = pkt_list.erase(pkt_iter);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ˢ��TCP���������Ϣ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::RefreshTcpConn(const PktMsgSP& pkt_msg)
{
	// ����ֻ�ᴦ�������ģ�������ζ���Ҫˢ�´˱�־
	if (!conn_info_.started)
	{
		conn_info_.started = true;
	}

	uint8 direction = pkt_msg->l3_pkt_info.direction;
	TMAS_ASSERT(direction < PKT_DIR_BUTT);

	HalfConnInfo& send_half_conn = conn_info_.half_conn[direction];
	HalfConnInfo& recv_half_conn = conn_info_.half_conn[(direction + 1) % 2];

	// ���·��Ͷ����Ӳ���

	send_half_conn.sent_seq_num = pkt_msg->l4_pkt_info.seq_num;
	send_half_conn.sent_ack_num = pkt_msg->l4_pkt_info.ack_num;

	send_half_conn.next_seq_num = send_half_conn.sent_seq_num + L4_DATA_LEN(pkt_msg);
	
	// SYN��FIN��ռ��һ���������
	if (SYN_FLAG(pkt_msg) || FIN_FLAG(pkt_msg))
	{
		send_half_conn.next_seq_num++;
	}

	send_half_conn.send_pkt_num++;

	// ���½��ն����Ӳ���

	recv_half_conn.next_ack_num = send_half_conn.next_seq_num;

	DLOG(INFO) << "TCP send half state | [" 
		       << send_half_conn.sent_seq_num 
			   << ":"
			   << send_half_conn.sent_ack_num
			   << "]["
			   << send_half_conn.next_seq_num
			   << ":"
			   << send_half_conn.next_ack_num
		       << "] | " 
			   << send_half_conn.send_pkt_num;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����ʧ�ܴ���
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��28��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnReorder<Next, Succ>::HandshakeFailed(const PktMsgSP& pkt_msg)
{
	// ����ʧ�ܺ����ӻ�ȴ����ձ��ĳ�ʱ���ɾ�������ǣ������л���
	// �ı��Ŀ��Բ��õ�����ɾ�����������
	conn_info_.half_conn[PKT_DIR_S2B].cached_pkt.clear();
	conn_info_.half_conn[PKT_DIR_B2S].cached_pkt.clear();

	tcp_monitor_->AbandonTcpConnection(pkt_msg->l4_pkt_info.conn_id);
}

}

#endif