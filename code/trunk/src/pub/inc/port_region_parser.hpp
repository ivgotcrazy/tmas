/*#############################################################################
 * 文件名   : port_region_parser.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : PortRegionParser类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PORT_REGION
#define BROADINTER_PORT_REGION

#include <vector>

#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * 描  述: 端口范围
 * 作  者: teck_zhou
 * 时  间: 2014年05月10日
 ******************************************************************************/
struct PortRegion
{
	PortRegion() : start_port(0), end_port(0) {}

	uint16 start_port;
	uint16 end_port;
};

typedef std::vector<PortRegion> PortRegionVec;

/*******************************************************************************
 * 描  述: 端口范围解析器
 * 作  者: teck_zhou
 * 时  间: 2014年05月10日
 ******************************************************************************/
class PortRegionParser
{
public:
	static bool ParsePortRegions(const string& config, PortRegionVec& port_regions);

private:
	static bool ParsePortRegion(const string& region, PortRegionVec& port_regions);

	static bool ParsePortPair(const string& region, uint16& start, uint16& end);
};

}

#endif