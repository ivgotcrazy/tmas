/*##############################################################################
 * �ļ���   : ip_region_parser.hpp
 * ������   : teck_zhou
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : IpRegionParser������
 * ��Ȩ���� : Copyright(c)2014 BroadInter.All rights reserved.
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
 * ��  ��: ��ʼ����Ip��ַ����Ϣ
 * ��  ��: tom_liu
 * ʱ  ��: 2014��04��17��
 ******************************************************************************/
struct IpRegion
{
	IpRegion() : start_addr(0), end_addr(0){}
	
	uint32 start_addr;
	uint32 end_addr;
};

typedef std::vector<IpRegion> IpRegionVec;

/*******************************************************************************
 * ��  ��: IP��Χ��������TCP��HTTP����Ҫʹ�á�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��08��
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