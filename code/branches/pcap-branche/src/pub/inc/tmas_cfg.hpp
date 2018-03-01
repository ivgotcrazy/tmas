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

// 性能测试宏开关
#ifndef TMAS_PERF_PROFILE
#define TMAS_PERF_PROFILE
#endif

// PCAP捕获报文的最小和最大长度
#define PCAP_PKT_MAX_SIZE		9 * 1024	// jumbo frame
#define PCAP_PKT_MIN_SIZE		42

#define COMMON_FRAME_MAX_SIZE	1518
#define JUMBO_FRAME_MAX_SIZE	PCAP_PKT_MAX_SIZE

// 报文分发器中缓冲队列默认缓存报文个数
#define PKT_QUEUE_DEFAULT_SIZE	128

// ETH报文的最小和最大长度
#define MAX_ETH_PKT_LEN			PCAP_PKT_MAX_SIZE
#define MIN_ETH_PKT_LEN			PCAP_PKT_MIN_SIZE

#define UDP_HDR_LEN				16
#define IP_HEADER_MIN_LEN		20
#define MAX_IP_PKT_LEN			65535

#define GLOBAL_CONFIG_PATH 		"../config/tmas.conf"
#define HTTP_CONFIG_PATH		"../config/http.conf"

#endif
