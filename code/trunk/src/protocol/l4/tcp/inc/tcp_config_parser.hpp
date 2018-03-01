/*#############################################################################
 * 文件名   : tcp_config_parser.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : TcpConfigParser类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONFIG_PARSER
#define BROADINTER_TCP_CONFIG_PARSER

#include <vector>
#include <boost/noncopyable.hpp>

#include "ip_region_parser.hpp"
#include "port_region_parser.hpp"
#include "config_parser.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: TCP配置域，一个配置域包含过滤行和“{}”中配置部分。
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
struct TcpConfigDomain
{
	TcpConfigDomain()
		: hs_delay_monitor(false)
		, hs_timeout_monitor(false)
		, conn_timeout_monitor(false)
		, conn_abort_monitor(false)
		, hs_delay_min(0) {}

	bool hs_delay_monitor;		// 握手时延监视开关
	bool hs_timeout_monitor;	// 握手超时监视开关
	bool conn_timeout_monitor;	// 连接超时监视开关
	bool conn_abort_monitor;	// 连接异常中断监视开关
	
	uint32 hs_delay_min;		// 时延记录阈值

	IpRegionVec ip_regions;		// IP地址过滤
	PortRegionVec port_regions;	// 端口过滤
};

typedef std::vector<TcpConfigDomain> TcpConfigDomainVec;

/******************************************************************************
 * 描述: TCP配置解析器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpConfigParser : public boost::noncopyable
{
public:
	static bool ParseTcpConfig(TcpConfigDomainVec& tcp_domains);

private:
	static bool ParseConfigDomain(const ConfigDomain& config_domain, TcpConfigDomainVec& tcp_domains);

	static bool ParseEntryLine(const string& entry_line, TcpConfigDomain& tcp_domain);

	static bool ParseConfigLines(const ConfigLineVec& config_lines, TcpConfigDomain& tcp_domain);

	static bool ParseFilter(const string& filter, TcpConfigDomain& tcp_domain);

};

}

#endif