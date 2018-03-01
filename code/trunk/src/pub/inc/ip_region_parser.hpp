/*##############################################################################
 * 文件名   : ip_region_parser.hpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年05月08日
 * 文件描述 : IpRegionParser类声明
 * 版权声明 : Copyright(c)2014 BroadInter.All rights reserved.
 * ###########################################################################*/

#ifndef BROADINTER_IP_REGION_PARSER
#define BROADINTER_IP_REGION_PARSER

#include <vector>
#include <string>

#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * 描  述: 起始结束Ip地址对信息
 * 作  者: tom_liu
 * 时  间: 2014年04月17日
 ******************************************************************************/
struct IpRegion
{
	IpRegion() : start_addr(0), end_addr(0){}
	
	uint32 start_addr;
	uint32 end_addr;
};

typedef std::vector<IpRegion> IpRegionVec;

/*******************************************************************************
 * 描  述: IP范围解析器，TCP和HTTP都需要使用。
 * 作  者: teck_zhou
 * 时  间: 2014年05月08日
 ******************************************************************************/
class IpRegionParser
{
public:
	static bool ParseIpRegions(const string& config, IpRegionVec& ip_addr_vec);

private:
	static bool VerifyIpAddr(const string& ip_addr);

	static bool AddIpAddr(const string& ip_item, IpRegionVec& ip_addr_vec);

	static bool IsIpAddrConflict(const IpRegion& region1, const IpRegion& region2); 
};

}

#endif