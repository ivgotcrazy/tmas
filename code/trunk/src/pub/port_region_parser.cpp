/*#############################################################################
 * �ļ���   : port_region_parser.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : PortRegionParser��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "port_region_parser.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

#define MAX_PORT 65535

/*-----------------------------------------------------------------------------
 * ��  ��: �����˿ڶ�
 * ��  ��: [in] config �˿ڶ��ַ���
 *         [out] port_regions �˿ڶ�����
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PortRegionParser::ParsePortRegions(const string& config, PortRegionVec& port_regions)
{
	std::vector<std::string> regions = SplitStr(config, ' ');
	if (regions.empty())
	{
		LOG(ERROR) << "No port region in config";
		return false;
	}

	for (const std::string& region : regions)
	{
		if (!ParsePortRegion(region, port_regions))
		{
			LOG(ERROR) << "Fail to parse port region | " << region;
			return false;
		}
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����˿ڶ�
 * ��  ��: [in] config �˿ڶ��ַ���
 *         [out] port_regions �˿ڶ�����
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PortRegionParser::ParsePortRegion(const string& region, PortRegionVec& port_regions)
{
	PortRegion port_region;

	if (!ParsePortPair(region, port_region.start_port, port_region.end_port))
	{
		LOG(ERROR) << "Fail to parse port pair | " << region;
		return false;
	}

	port_regions.push_back(port_region);

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����˿ڶε���ʼ�˿ںͽ����˿�
 * ��  ��: [in] region �˿ڶ��ַ���
 *         [out] start ��ʼ�˿�
 *         [out] end �����˿�
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PortRegionParser::ParsePortPair(const string& region, uint16& start, uint16& end)
{
	// ȷ���Ƿ���port1-port2���÷�ʽ
	static boost::regex reg("\\s*([0-9]+)-([0-9]+)\\s*$");

	uint32 start_port, end_port;

	boost::smatch match;  // ������ʽƥ��Ľ��
	if (boost::regex_match(region, match, reg) && match.size() == 3)
	{
		try 
		{
			start_port = boost::lexical_cast<uint32>(match[1]);
		}
		catch (...)
		{
			LOG(ERROR) << "Invalid start port | " << match[1];
			return false;
		}

		if (start_port > MAX_PORT)
		{
			LOG(ERROR) << "Invalid start port value " << start_port;
			return false;
		}

		try 
		{
			end_port = boost::lexical_cast<uint32>(match[2]);
		}
		catch (...)
		{
			LOG(ERROR) << "Invalid end port | " << match[2];
			return false;
		}

		if (end_port > MAX_PORT)
		{
			LOG(ERROR) << "Invalid end port value " << end_port;
			return false;
		}

		start = static_cast<uint16>(std::min(start_port, end_port));
		end   = static_cast<uint16>(std::max(start_port, end_port));

		return true;
	}
	else
	{
		try 
		{
			start_port = boost::lexical_cast<uint32>(region);
		}
		catch (...)
		{
			LOG(ERROR) << "Invalid port | " << region;
			return false;
		}

		if (start_port > MAX_PORT)
		{
			LOG(ERROR) << "Invalid port value " << start_port;
			return false;
		}

		start = static_cast<uint16>(start_port);
		end   = static_cast<uint16>(start_port);

		return true;
	}
}

}