/*
 * Copyright (c) 2010 Yang CHI, CDMC, University of Cincinnati
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * Authors: Yang CHI <chiyg@mail.uc.edu>
 */


#include "IPCope-header.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE("IPCopeHeader");

namespace ns3
{
namespace ipcope
{

IPCopeHeader::IPCopeHeader()
{
	m_encodedNum = 0;
	m_reportNum = 0;
	m_ackNum = 0;
}
IPCopeHeader::~IPCopeHeader(){}

TypeId
IPCopeHeader::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::cope::IPCopeHeader")
		.SetParent<Header>()
		.AddConstructor<IPCopeHeader>()
		;
	return tid;
}

TypeId
IPCopeHeader::GetInstanceTypeId(void) const
{
	return GetTypeId();
}

void
IPCopeHeader::Print (std::ostream &os) const
{
	os << "Sender IP " << m_ip << ", ";
	os << "ENCODED NUM " << m_encodedNum << ", ";
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{
		os << (*iter).second << ", " << (*iter).first << ", ";
	}

	os << "REPORT NUM " << m_reportNum << ", ";
	/*
	std::vector<RecpReport>::const_iterator RecpRepIter;
	for(RecpRepIter = m_receptionReports.begin(); RecpRepIter != m_receptionReports.end(); RecpRepIter++)
	{
		os << (*RecpRepIter).address << ", " << (*RecpRepIter).lastPkt << ", " << (int)(*RecpRepIter).bitMap << ", ";
	}*/
	std::vector<uint32_t>::const_iterator recpIter;
	for(recpIter = m_recps.begin(); recpIter != m_recps.end(); recpIter++)
	{
		os<<(*recpIter)<<", ";
	}

	os << "ACK NUM " << m_ackNum << ", ";
	//os << "LOCAL PKT SEQ NUM " << m_localPktSeqNum << ", ";
	std::vector<AckBlock>::const_iterator ackBlockIter;
	for(ackBlockIter = m_ackBlocks.begin(); ackBlockIter != m_ackBlocks.end(); ackBlockIter++)
	{
		//os << (*ackBlockIter).address << ", " << (*ackBlockIter).lastAck << ", " << (int)(*ackBlockIter).ackMap ;
		os<< (*ackBlockIter).address << ", "<<(*ackBlockIter).pid<<", ";
	}
}

void
IPCopeHeader::Serialize(Buffer::Iterator start) const
{
	WriteTo(start, m_ip);
	start.WriteHtonU16 (m_encodedNum);
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{	
		WriteTo(start, (*iter).first);
		start.WriteHtonU32 ((*iter).second);
	}
	
	start.WriteHtonU16 (m_reportNum);
	/*
	std::vector<RecpReport>::const_iterator RecpRepIter;
	for(RecpRepIter = m_receptionReports.begin(); RecpRepIter != m_receptionReports.end(); RecpRepIter++)
	{
		WriteTo(start, (*RecpRepIter).address);
		start.WriteHtonU32 ((*RecpRepIter).lastPkt);
		start.WriteU8 ((*RecpRepIter).bitMap);
	}
	*/
	std::vector<uint32_t>::const_iterator recpIter;
	for(recpIter = m_recps.begin(); recpIter != m_recps.end(); recpIter++)
	{
		start.WriteHtonU32(*recpIter);
	}

	start.WriteHtonU16(m_ackNum);
	//start.WriteHtonU16(m_localPktSeqNum);
	std::vector<AckBlock>::const_iterator ackBlockIter;
	for(ackBlockIter = m_ackBlocks.begin(); ackBlockIter != m_ackBlocks.end(); ackBlockIter++)
	{
		WriteTo(start, (*ackBlockIter).address);
		//start.WriteHtonU32((*ackBlockIter).lastAck);
		//start.WriteU8((*ackBlockIter).ackMap);
		start.WriteHtonU32((*ackBlockIter).pid);
	}
}

uint32_t
IPCopeHeader::GetSerializedSize() const
{
	NS_ASSERT(m_reportNum == m_recps.size());
	NS_ASSERT(m_ackNum == m_ackBlocks.size());
	return 4 //m_ip
		+ 2 //m_encodedNum, uint16_t
		+ (6+4)*(uint32_t)m_pidNexthops.size()
		+ 2 // m_reportNum, uint16_t
		//+ (4+4+1)*(uint32_t)m_receptionReports.size() 
		+ 4 * (uint32_t)m_recps.size()
		+ 2 //m_ackNum, uint16_t
		//+ 2 //m_localPktSeqNum
		//+ (4+4+1)*(uint32_t)m_ackBlocks.size();
		+(4+4)*(uint32_t)m_ackBlocks.size();
}

uint32_t
IPCopeHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator bufIter = start;
	ReadFrom(bufIter, m_ip);
	m_encodedNum = bufIter.ReadNtohU16();
	m_pidNexthops.clear();
	uint32_t pktId;
	Mac48Address add;
	for(int i = 0; i<m_encodedNum; i++)
	{	
		ReadFrom(bufIter, add);
		pktId = bufIter.ReadNtohU32();
		m_pidNexthops.insert(std::make_pair(add, pktId));
	}

	m_reportNum = bufIter.ReadNtohU16();
	/*
	RecpReport report;
	m_receptionReports.clear();
	for(int i = 0; i<m_reportNum; i++)
	{
		ReadFrom(bufIter, report.address);
		report.lastPkt = bufIter.ReadNtohU32();
		report.bitMap = bufIter.ReadU8();
		m_receptionReports.push_back(report);
	}
	*/
	m_recps.clear();
	for(int i = 0; i<m_reportNum; i++)
	{
		uint32_t id = bufIter.ReadNtohU32();
		m_recps.push_back(id);
	}

	m_ackNum = bufIter.ReadNtohU16();
	//m_localPktSeqNum = bufIter.ReadNtohU16();
	AckBlock ack;
	m_ackBlocks.clear();
	for(int i = 0; i<m_ackNum; i++)
	{
		ReadFrom(bufIter, ack.address);
		//ack.lastAck = bufIter.ReadNtohU32();
		//ack.ackMap = bufIter.ReadU8();
		ack.pid = bufIter.ReadNtohU32();
		m_ackBlocks.push_back(ack);
	}

	uint32_t dist = bufIter.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	return dist;
}

std::vector<uint32_t>
IPCopeHeader::GetRecpReports() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_recps;
}

uint16_t
IPCopeHeader::GetReportNum() const
{
	NS_LOG_FUNCTION(this<<m_reportNum);
	NS_ASSERT(m_recps.size() == m_reportNum);
	return m_reportNum;
}

bool
IPCopeHeader::AddIdNexthop(const Mac48Address & nexthop, uint32_t pktId)
{
	NS_LOG_FUNCTION_NOARGS();
	if(m_pidNexthops.find(nexthop) != m_pidNexthops.end())
		return false;
	NS_ASSERT(GetEncodedNum() < 65536);//2^16 = 65536
	m_pidNexthops.insert(std::make_pair(nexthop, pktId));
	m_encodedNum++;
	return true;
}

uint16_t
IPCopeHeader::GetEncodedNum() const
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(m_pidNexthops.size() == m_encodedNum);
	return m_encodedNum;
}

void
IPCopeHeader::SetEncodedNum(uint16_t encodedNum)
{
	m_encodedNum = encodedNum;
}

std::map<Mac48Address, uint32_t>
IPCopeHeader::GetIdNexthops() const
{
	return m_pidNexthops;
}

bool
IPCopeHeader::AmINext(std::vector<Mac48Address> macs, uint32_t &pid) const
{
	std::vector<Mac48Address>::const_iterator mac_iter;
	for(mac_iter = macs.begin(); mac_iter != macs.end(); mac_iter++)
	{
		std::map<Mac48Address, uint32_t>::const_iterator iter = m_pidNexthops.find(*mac_iter);
		if(iter != m_pidNexthops.end())
		{
			pid = iter->second;
			return true;
		}
	}
	return false;
}

bool
IPCopeHeader::AmINext(const Mac48Address & mac, uint32_t &pid) const
{
	std::map<Mac48Address, uint32_t>::const_iterator iter = m_pidNexthops.find(mac);
	if(iter == m_pidNexthops.end())
		return false;
	else{
		pid = iter->second;
		return true;
	}
}

bool
IPCopeHeader::IsBroadcast() const
{
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{
		if(iter->first.IsBroadcast())
		{
			NS_ASSERT(m_pidNexthops.size() == 1);
			return true;
		}
	}
	return false;
}

/*
bool
IPCopeHeader::AmINext(Ipv4Address & add, uint32_t & pid) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::map<Ipv4Address, uint32_t>::const_iterator iter = m_pidNexthops.find(add);
	if(iter == m_pidNexthops.end())
		return false;
	else{
		pid = iter->second;
		return true;
	}
}
*/

/*
bool
IPCopeHeader::AmINext(Ipv4Mask & mask, uint32_t & pid) const
{
	NS_LOG_FUNCTION(this<<mask);
	std::map<Ipv4Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{
		if(iter->first.IsBroadcast() || iter->first.IsMulticast() || iter->first.IsLocalMulticast() || iter->first.IsSubnetDirectedBroadcast(mask))
		{
			pid = iter->second;
			return true;
		}
	}
	return false;
}
*/

/*
bool
IPCopeHeader::Search(const std::vector<RecpReport> reportVec, const Ipv4Address addr, RecpReport* report) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<RecpReport>::const_iterator iter;
	for(iter = reportVec.begin(); iter!= reportVec.end(); iter++)
	{
		if(iter->address == addr)
		{
			if(report)
				*report = *iter;
			return true;
		}
	}
	return false;
}
*/


bool
IPCopeHeader::Search(const std::vector<AckBlock> ackVec, const Ipv4Address & addr, AckBlock* ackblock) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<AckBlock>::const_iterator iter;
	for(iter = ackVec.begin(); iter != ackVec.end(); iter++)
	{
		if(iter->address == addr)
		{
			if(ackblock)
				*ackblock = *iter;
			return true;
		}
	}
	return false;
}

std::vector<uint32_t>
IPCopeHeader::Search(const std::vector<AckBlock> ackVec, const Ipv4Address & addr)
{
	NS_LOG_FUNCTION(this<<"pids we should retrieve from Search: "<<ackVec.size());
	std::vector<uint32_t> pids;
	std::vector<AckBlock>::const_iterator iter;
	for(iter = ackVec.begin(); iter!= ackVec.end(); iter++)
	{
		if(iter->address == addr)
			pids.push_back(iter->pid);
	}
	return pids;
}

bool
IPCopeHeader::AddRecpReport(uint32_t pid)
{
	NS_LOG_FUNCTION_NOARGS();
	if(find(m_recps.begin(), m_recps.end(), pid) != m_recps.end())
		return false;
	m_recps.push_back(pid);
	m_reportNum++;
	NS_ASSERT(m_reportNum == m_recps.size());
	return true;
}

bool
IPCopeHeader::AddAckBlock(const Ipv4Address & address, uint32_t pid)
{
	if(!Search(m_ackBlocks, address).size())
		return false;
	AckBlock ackBlock;
	ackBlock.address = address;
	ackBlock.pid = pid;
	m_ackBlocks.push_back(ackBlock);
	m_ackNum++;
	return true;
}

void
IPCopeHeader::SetReportNum(uint16_t reportNum)
{
	NS_LOG_FUNCTION(this<<reportNum);
	m_reportNum = reportNum;
}

void
IPCopeHeader::SetAckNum(uint16_t ackNum)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ackNum = ackNum;
}

void
IPCopeHeader::AddAckBlock(std::vector<AckBlock> ackBlock)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ackBlocks = ackBlock;
}

void
IPCopeHeader::AddAckBlock(AckBlock ack)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ackBlocks.push_back(ack);
	m_ackNum++;
}

void
IPCopeHeader::SetIp(const Ipv4Address & ip)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ip = ip;
}

Ipv4Address
IPCopeHeader::GetIp() const
{
	return m_ip;
}

IPCopeType::IPCopeType(MessageType type ):
	m_type (type)
{
}

IPCopeType::IPCopeType()
{}

IPCopeType::~IPCopeType(){}

TypeId
IPCopeType::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::cope::IPCopeType")
		.SetParent<Header>()
		.AddConstructor<IPCopeType>()
		;
	return tid;
}

TypeId
IPCopeType::GetInstanceTypeId(void) const
{
	return GetTypeId();
}

void
IPCopeType::Print (std::ostream &os) const
{
	os << "This is a ";
	if(m_type == DATA)
		os << "DATA ";
	else
		os<<"HELLO ";
	os<<"packet.";
	os.flush();
}

void
IPCopeType::Serialize(Buffer::Iterator start) const
{
	start.WriteU8((uint8_t) m_type);
}

uint32_t
IPCopeType::GetSerializedSize() const
{
	return 1;
}

uint32_t
IPCopeType::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator bufIter = start;
	uint8_t type = bufIter.ReadU8();
	m_type = (MessageType) type;
	uint32_t dist = bufIter.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	return dist;
}

bool
IPCopeType::IsHello() const
{
	NS_LOG_FUNCTION(this<<m_type<<HELLO);
	return (m_type == HELLO);
}

bool
IPCopeType::IsData() const
{
	NS_LOG_FUNCTION(this<<m_type<<DATA);
	return (m_type == DATA);
}

IPCopeHello::IPCopeHello()
{
}

IPCopeHello::~IPCopeHello(){}

TypeId
IPCopeHello::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::cope::IPCopeHello")
		.SetParent<Header>()
		.AddConstructor<IPCopeHello>()
		;
	return tid;
}

TypeId
IPCopeHello::GetInstanceTypeId(void) const
{
	return GetTypeId();
}

void
IPCopeHello::Print (std::ostream &os) const
{
	os<<"This hello mesg contains "<<m_addPair.size()<<" AddCh pairs: ";
	std::vector<AddressPair>::const_iterator iter;
	for(iter = m_addPair.begin(); iter!=m_addPair.end(); iter++)
	{
		os<<"ip: "<<iter->ip<<", ";
		os<<"mac: "<<iter->mac<<", ";
		os<<"ch#: "<<iter->channel;
	}
	os.flush();
}

void
IPCopeHello::Serialize(Buffer::Iterator start) const
{
	start.WriteU8(GetLength());
	std::vector<AddressPair>::const_iterator iter;
	for(iter = m_addPair.begin(); iter != m_addPair.end(); iter++)
	{
		WriteTo(start, iter->ip);
		WriteTo(start, iter->mac);
		start.WriteHtonU16(iter->channel);
	}
}

uint32_t
IPCopeHello::GetSerializedSize() const
{
	return 1+(4+6+2)*(uint32_t)m_addPair.size();
}

uint32_t
IPCopeHello::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator bufIter = start;
	uint8_t length = bufIter.ReadU8();
	m_addPair.clear();
	for(uint8_t i = 0; i<length; i++)
	{
		AddressPair pair;
		ReadFrom(bufIter, pair.ip);
		ReadFrom(bufIter, pair.mac);
		pair.channel = bufIter.ReadNtohU16();
		m_addPair.push_back(pair);
	}
	uint32_t dist = bufIter.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	return dist;
}

void
IPCopeHello::Add(const Ipv4Address & ip, const Mac48Address & mac, uint16_t channel)
{
	NS_LOG_FUNCTION_NOARGS();
	AddressPair pair = {ip, mac, channel};
	m_addPair.push_back(pair);
}

uint8_t
IPCopeHello::GetLength() const
{
	return m_addPair.size();
}

AddressPair
IPCopeHello::Get(uint8_t index) const
{
	NS_ASSERT(index < m_addPair.size());
	return m_addPair[index];
}

}//namespace cope
}//namespace ns3
