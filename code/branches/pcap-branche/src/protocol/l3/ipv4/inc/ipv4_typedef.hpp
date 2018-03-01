/*#############################################################################
 * �ļ���   : ipv4_typedef.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��02��
 * �ļ����� : IPv4Э���������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IPV4_TYPEDEF
#define BROADINTER_IPV4_TYPEDEF

#include <boost/multi_index/hashed_index.hpp>

#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: IP���ı�ʶ
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��04��02��
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

	uint32 src_ip;	// ԴIP
	uint32 dst_ip;	// Ŀ��IP
	uint16 id;		// ���ı�ʶ
	uint8 prot;		// Э������
};

/*-----------------------------------------------------------------------------
 * ��  ��: ����multi_index_container��hash����
 * ��  ��: [in] pkt_idx ��������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��02��
 *   ���� teck_zhou
 *   ���� ����
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