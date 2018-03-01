/*#############################################################################
 * �ļ���   : port_region_parser.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : PortRegionParser������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PORT_REGION
#define BROADINTER_PORT_REGION

#include <vector>

#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * ��  ��: �˿ڷ�Χ
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��10��
 ******************************************************************************/
struct PortRegion
{
	PortRegion() : start_port(0), end_port(0) {}

	uint16 start_port;
	uint16 end_port;
};

typedef std::vector<PortRegion> PortRegionVec;

/*******************************************************************************
 * ��  ��: �˿ڷ�Χ������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��10��
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