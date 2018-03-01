/*#############################################################################
 * 文件名   : pkt_parser.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月22日
 * 文件描述 : 报文各层信息解析
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_PARSER
#define BROADINTER_PKT_PARSER

#include "message.hpp"

namespace BroadInter
{

bool ParsePktEthInfo(PktMsg* pkt_msg);

bool ParsePktIpInfo(bool enable_checksum, PktMsg* pkt_msg);

bool ParsePktTcpInfo(PktMsg* pkt_msg);

bool ParsePktHttpInfo(PktMsg* pkt_msg);

bool ParsePktUdpInfo(PktMsg* pkt_msg);

}

#endif