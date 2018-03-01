/*##############################################################################
 * �ļ���   : http_config_parser.hpp
 * ������   : teck_zhou
 * ����ʱ�� : 2014��3��29��
 * �ļ����� : HttpConfigParser��������ļ�
 * ��Ȩ���� : Copyright(c)2014 BroadInter.All rights reserved.
 * ###########################################################################*/
#ifndef BROADINTER_HTTP_CONFIG_PARSER
#define BROADINTER_HTTP_CONFIG_PARSER

#include <string>
#include <vector>

#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

//typedef std::vector<std::string> IpAddrVec;

/*******************************************************************************
 * ��  ��: ��ʼ����Ip��ַ����Ϣ
 * ��  ��: tom_liu
 * ʱ  ��: 2014��04��17��
 ******************************************************************************/
struct IpAddrRegion
{
	IpAddrRegion() : start_addr(0), end_addr(0){}
	
	uint32 start_addr;
	uint32 end_addr;
};

typedef std::vector<IpAddrRegion> IpAddrVec;

/*******************************************************************************
 * ��  ��: HOST����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��29��
 ******************************************************************************/
struct HostConfig
{
	HostConfig() : min_delay(0), min_speed(0), stat_interval(0) {}

	string host;
	
	bool delay_monitor;		// ����ʱ�Ӽ�⿪��
	uint32 min_delay;		// ���ڴ�ʱ�Ӳż�¼

	bool speed_monitor;		// �����ٶȼ�⿪��
	uint32 min_speed;		// ���ڴ��ٶȲż�¼

	IpAddrVec src_ips;
	IpAddrVec dst_ips;

	uint32 stat_interval;	// ��¼ʱ����

	std::vector<string> uris;
};

/*******************************************************************************
 * ��  ��: URI����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��29��
 ******************************************************************************/
struct UriConfig
{
	UriConfig() : min_delay(0), min_speed(0), stat_interval(0) {}

	string uri;

	bool delay_monitor;		// ����ʱ�Ӽ�⿪��
	uint32 min_delay;		// ���ڴ�ʱ�Ӳż�¼

	bool speed_monitor;		// �����ٶȼ�⿪��
	uint32 min_speed;		// ���ڴ��ٶȲż�¼

	IpAddrVec src_ips;
	IpAddrVec dst_ips;

	uint32 stat_interval;	// ��¼ʱ����
};

typedef std::vector<HostConfig> HostConfigVec;
typedef std::vector<UriConfig> UriConfigVec;

/*******************************************************************************
 * ��  ��: HTTP�����ļ�������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��29��
 ******************************************************************************/
class HttpConfigParser
{
public:
	static bool ParseConfig(HostConfigVec& host_configs, UriConfigVec& uri_configs);

private:
	typedef std::vector<string> ConfigLineVec;

	static bool ConfigItemMatch(const string& origin, string& key, string& value);

	static bool ParseConfigEntry(const string& entry_line, const ConfigLineVec& config_lines, 
		HostConfigVec& host_config, UriConfigVec& uri_configs);

	static bool ParseUriConfig(const string& entry_value, 
		const ConfigLineVec& config_lines, UriConfigVec& uri_configs);

	static bool ParseHostConfig(const string& entry_value,
		const ConfigLineVec& config_lines, HostConfigVec& host_configs);

	static bool IfHostEntryExist(const string& entry, const HostConfigVec& host_configs);

	static bool IfUriEntryExist(const string& entry, const UriConfigVec& uri_configs);

	static bool ParseIpItem(const string config_value, IpAddrVec& ip_addr_vec);

	static bool VerifyIpAddr(const string ip_addr);

	static bool AddIpAddr(const string ip_item, IpAddrVec& ip_addr_vec);

	static bool IsIpAddrConflict(const uint32 start, const uint32 end
											,const uint32 cmp_start, const uint32 cmp_end);
};

}  // namespace BroadInter

#endif  // BROADINTER_URI_CONFIG_PARSER
