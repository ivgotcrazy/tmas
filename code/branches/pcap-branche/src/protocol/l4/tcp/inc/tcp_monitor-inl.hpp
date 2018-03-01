/*#############################################################################
 * �ļ���   : tcp_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : TcpMonitor��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include "tcp_monitor.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "tmas_config_parser.hpp"
#include "tcp_conn_reorder.hpp"
#include "tcp_conn_analyzer.hpp"
#include "pkt_parser.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
TcpMonitor<Next, Succ>::TcpMonitor() : tcp_switch_(false)
	, remove_inactive_conn_timeout_(30)
{
	GET_TMAS_CONFIG_BOOL("global.protocol.tcp", tcp_switch_);
	if (!tcp_switch_)
	{
		DLOG(WARNING) << "Tcp monitor is closed";
	}

	GET_TMAS_CONFIG_INT("global.tcp.remove-inactive-connection-timeout", 
		                remove_inactive_conn_timeout_);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool TcpMonitor<Next, Succ>::Init()
{
	// �����ڳ�ʼ��TcpMonitor��ʱ�򣬻�δ������Next/Succ����ˣ����뽫
	// ����صĳ�ʼ��С��Ϊ0��ǿ����������ʱ���ɶ��󣬷���Analyzer��
	// ���δ�����������ʧ�ܡ�
	conn_info_pool_.reset(new ObjectPool<TcpConnInfo>(
		boost::bind(&TcpMonitor::ConstructTcpConnInfo, this),
		boost::bind(&TcpMonitor::DestructTcpConnInfo, this, _1),
		boost::bind(&TcpMonitor::ReinitTcpConnInfo, this, _1),
		0));

	tcp_timer_.reset(new FreeTimer(boost::bind(&TcpMonitor::OnTick, this), 5));
	TMAS_ASSERT(tcp_timer_);

	tcp_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����TcpConnInfo
 * ��  ��: 
 * ����ֵ: TcpConnInfo*
 * ��  ��:
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline TcpConnInfo* TcpMonitor<Next, Succ>::ConstructTcpConnInfo()
{
	TcpConnInfo* conn_info = new TcpConnInfo;

	TcpConnReorderTypeSP reorder(new TcpConnReorderType(this));
	TcpConnAnalyzerTypeSP analyzer(new TcpConnAnalyzerType(this));

	reorder->SetSuccProcessor(analyzer);
	analyzer->SetSuccProcessor(this->GetSuccProcessor());

	conn_info->processor = reorder;
	
	return conn_info;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����TcpConnInfo
 * ��  ��: [in] conn_info TcpConnInfo*
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::DestructTcpConnInfo(TcpConnInfo* conn_info)
{
	delete conn_info;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���³�ʼ��TcpConnInfo
 * ��  ��: [in] conn_info TcpConnInfo*
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::ReinitTcpConnInfo(TcpConnInfo* conn_info)
{
	conn_info->abandoned = false;

	TMAS_ASSERT(conn_info->processor);

	conn_info->processor->ProcessMsg(MSG_REINITIALIZE, 0);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::AbandonTcpConnection(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	HashView& hash_view = tcp_conns_.get<0>();
	auto iter = hash_view.find(conn_id);
	if (iter == tcp_conns_.end())
	{
		DLOG(WARNING) << "Fail to abandon connection | " << conn_id;
		return;
	}

	(*iter)->abandoned = true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ر��Ĵ����������رձ���
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::RemoveTcpConnection(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	HashView& hash_view = tcp_conns_.get<0>();
	auto iter = hash_view.find(conn_id);
	if (iter == tcp_conns_.end())
	{
		DLOG(WARNING) << "Fail to remove connection | " << conn_id;
		return;
	}

	// ��֪ͨ�ϲ�
	this->PassMsgToSuccProcessor(MSG_TCP_CONN_CLOSED, &((*iter)->conn_id));

	hash_view.erase(iter); // ��������ɾ��������

	DLOG(WARNING) << "Closed one tcp connection | " << conn_id;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: �����Ƿ���Ҫ��������
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/ 
template<class Next, class Succ>
inline ProcInfo TcpMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	// ��TCP�����ɱ�������������������������
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_TCP)
	{
		return PI_NOT_PROCESSED;
	}

	if (!tcp_switch_) return PI_HAS_PROCESSED;

	if (!ParsePktTcpInfo(pkt_msg))
	{
		DLOG(WARNING) << "Fail to parse tcp info";
		return PI_HAS_PROCESSED;
	}

	PrintTcpInfo(pkt_msg);

	ProcessTcpPkt(pkt_msg);

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����TCP����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::ProcessTcpPkt(PktMsgSP& pkt_msg)
{
	TcpConnInfoSP conn_info;

	// ��ȡ�򴴽�������Ϣ
	if (!GetTcpConnInfo(pkt_msg, conn_info))
	{
		return; // �ڲ������Ѿ��д�ӡ
	}

	TMAS_ASSERT(conn_info); 

	// ��������Ѿ�����������������������
	if (conn_info->abandoned)
	{
		DLOG(WARNING) << "Received packet on abandoned connection";
		return;
	}

	TMAS_ASSERT(conn_info->processor);
	
	// �������д�����
	conn_info->processor->ProcessPkt(pkt_msg);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡTCP����
 * ��  ��: [in] pkt_msg ������Ϣ
 *         [out] conn_info ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline bool TcpMonitor<Next, Succ>::GetTcpConnInfo(const PktMsgSP& pkt_msg, 
	                                               TcpConnInfoSP& conn_info)
{
	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	boost::mutex::scoped_lock lock(conn_mutex_);

	// �Ȳ��������Ƿ��Ѿ�����
	HashView& hash_view = tcp_conns_.get<0>();
	auto hash_iter = hash_view.find(conn_id);
	if (hash_iter != tcp_conns_.end())
	{
		hash_view.modify(hash_iter, LastPktInModifier(time(0)));
		conn_info = *hash_iter;

		DLOG(INFO) << "Tcp connection exists | " << conn_id;
		return true;
	}

	// FIN��RST���Ĳ���������
	if (FIN_FLAG(pkt_msg) || RST_FLAG(pkt_msg)) return false;

	DLOG(INFO) << "Incoming a new tcp connection " << conn_id;
	
	// �Ӷ�����л�ȡ�µ�ConnInfo
	conn_info = conn_info_pool_->AllocObject();
	conn_info->conn_id = conn_id;
	conn_info->last_pkt_in = time(0);
	
	// ����ConnInfo��������
	TcpConnContainer::value_type insert_value(conn_info);
	if (!hash_view.insert(insert_value).second)
	{
		LOG(ERROR) << "Fail to insert connection";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʱ��������ǰ����û�п���Ч�ʣ�������Ҫ�Ż�
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::OnTick()
{
	ClearTimeoutConns();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����ܾö�û���յ����ģ���ɾ������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::ClearTimeoutConns()
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	// ʱ��Ļ�ȡ��������֮�󣬷�����ܳ���ʱ����ת���
	uint32 time_now = time(0);

	TimeView& time_view = tcp_conns_.get<1>();
	for (auto iter = time_view.begin(); iter != time_view.end(); )
	{
		TMAS_ASSERT(time_now >= (*iter)->last_pkt_in);

		if (time_now - (*iter)->last_pkt_in < remove_inactive_conn_timeout_)
		{
			return;
		}

		DLOG(WARNING) << "Removed one timeout connection | " << (*iter)->conn_id;

		// ��֪ͨ�ϲ�
		this->PassMsgToSuccProcessor(MSG_TCP_CONN_CLOSED, &((*iter)->conn_id));

		// �ٴ�������ɾ��
		iter = time_view.erase(iter);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ӡTCP��Ϣ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::PrintTcpInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### TCP || seq: " << SEQ_NUM(pkt_msg) 
		       << " ack: " << ACK_NUM(pkt_msg) 
			   << ", A:"   << ACK_FLAG(pkt_msg) 
			   << "|S:"    << SYN_FLAG(pkt_msg) 
		       << "|R:"    << RST_FLAG(pkt_msg) 
			   << "|F:"    << FIN_FLAG(pkt_msg)
			   << ", data: " << L4_DATA_LEN(pkt_msg);
}

}
