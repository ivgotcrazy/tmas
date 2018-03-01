/*#############################################################################
 * 文件名   : tmas_cfg.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月26日
 * 文件描述 : TMAS工程配置
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_CFG
#define BROADINTER_TMAS_CFG

// GTEST测试宏
//#ifndef TMAS_GTEST
//#define TMAS_GTEST
//#endif

// PCAP捕获报文的最小和最大长度
#define PCAP_PKT_MAX_SIZE	1518
#define PCAP_PKT_MIN_SIZE	42

// 报文分发器中缓冲队列缓存报文个数
#define PKT_QUEUE_MAX_SIZE	128

// ETH报文的最小和最大长度
#define MAX_ETH_PKT_LEN		1514	// 1500 + 14
#define MIN_ETH_PKT_LEN		54		// 14 + 20 + 20 TODO: 这里有问题

#define IP_HEADER_MIN_LEN	20
#define MAX_IP_PKT_LEN		65535

#endif