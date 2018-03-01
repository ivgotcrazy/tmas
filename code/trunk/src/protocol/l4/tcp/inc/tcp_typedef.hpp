/*#############################################################################
 * 文件名   : tcp_typedef.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月20日
 * 文件描述 : TCP公共类型定义
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_TYPEDEF
#define BROADINTER_TCP_TYPEDEF

#include "tmas_typedef.hpp"
#include "connection.hpp"

namespace BroadInter
{

enum TcpConnRole
{
	TCR_UNKONWN = 0,
	TCR_SERVER  = 1,
	TCR_CLIENT  = 2
};

enum TcpFsmState
{
	TFS_INIT		= 0,
	TFS_LISTEN		= 1,
	TFS_SYN_RCV		= 2,
	TFS_SYN_SENT	= 3,
	TFS_ESTABLISHED	= 4,
	TFS_FIN_WAIT_1	= 5,
	TFS_FIN_WAIT_2	= 6,
	TFS_TIME_WAIT	= 7,
	TFS_CLOSING		= 8,
	TFS_CLOSE_WAIT	= 9,
	TFS_LAST_ACK	= 10,
	TFS_CLOSED		= 11,
	TFS_BUTT		= 12
};

typedef uint8 TcpFsmEvent;

#define TFE_NONE			0x00
#define	TFE_SEND_SYN		0x01
#define	TFE_SEND_ACK		0x02
#define	TFE_SEND_SYN_ACK	0x03
#define	TFE_SEND_FIN		0x04
#define	TFE_SEND_FIN_ACK	0x06
#define	TFE_RECV_SYN		0x10
#define	TFE_RECV_ACK		0x20
#define	TFE_RECV_SYN_ACK	0x30
#define	TFE_RECV_FIN		0x40
#define	TFE_RECV_FIN_ACK	0x60

enum ConnCloseReason
{
	CCR_UNKNOWN,
	CCR_ERROR,
	CCR_RECV_RST,
	CCR_RECV_FIN,
	CCR_HANDSHAKE_FAIL,
	CCR_CACHED_OVERFLOW,
};

#define SEND_DIR	(pkt_msg->l3_pkt_info.direction)
#define RECV_DIR	((SEND_DIR + 1) % 2)

enum TcpConnEvent
{
	TCE_CONN_CLOSE,
	TCE_HS_TIMEOUT,
	TCE_CONN_TIMEOUT,
	TCE_CONN_ABORT
};

struct TcpRecordInfo
{
	ConnId conn_id;
	TcpConnEvent conn_event;

	uint32 hs_delay;
};

class TcpRecorder;
typedef boost::shared_ptr<TcpRecorder> TcpRecorderSP;

class TcpFilter;
typedef boost::shared_ptr<TcpFilter> TcpFilterSP;

}

#endif
