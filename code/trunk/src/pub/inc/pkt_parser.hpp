/*#############################################################################
 * �ļ���   : pkt_parser.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��22��
 * �ļ����� : ���ĸ�����Ϣ����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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