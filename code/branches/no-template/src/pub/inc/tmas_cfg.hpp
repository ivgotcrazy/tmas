/*#############################################################################
 * �ļ���   : tmas_cfg.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��26��
 * �ļ����� : TMAS��������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_CFG
#define BROADINTER_TMAS_CFG

// GTEST���Ժ�
//#ifndef TMAS_GTEST
//#define TMAS_GTEST
//#endif

// PCAP�����ĵ���С����󳤶�
#define PCAP_PKT_MAX_SIZE	1518
#define PCAP_PKT_MIN_SIZE	42

// ���ķַ����л�����л��汨�ĸ���
#define PKT_QUEUE_MAX_SIZE	128

// ETH���ĵ���С����󳤶�
#define MAX_ETH_PKT_LEN		1514	// 1500 + 14
#define MIN_ETH_PKT_LEN		54		// 14 + 20 + 20 TODO: ����������

#define IP_HEADER_MIN_LEN	20
#define MAX_IP_PKT_LEN		65535

#endif