/*#############################################################################
 * �ļ���   : pkt_resolver.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��03��
 * �ļ����� : PktResolver������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_RESOLVER
#define BROADINTER_PKT_RESOLVER

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "tmas_typedef.hpp"

namespace BroadInter
{

namespace PR // packet resolver
{

// IPα�ײ������ڼ���У���
#pragma pack(1)
	struct PseudoHeader
	{
		uint32 source_ip = 0;  // ԴIP��ַ
		uint32 dest_ip	 = 0;  // Ŀ��IP��ַ
		uint8  user      = 0;  // δ֪
		uint8  protocol  = 0;  // �����Э��
		uint16 length    = 0;  // IP�����ݳ���
	};
#pragma pack()

//--- IP��

inline uint32 GetSourceIpN(char* ip_hdr);	// ��ȡIP��Դ��ַ�������ֽ�˳��
inline uint32 GetSourceIpH(char* ip_hdr);	// ��ȡIP��Դ��ַ�������ֽ�˳��
inline uint32 GetDestIpN(char* ip_hdr);		// ��ȡIP��Ŀ�ĵ�ַ�������ֽ�˳��
inline uint32 GetDestIpH(char* ip_hdr);		// ��ȡIP��Ŀ�ĵ�ַ�������ֽ�˳��

inline uint16 GetFragPktId(char* ip_hdr);	// ��ȡ��Ƭ���ı�ʶ
inline uint16 GetFragOffset(char* ip_hdr);	// ��ȡ��Ƭƫ��
inline bool GetMfFlag(char* ip_hdr);		// ��ȡ��Ƭ��ʶ

inline uint16 GetIpHdrLen(char* ip_hdr);	// ��ȡIPͷ������
inline uint16 GetIpDataLen(char* ip_hdr);	// ��ȡIP���ݳ���
inline uint16 GetIpTotalLen(char* ip_hdr);	// ��ȡIP�����ܳ���

inline uint8 GetIpHdrProt(char* ip_hdr);	// ��ȡIP�����Э��
inline uint8 GetIpVersion(char* ip_hdr);	// ��ȡIPЭ��汾

inline bool CheckIpHeaderCrc(char* ip_hdr, uint16 header_len);	// IPͷ����CRCУ��

inline void SetIpTotalLen(char* ip_hdr, uint16 total_len);	// ����IP�����ܳ���
inline void SetMfFlag(char* ip_hdr, bool flag);
inline void SetFragOffset(char* ip_hdr, uint16 offset);
inline void SetFragPktId(char* ip_hdr, uint16 pkt_id);

//--- TCP

inline uint16 GetTcpSrcPortN(char* l4_hdr);
inline uint16 GetTcpSrcPortH(char* l4_hdr);
inline uint16 GetTcpDstPortN(char* l4_hdr);
inline uint16 GetTcpDstPortH(char* l4_hdr);

inline bool IsTcpAckSet(char* l4_hdr);	// ACK��־λ
inline bool IsTcpSynSet(char* l4_hdr);	// SYN��־λ
inline bool IsTcpFinSet(char* l4_hdr);	// FIN��־λ
inline bool IsTcpRstSet(char* l4_hdr);	// RST��־λ

inline uint32 GetTcpSeqNum(char* l4_hdr);	// ˳���
inline uint32 GetTcpAckNum(char* l4_hdr);	// ȷ�Ϻ�

inline uint16 GetTcpHdrLen(char* l4_hdr);	// TCPͷ������

//--- UDP

inline uint16 GetUdpSrcPortN(char* l4_hdr);
inline uint16 GetUdpSrcPortH(char* l4_hdr);
inline uint16 GetUdpDstPortN(char* l4_hdr);
inline uint16 GetUdpDstPortH(char* l4_hdr);

//--- ����

PseudoHeader GetPseudoHeader();  // ��ȡα�ײ�

} // namespace PR

} // namespace BroadInter

#include "pkt_resolver_inl.hpp"

#endif

