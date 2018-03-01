/*#############################################################################
 * �ļ���   : tcp_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : TcpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_MONITOR
#define BROADINTER_TCP_MONITOR

#include <list>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "timer.hpp"
#include "tcp_typedef.hpp"
#include "object_pool.hpp"

namespace BroadInter
{

using namespace boost::multi_index;

/*******************************************************************************
 * ��  ��: TCP������Ϣ���˶������һ��TCP����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
struct TcpConnInfo
{
	TcpConnInfo() : abandoned(false) {}

	ConnId conn_id;					// ���ӱ�ʶ
	bool   abandoned;				// �����Ƿ��Ѿ�����������������ٴ�����
	uint32 first_pkt_in;			// ���ӵ�һ�����ĵ���ʱ��
	uint32 last_pkt_in;				// �������һ�����Ĵﵽʱ��
	TcpConnReorderTypeSP processor;	// ���Ĵ�����
};

typedef boost::shared_ptr<TcpConnInfo> TcpConnInfoSP;

/*******************************************************************************
 * ��  ��: TCP���Ĵ�����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
template<class Next, class Succ>
class TcpMonitor : public PktProcessor<TcpMonitor<Next, Succ>, Next, Succ>
{
public:
	TcpMonitor();

	bool Init();

	//--- TODO: ���Ż���������

	// ������Ϣ����
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// �ṩ�����Ӵ������ص��������쳣��ֹ
	inline void AbandonTcpConnection(const ConnId& conn_id);

	// �ṩ�����Ӵ������ص������������ر�
	inline void RemoveTcpConnection(const ConnId& conn_id);

	// ����/����/���³�ʼ�����Ӵ����������֧�ֳػ�
	inline TcpConnInfo* ConstructTcpConnInfo();
	inline void DestructTcpConnInfo(TcpConnInfo* conn_info);
	inline void ReinitTcpConnInfo(TcpConnInfo* conn_info);

private:
	
	//--------------------------------------------------------------------------

	typedef multi_index_container<
		TcpConnInfoSP,
		indexed_by<
			hashed_unique<
				member<TcpConnInfo, ConnId, &TcpConnInfo::conn_id>
			>,
			ordered_non_unique<
				member<TcpConnInfo, uint32, &TcpConnInfo::last_pkt_in>
			>
		> 
	>TcpConnContainer;
	typedef TcpConnContainer::nth_index<0>::type HashView;
	typedef TcpConnContainer::nth_index<1>::type TimeView;

	//--------------------------------------------------------------------------

	struct LastPktInModifier
	{
		LastPktInModifier(uint32 time_new) : new_time(time_new) {}

		void operator()(TcpConnInfoSP& conn_info)
		{
			conn_info->last_pkt_in = new_time;
		}

		uint32 new_time;
	};

	//--------------------------------------------------------------------------

	struct TcpMonitorStat
	{
		TcpMonitorStat() : closed_conn_for_handshake_fail(0)
			, closed_conn_for_cached_too_many_pkt(0) {}

		uint64 closed_conn_for_handshake_fail;
		uint64 closed_conn_for_cached_too_many_pkt;
	};
	
private:
	// �����ʱ���Ӷ�ʱ������
	inline void OnTick();

	// ����TCP����
	inline void ProcessTcpPkt(PktMsgSP& pkt_msg);

	// ��ȡTCP���Ӵ�����Ϣ�ṹ
	inline bool GetTcpConnInfo(const PktMsgSP& pkt_msg, TcpConnInfoSP& conn_info);

	// ��ӡ���ĵ�TCP����Ϣ
	inline void PrintTcpInfo(const PktMsgSP& pkt_msg);

	// �����ʱ����
	inline void ClearTimeoutConns();

private:
	// tcpЭ�鿪��
	bool tcp_switch_;

	// ��������TCP����
	TcpConnContainer tcp_conns_;
	boost::mutex conn_mutex_;

	// ����ʱ���Ӷ�ʱ��
	boost::scoped_ptr<FreeTimer> tcp_timer_;

	// ͳ����Ϣ
	TcpMonitorStat tcp_monitor_stat_;

	// �����趨ʱ��δ�յ��κα��ģ�����Ϊ���ӳ�ʱ
	uint32 remove_inactive_conn_timeout_;

	// ���Ӵ�������
	boost::shared_ptr<ObjectPool<TcpConnInfo> > conn_info_pool_;
};

}

#include "tcp_monitor-inl.hpp"

#endif