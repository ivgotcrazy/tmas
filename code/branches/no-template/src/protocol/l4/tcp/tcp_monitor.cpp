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

namespace BroadInter
{

#define MAX_ALIVE_TIME_WITHOUT_PKT	30 //��

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] tcp_monitor ����TcpMonitor
 *         [in] pool_size ����ش�С
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
TcpConnInfoPool::TcpConnInfoPool(TcpMonitor* monitor, size_t pool_size /* = EXPAND_SIZE */)
	: tcp_monitor_(monitor)
{
	Expand(pool_size);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
TcpConnInfoSP TcpConnInfoPool::Alloc()
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	if (conn_info_list_.empty())
	{
		Expand();
		DLOG(WARNING) << "Expand tcp connection processors";
	}

	TcpConnInfoSP conn_info = conn_info_list_.front();
	conn_info_list_.pop_front();
	
	// ���³�ʼ��
	conn_info->pkt_processor->ReInit();
	conn_info->pkt_processor->get_successor()->ReInit();

	return conn_info;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ͷŶ���
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void TcpConnInfoPool::Free(const TcpConnInfoSP& conn_info)
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	conn_info_list_.push_back(conn_info);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������ش�С
 * ��  ��: [in] expand_size �����С
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void TcpConnInfoPool::Expand(size_t expand_size /* = EXPAND_SIZE */)
{
	for (size_t i = 0; i < expand_size; i++)
	{
		TcpConnInfoSP conn_info(new TcpConnInfo);

		PktProcessorSP processor(new TcpConnReorder(tcp_monitor_));
		processor->set_successor(PktProcessorSP(new TcpConnAnalyzer(tcp_monitor_)));

		conn_info->pkt_processor = processor;

		conn_info_list_.push_back(conn_info);
	}
}

///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
TcpMonitor::TcpMonitor(const PktProcessorSP& processor) 
	: tcp_switch_(false)
	, remove_inactive_conn_timeout_(30)
	, l4_monitor_(processor)
	, conn_info_pool_(this)
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
bool TcpMonitor::Init()
{
	tcp_timer_.reset(new FreeTimer(boost::bind(&TcpMonitor::OnTick, this), 5));

	TMAS_ASSERT(tcp_timer_);

	tcp_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����Ĵ��ݵ�L7
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void TcpMonitor::TransferPktToL7(const PktMsgSP& pkt_msg)
{
	l4_monitor_->get_successor()->Process(MSG_PKT, VOID_SHARED(pkt_msg));
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
void TcpMonitor::AbandonTcpConnection(const ConnId& conn_id)
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
void TcpMonitor::RemoveTcpConnection(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	HashView& hash_view = tcp_conns_.get<0>();
	auto iter = hash_view.find(conn_id);
	if (iter == tcp_conns_.end())
	{
		DLOG(WARNING) << "Fail to remove connection | " << conn_id;
		return;
	}

	conn_info_pool_.Free(*iter);

	hash_view.erase(conn_id);
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
ProcInfo TcpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	if (!tcp_switch_) // TCP���������عرգ��ɱ���������������������
	{
		return PI_CHAIN_CONTINUE;
	}

	TMAS_ASSERT(msg_type == MSG_PKT);
	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// ��TCP�����ɱ�������������������������
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_TCP)
	{
		return PI_CHAIN_CONTINUE;
	}

	if (!ParsePktTcpInfo(pkt_msg))
	{
		DLOG(WARNING) << "Fail to parse tcp info";
		return PI_RET_STOP;
	}

	PrintTcpInfo(pkt_msg);

	ProcessTcpPkt(pkt_msg); // ������
	
	return PI_RET_STOP; // ����ȫ��ͨ���ֶ����ϲ㴫��
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
void TcpMonitor::ProcessTcpPkt(const PktMsgSP& pkt_msg)
{
	TcpConnInfoSP conn_info;

	// ��ȡ�򴴽�������Ϣ
	if (!GetTcpConnInfo(pkt_msg, conn_info))
	{
		return; // �ڲ������Ѿ��д�ӡ
	}

	TMAS_ASSERT(conn_info);

	// ������ˢ���������һ�����ĵ���ʱ��
	conn_info->last_pkt_in = time(0); 

	// ��������Ѿ�����������������������
	if (conn_info->abandoned)
	{
		DLOG(WARNING) << "Received packet on abandoned connection";
		return;
	}

	TMAS_ASSERT(conn_info->pkt_processor);

	// �������д�����
	conn_info->pkt_processor->Process(MSG_PKT, VOID_SHARED(pkt_msg));
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
bool TcpMonitor::GetTcpConnInfo(const PktMsgSP& pkt_msg, TcpConnInfoSP& conn_info)
{
	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	boost::mutex::scoped_lock lock(conn_mutex_);

	// �Ȳ��������Ƿ��Ѿ�����
	HashView& hash_view = tcp_conns_.get<0>();
	auto hash_iter = hash_view.find(conn_id);
	if (hash_iter != tcp_conns_.end())
	{
		DLOG(INFO) << "Tcp connection exists | " << conn_id;
		conn_info = *hash_iter;
		return true;
	}

	// FIN��RST���Ĳ���������
	if (FIN_FLAG(pkt_msg) || RST_FLAG(pkt_msg)) return false;

	DLOG(INFO) << "Incoming a new tcp connection " << conn_id;
	
	// �Ӷ�����л�ȡ�µ�ConnInfo
	conn_info = conn_info_pool_.Alloc();
	conn_info->conn_id = conn_id;
	
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
void TcpMonitor::OnTick()
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
void TcpMonitor::ClearTimeoutConns()
{
	uint32 time_now = time(0);

	boost::mutex::scoped_lock lock(conn_mutex_);

	TimeView& time_view = tcp_conns_.get<1>();
	for (auto iter = time_view.begin(); iter != time_view.end(); )
	{
		TMAS_ASSERT(time_now >= (*iter)->last_pkt_in);
		if (time_now - (*iter)->last_pkt_in < remove_inactive_conn_timeout_)
		{
			return;
		}

		DLOG(WARNING) << "Removed one timeout connection | " << (*iter)->conn_id;

		conn_info_pool_.Free(*iter);

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
void TcpMonitor::PrintTcpInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### TCP || seq: " << SEQ_NUM(pkt_msg) 
		      << " ack: " << ACK_NUM(pkt_msg) 
			  << ", A:" << ACK_FLAG(pkt_msg) 
			  << "|S:" << SYN_FLAG(pkt_msg) 
		      << "|R:" << RST_FLAG(pkt_msg) 
			  << "|F:" << FIN_FLAG(pkt_msg)
			  << ", data: " << L4_DATA_LEN(pkt_msg);
}

}
