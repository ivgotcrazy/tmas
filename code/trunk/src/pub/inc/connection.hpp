/*#############################################################################
 * �ļ���   : connection.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��11��
 * �ļ����� : ��������ӱ�ʶ
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CONNECTION
#define BROADINTER_CONNECTION

#include <boost/unordered_map.hpp>

#include "tmas_typedef.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���Ӷ��壬��С����ϴ�������IP��ַ��С�����֡�
           ���ǵ�Ч�ʣ�IP��Port���������ֽ��򱣴�ġ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��11��
 ******************************************************************************/
struct ConnId
{
	ConnId()
	{
		protocol = smaller_ip = bigger_ip = smaller_port = bigger_port = 0;
	}

	ConnId(uint8 prot, uint32 ip1, uint16 port1, uint32 ip2, uint16 port2)
	{
		protocol = prot;
		smaller_ip = std::min(ip1, ip2);
		bigger_ip = std::max(ip1, ip2);
		smaller_port = (smaller_ip == ip1) ? port1 : port2;
		bigger_port = (bigger_ip == ip1) ? port1 : port2;
	}

	ConnId& operator=(const ConnId& conn_id)
	{
		protocol = conn_id.protocol;
		smaller_ip = conn_id.smaller_ip;
		bigger_ip = conn_id.bigger_ip;
		smaller_port = conn_id.smaller_port;
		bigger_port = conn_id.bigger_port;

		return *this;
	}

	bool operator==(const ConnId& conn_id) const
	{
		return (conn_id.protocol == protocol)
			&& (conn_id.smaller_ip == smaller_ip)
			&& (conn_id.bigger_ip == bigger_ip)
			&& (conn_id.smaller_port == smaller_port)
			&& (conn_id.bigger_port == bigger_port);
	}

	uint8  protocol;	 // Э������
	uint32 smaller_ip;	 // ��С��IP
	uint32 bigger_ip;	 // �ϴ���IP
	uint16 smaller_port; // ��С��Port
	uint16 bigger_port;	 // �ϴ���Port
};

/*-----------------------------------------------------------------------------
 * ��  ��: ���ӱ�ʶ��hash����
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: hash������
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
inline std::size_t hash_value(const ConnId& conn_id)
{
	std::size_t seed = 0;

	boost::hash_combine(seed, boost::hash_value(conn_id.protocol));
	boost::hash_combine(seed, boost::hash_value(conn_id.smaller_ip));
	boost::hash_combine(seed, boost::hash_value(conn_id.bigger_ip));
	boost::hash_combine(seed, boost::hash_value(conn_id.smaller_port));
	boost::hash_combine(seed, boost::hash_value(conn_id.bigger_port));

	return seed;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ӡ���ӱ�ʶ
 * ��  ��: [in] stream �����
 *         [in] conn_id ���ӱ�ʶ
 * ����ֵ: �����
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Stream>
inline Stream& operator<<(Stream& stream, const ConnId& conn_id)
{
	stream << "[" 
		   << to_string(conn_id.smaller_ip) << ":" << conn_id.smaller_port
		   << "<->" 
		   << to_string(conn_id.bigger_ip) << ":" << conn_id.bigger_port
		   << "]";
	return stream;
}

typedef boost::shared_ptr<ConnId> ConnIdSP;

}

#endif


