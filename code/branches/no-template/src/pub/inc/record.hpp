/*#############################################################################
 * �ļ���   : record.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��13��
 * �ļ����� : DatabaseRecorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_RECORD
#define BROADINTER_RECORD

#include "tmas_typedef.hpp"
#include "connection.hpp"

namespace BroadInter
{

struct ConnRecord
{
	ConnId conn_id;	// PK�����ӱ�ʶ
	ptime begin;	// PK�����ӿ�ʼʱ��(����)

	uint32 duration; // ����ʱ��(����)

	uint64 s2b_transfer; // ��С����ϴ�˴����ֽ���
	uint64 b2s_transfer; // �ϴ�����С�˴����ֽ���

	uint64 s2b_speed; // ��С����ϴ�˴����ٶ�
	uint64 b2s_speed; // �ϴ�����С�˴����ٶ�

	uint32 aver_resp_delay;	// ƽ����Ӧʱ��
	
	uint8 record_type; // 0: normal��1: ����ʧ�ܣ�2: ���ӳ�ʱ
};

struct HttpSessionRecord
{
	ConnId conn_id;	 // PK�����ӱ�ʶ
	std::string url; // PK����Դ��ʶ
	ptime begin;	 // PK���Ự��ʼʱ��
	
	uint32 duration; // �Ựʱ��

	uint32 resp_delay;	   // ��Ӧʱ��
	uint32 download_speed; // �����ٶ�

	uint8 record_type; // 0: normal��1: ��Ӧ��ʱ
};

}

#endif
