/*#############################################################################
 * �ļ���   : pkt_processor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : PktProcessor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_PROCESSOR
#define BROADINTER_PKT_PROCESSOR

#include <boost/noncopyable.hpp>

#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ������������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��26��
 ******************************************************************************/
enum ProcInfo
{
	PI_NOT_PROCESSED,	// δ����ָʾ�ϲ㴦��������Ϣ���ݸ�Next����������
	PI_HAS_PROCESSED	// �Ѵ���ָʾ�ϲ㴦����������ɣ�ֱ�ӷ��ؼ���
};

/*******************************************************************************
 * ��  ��: ����Ϣ����������ʾ������սᣬ����Э��������������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��26��
 ******************************************************************************/
class None : public boost::noncopyable
{
public:
	ProcInfo ProcessPkt(PktMsgSP& pkt_msg) { return PI_HAS_PROCESSED; }
	ProcInfo ProcessMsg(MsgType msg_type, void* msg_data) { return PI_HAS_PROCESSED; }

	ProcInfo DoProcessPkt(PktMsgSP& pkt_msg) { return PI_HAS_PROCESSED; }
	ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data) { return PI_HAS_PROCESSED; }
};

/*******************************************************************************
 * ��  ��: ��Ϣ����������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��11��
 ******************************************************************************/
template<class Drived, class Next, class Succ>
class PktProcessor : public boost::noncopyable
{
public:
	// ��������
	virtual ~PktProcessor() {}

	// ��������Ϣ
	inline ProcInfo ProcessPkt(PktMsgSP& pkt_msg);

	// ����Ǳ�����Ϣ
	inline ProcInfo ProcessMsg(MsgType msg_type, void* msg_data);

	// ֧���ֶ����Ʊ��Ĵ��ݵ����δ���������
	inline void PassPktToSuccProcessor(PktMsgSP& pkt_msg);

	// ֧���ֶ�������Ϣ���ݵ����δ���������
	inline void PassMsgToSuccProcessor(MsgType msg_type, void* msg_data);

	// ������һ������
	inline void SetNextProcessor(boost::shared_ptr<Next> processor);

	// ���ü��δ�����
	inline void SetSuccProcessor(boost::shared_ptr<Succ> processor);

	// ��ȡ��һ����������ӿ�
	inline boost::shared_ptr<Next> GetNextProcessor() const;

	// ��ȡ���δ���������ӿ�
	inline boost::shared_ptr<Succ> GetSuccProcessor() const;

private:
	// ��ȡ���������ָ��
	inline Drived* GetDrived();

	// �ṩĬ��ʵ�֣���������Բ�ʵ�ִ˽ӿ�
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// �ṩĬ��ʵ�֣���������Բ�ʵ�ִ˽ӿ�
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data);

private:

	// ��ǰ������δ������Ϣ����Ҫ����Ϣ������ȥ����һ��������
	boost::shared_ptr<Next> next_processor_;

	// ��ǰ��������������Ϣ����Ҫ����Ϣ������ȥ�ļ��δ�����
	boost::shared_ptr<Succ> succ_processor_;
};

}

#include "pkt_processor-inl.hpp"

#endif