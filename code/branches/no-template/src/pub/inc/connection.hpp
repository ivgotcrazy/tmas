/*#############################################################################
 * 文件名   : connection.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月11日
 * 文件描述 : 传输层连接标识
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CONNECTION
#define BROADINTER_CONNECTION

#include <boost/unordered_map.hpp>

#include "tmas_typedef.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 连接定义，较小者与较大者是以IP地址大小来区分。
           考虑到效率，IP和Port都是网络字节序保存的。
 * 作  者: teck_zhou
 * 时  间: 2014年01月11日
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

	uint8  protocol;	 // 协议类型
	uint32 smaller_ip;	 // 较小者IP
	uint32 bigger_ip;	 // 较大者IP
	uint16 smaller_port; // 较小者Port
	uint16 bigger_port;	 // 较大者Port
};

/*-----------------------------------------------------------------------------
 * 描  述: 连接标识的hash函数
 * 参  数: [in] conn_id 连接标识
 * 返回值: hash计算结果
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 打印连接标识
 * 参  数: [in] stream 输出流
 *         [in] conn_id 连接标识
 * 返回值: 输出流
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
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


