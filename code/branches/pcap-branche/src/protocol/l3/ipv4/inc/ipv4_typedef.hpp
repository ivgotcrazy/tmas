/*#############################################################################
 * 文件名   : ipv4_typedef.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月02日
 * 文件描述 : IPv4协议相关声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IPV4_TYPEDEF
#define BROADINTER_IPV4_TYPEDEF

#include <boost/multi_index/hashed_index.hpp>

#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: IP报文标识
 * 作  者: teck_zhou
 * 时  间: 2014年04月02日
 ******************************************************************************/
struct Ipv4PktIdx
{
	Ipv4PktIdx() : src_ip(0), dst_ip(0), id(0), prot(0) {}

	Ipv4PktIdx(uint8 protocol, uint32 src, uint32 dst, uint16 seq) 
		: src_ip(src)
		, dst_ip(dst)
		, id(seq)
		, prot(protocol) {}

	Ipv4PktIdx(const PktMsgSP& pkt_msg)
		: src_ip(pkt_msg->l3_pkt_info.src_ip)
		, dst_ip(pkt_msg->l3_pkt_info.dst_ip)
		, id(pkt_msg->l3_pkt_info.pkt_id)
		, prot(pkt_msg->l2_pkt_info.l3_prot) {}

	bool operator==(const Ipv4PktIdx& pkt_idx) const
	{
		return (src_ip == pkt_idx.src_ip) 
			&& (dst_ip == pkt_idx.dst_ip)
			&& (id == pkt_idx.id) 
			&& (prot == pkt_idx.prot);
	}

	uint32 src_ip;	// 源IP
	uint32 dst_ip;	// 目的IP
	uint16 id;		// 报文标识
	uint8 prot;		// 协议类型
};

/*-----------------------------------------------------------------------------
 * 描  述: 适配multi_index_container的hash函数
 * 参  数: [in] pkt_idx 报文索引
 * 返回值: 
 * 修  改:
 *   时间 2014年04月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
inline std::size_t hash_value(const Ipv4PktIdx& pkt_idx)
{
	size_t seed = 0;
	boost::hash_combine(seed, boost::hash_value(pkt_idx.prot));
	boost::hash_combine(seed, boost::hash_value(pkt_idx.src_ip));
	boost::hash_combine(seed, boost::hash_value(pkt_idx.dst_ip));
	boost::hash_combine(seed, boost::hash_value(pkt_idx.id));

	return seed;
}

}

#endif