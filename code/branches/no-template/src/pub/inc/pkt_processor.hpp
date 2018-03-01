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
#include <boost/enable_shared_from_this.hpp>

#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "message.hpp"

namespace BroadInter
{

// ���Ĵ���ģ�ͣ�
//=============================================================================
// --->A-------->B ------------>C-------->I
//               |              |
//              \|/            \|/
//			     D--->E--->F    G
//               |
//              \|/
//               H
//=============================================================================
// ���Ĵ�����򣺱��Ĵ����ң����ϵ��µ���������
// 
// ���Ԫ�أ�
// ����ڵ㣺�����Ĵ�������Ҫ���ݴ��������б��Ĵ��ݿ��ƣ�����A��B��C�ȡ�
// �� �� ����������ڵ��һ�����Խṹ������A-B-C-I��D-H�ȡ�
// 
// �������:
// 1) ÿ������ڵ������ĳһ������������D�ڵ�����D-H��������
// 2) ����ڵ���԰����Ӵ�����������B�ڵ����D-H��������
// 3) һ������ڵ�ֻ������һ��������������D�ڵ�����B-D-H��������������E-F��������

// �ӿ�ProcessPkt�ķ���ֵ���ͣ����������߽��б��Ĵ��ݿ��Ƶ�������Ϣ��֧����չ��
typedef uint8 ProcInfo;		

//---------------------------- ����ֵ������Ϣ���� -----------------------------

// PI: Process Information

#define PI_CHAIN_CONTINUE	0x01	// ��0�����ر�ʾ�������Ƿ���Ҫ���������´���
#define PI_L4_CONTINUE		0x02	// ��1�����ر�ʾL4�Ƿ���Ҫ�����ļ������´���
#define PI_L7_CONTINUE		0x04	// ��2�����ر�ʾL7�Ƿ���Ҫ�����ļ������´���

//------------------------------- ����ֵԤ���� --------------------------------

#define PI_RET_STOP		0x00	// ���д���ֹͣ����������

/*******************************************************************************
 * ��  ��: ��Ϣ����������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��11��
 ******************************************************************************/
class PktProcessor : public boost::enable_shared_from_this<PktProcessor>
{
public:
	virtual ~PktProcessor() {}

	virtual void ReInit() {} // TODO: �˴���Ϊ�˼��ݴ������ػ���ugly

	ProcInfo Process(MsgType msg_type, VoidSP msg_data);

	void set_successor(const PktProcessorSP& processor) { successor_ = processor; }
	PktProcessorSP get_successor() const { return successor_; }

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) = 0;

protected:
	PktProcessorSP successor_; // ���δ�����
};


}

#endif