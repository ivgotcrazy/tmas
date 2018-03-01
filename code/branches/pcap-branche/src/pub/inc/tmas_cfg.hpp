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

// ���ܲ��Ժ꿪��
#ifndef TMAS_PERF_PROFILE
#define TMAS_PERF_PROFILE
#endif

// PCAP�����ĵ���С����󳤶�
#define PCAP_PKT_MAX_SIZE		9 * 1024	// jumbo frame
#define PCAP_PKT_MIN_SIZE		42

#define COMMON_FRAME_MAX_SIZE	1518
#define JUMBO_FRAME_MAX_SIZE	PCAP_PKT_MAX_SIZE

// ���ķַ����л������Ĭ�ϻ��汨�ĸ���
#define PKT_QUEUE_DEFAULT_SIZE	128

// ETH���ĵ���С����󳤶�
#define MAX_ETH_PKT_LEN			PCAP_PKT_MAX_SIZE
#define MIN_ETH_PKT_LEN			PCAP_PKT_MIN_SIZE

#define UDP_HDR_LEN				16
#define IP_HEADER_MIN_LEN		20
#define MAX_IP_PKT_LEN			65535

#define GLOBAL_CONFIG_PATH 		"../config/tmas.conf"
#define HTTP_CONFIG_PATH		"../config/http.conf"

#endif
