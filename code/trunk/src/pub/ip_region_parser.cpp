/*##############################################################################
 * 文件名   : ip_region_parser.hpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年05月08日
 * 文件描述 : IpRegionParser类声明
 * 版权声明 : Copyright(c)2014 BroadInter.All rights reserved.
 * ###########################################################################*/

#include <glog/logging.h>
#include <boost/lexical_cast.hpp>

#include "ip_region_parser.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 解析Ip地址配置字段
 * 参  数: [in] config_value 配置字段
 *         [in] ip_addr_vec 存储地址容器
 * 返回值: 是否已经存在
 * 修  改:
 *   时间 2014年04月017日
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool IpRegionParser::ParseIpRegions(const string& config, IpRegionVec& ip_addr_vec)
{
	uint32 cur = 0;

	// 游标移动到非空位置
	while (config[cur] == ' ')
	{
		cur++;
		continue;
	}

	if (cur > config.size()) 
	{
		LOG(ERROR) << "Empty ip-filter";
		return false;
	}

	uint32 begin = 0;
	while (cur <= config.size())
	{
		while (config[cur] != ' ' && cur < config.size())
		{
			cur++;
			continue;
		}

		string ip_item = config.substr(begin, cur - begin);

		// 判断Ip地址信息是否符合规范
		if (!VerifyIpAddr(ip_item))
		{
			LOG(ERROR) << "Ip addr don't match the standard | value:" << ip_item;
			return false;
		}

		// 跟前述地址有没有冲突，没有就进行插入
		if (!AddIpAddr(ip_item, ip_addr_vec))
		{
			LOG(ERROR) << "Ip addr conflict";
			return false;
		}

		cur++;
		begin = cur;
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 添加Ip地址
 * 参  数: [in] ip_item Ip地址段
 *         [in] ip_addr_vec Ip地址存储容器
 * 返回值: 添加是否成功
 * 修  改:
 *   时间 2014年04月14日
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool IpRegionParser::AddIpAddr(const string& ip_item, IpRegionVec& ip_addr_vec)
{
	uint32 begin_addr = 0;
	uint32 end_addr = 0;
	string::size_type found = ip_item.find('/');
	if (found != string::npos)
	{
		uint32 address = ntohl(inet_addr(ip_item.substr(0, found).c_str()));
		uint32 net_mask = boost::lexical_cast<uint32>(ip_item.substr(found + 1));

		TMAS_ASSERT(net_mask < 32);

		uint32 offset = 0xFFFFFFFF >> net_mask;
		if ((address & offset) != 0)
		{
			begin_addr = end_addr = address;
		}
		else
		{
			begin_addr = address;
			end_addr = address | offset;
		}
	}
	else
	{
		string::size_type pos = ip_item.find('-');
		begin_addr =ntohl(inet_addr(ip_item.substr(0, pos).c_str()));
		end_addr = ntohl(inet_addr(ip_item.substr(pos + 1).c_str()));
	}

	IpRegion region;
	region.start_addr = begin_addr;
	region.end_addr = end_addr;

	if (!ip_addr_vec.empty())
	{
		for (auto addr_region : ip_addr_vec)
		{
			if (IsIpAddrConflict(region, addr_region))
			{	
				LOG(ERROR) << "Ip Addr Conflict |orign addr ";
				return false;
			}
		}
	}

	ip_addr_vec.push_back(region);
	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 判断Ip地址段是否符合格式要求
 * 参  数: [in] ip_addr Ip地址段
 * 返回值: 是否符合要求
 * 修  改:
 *   时间 2014年04月14日
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool IpRegionParser::VerifyIpAddr(const string& ip_addr)
{
	// 验证标准: 1.匹配模式判断  2.四个点分字段. 
	if (ip_addr.find('/') != string::npos)
	{
		string::size_type pos = 0;
		string::size_type found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;
		found = ip_addr.find('/', pos);
		if (found == string::npos) return false;

		uint64 net_mask = 0;
		try 
		{
			net_mask = boost::lexical_cast<uint32>(ip_addr.substr(found + 1));
		}
		catch (...)
		{
			LOG(ERROR) << "Parse net_mask failed, addr: " << ip_addr;
			return false;
		}

		if (net_mask > 32 || net_mask < 0)
		{
			LOG(ERROR) << "netmask is invalid";
			return false;
		}
	}
	else if (ip_addr.find('-') != string::npos)
	{
		string::size_type pos = 0;
		string::size_type found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;	
		found = ip_addr.find('-', pos);
		if (found == string::npos) return false;

		string::size_type split_pos = found;
		uint64 start_addr = ntohl(inet_addr(ip_addr.substr(0, split_pos).c_str()));

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;

		uint64 end_addr = ntohl(inet_addr(ip_addr.substr(split_pos + 1).c_str()));

		if (end_addr < start_addr)
		{
			LOG(ERROR) << "End addr is bigger than start addr.";
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 判断Ip地址是否冲突
 * 参  数: [in] start 开始地址
 *         [in] end 结束地址
 *         [in] cmp_start 比较开始地址
 *         [in] cmp_end 比较结束地址
 * 返回值: 是否有地址冲突, 冲突返回真
 * 修  改:
 *   时间 2014年04月16日
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool IpRegionParser::IsIpAddrConflict(const IpRegion& region1, const IpRegion& region2)
{
	TMAS_ASSERT(region1.start_addr <= region1.end_addr);
	TMAS_ASSERT(region2.start_addr <= region2.end_addr);

	if (region1.start_addr == region1.end_addr)
	{
		if (region1.start_addr > region2.start_addr 
			&& region1.start_addr < region2.end_addr)
		{
			return true;
		}
	}
	else
	{
		if (region1.start_addr > region2.end_addr 
			|| region1.end_addr < region2.start_addr)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

}