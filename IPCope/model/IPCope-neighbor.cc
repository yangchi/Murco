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
 * Authors: Yang CHI <chiyg@mail.uc.edu>
 */

#include <stdlib.h>
#include <stdio.h>
#include "IPCope-neighbor.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE("IPCopeNeighbor");

namespace ns3{
namespace ipcope{

IPCopeNeighbor::IPCopeNeighbor()
{
	Init();
}

IPCopeNeighbor::~IPCopeNeighbor(){}

IPCopeNeighbors::IPCopeNeighbors(){}
IPCopeNeighbors::~IPCopeNeighbors(){}

void
IPCopeNeighbor::Init()
{
}

/*
bool
IPCopeNeighbor::AddMac(const Mac48Address & mac)
{
	NS_LOG_FUNCTION(this<<mac);
	std::deque<Mac48Address>::const_iterator iter = find(m_macAddresses.begin(), m_macAddresses.end(), mac);
	if(iter == m_macAddresses.end())
	{
		m_macAddresses.push_back(mac);
		return true;
	}
	else 
		return false;
}
*/

std::deque<Mac48Address> 
IPCopeNeighbor::GetMacs() const
{
	std::deque<Mac48Address> macs;
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); iter++)
		macs.push_back(iter->mac);
	return macs;
}

Mac48Address
IPCopeNeighbor::GetMac() const
{
	NS_LOG_FUNCTION(this<<m_addressPairs.size());
	if(!m_addressPairs.size())
		return NULL;
	Mac48Address min = m_addressPairs.begin()->mac;
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter!=m_addressPairs.end(); iter++)
	{
		if (iter->mac < min)
			min = iter->mac;
	}
	NS_LOG_FUNCTION(min);
	return min;
}

IPCopeQueueEntry*
IPCopeNeighbor::RemoveVirtualQueueEntry()
{
	NS_LOG_FUNCTION_NOARGS();
	IPCopeQueueEntry* ventry;
	if(!m_virtualQueue.size())
		return 0;
	ventry = m_virtualQueue.front();
	m_virtualQueue.pop_front();
	return ventry;
}

bool
IPCopeNeighbor::RemoveVirtualQueueEntry(uint32_t packetId)
{
	NS_LOG_FUNCTION_NOARGS();
	bool isRemoved = false;
	std::deque<IPCopeQueueEntry *>::iterator iter;
	for(iter = m_virtualQueue.begin(); iter!= m_virtualQueue.end(); iter++)
	{
		if ((**iter).GetPacketId() == packetId)
		{
			m_virtualQueue.erase(iter);
			isRemoved = true;
			break;
		}
	}
	return isRemoved;
}

IPCopeQueueEntry*
IPCopeNeighbor::GetVirtualQueueEntry() const
{
	if(!m_virtualQueue.size())
		return 0;
	else 
		return m_virtualQueue.front();
}

void
IPCopeNeighbor::AddVirtualQueueEntryFront(IPCopeQueueEntry * vqe)
{
	NS_LOG_FUNCTION_NOARGS();
	uint32_t id = vqe->GetPacketId();
	std::deque<IPCopeQueueEntry *>::const_iterator iter;
	for(iter = m_virtualQueue.begin(); iter != m_virtualQueue.end(); iter++)
	{
		if ((**iter).GetPacketId() == id)
			NS_FATAL_ERROR(this<<"Duplicate packet with id: "<<id);
	}
	if(iter == m_virtualQueue.end())
		m_virtualQueue.push_front(vqe);
}

void
IPCopeNeighbor::AddVirtualQueueEntry(IPCopeQueueEntry * vqe)
{
	NS_LOG_FUNCTION_NOARGS();
	uint32_t id = vqe->GetPacketId();
	std::deque<IPCopeQueueEntry *>::const_iterator iter;
	for(iter = m_virtualQueue.begin(); iter != m_virtualQueue.end(); iter++)
	{
		if ((**iter).GetPacketId() == id)
			NS_FATAL_ERROR(this<<"Duplicate packet with id: "<<id);
	}
	if(iter == m_virtualQueue.end())
		m_virtualQueue.push_back(vqe);
}

/*
uint32_t
IPCopeNeighbor::AddIP(const Ipv4Address & addr)
{
	NS_LOG_FUNCTION_NOARGS();
	if(!HasIP(addr))
		m_ips.push_back(addr);
	return m_ips.size();
}
*/

std::deque<Ipv4Address>
IPCopeNeighbor::GetIPs() const
{
	NS_LOG_FUNCTION_NOARGS();
	std::deque<Ipv4Address> ips;
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); iter++)
		ips.push_back(iter->ip);
	return ips;
}

Ipv4Address
IPCopeNeighbor::GetIp() const
{
	if(!m_addressPairs.size())
		return NULL;
	Ipv4Address min;
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter!=m_addressPairs.end(); iter++)
	{
		if (iter->ip < min)
			min = iter->ip;
	}
	return min;
}

/*
bool
IPCopeNeighbor::operator< (const IPCopeNeighbor& neighbor) const
{
	return m_ipAddress<neighbor.m_ipAddress;
}

bool
IPCopeNeighbor::operator== (const IPCopeNeighbor& neighbor) const
{
	return m_ipAddress == neighbor.m_ipAddress;
}
*/

/*
void
IPCopeNeighbor::SetSeqCounter(uint32_t seq)
{
	m_IPCopeNeighborSeqnoCounter = seq;
}

void
IPCopeNeighbor::IncSeqCounter()
{
	SetSeqCounter(GetSeqCounter()+1);
}
*/

/*
bool
IPCopeNeighbor::SetLastAckFromN(uint32_t last, uint8_t map)
{
	NS_LOG_FUNCTION(this<<last<<m_lastAckFromN);
	if(last > m_lastAckFromN)
	{
		m_lastAckFromN = last;
		m_ackMapFromN = map;
		return true;
	}
	else 
		return false;
	//m_lastAckFromN = last > m_lastAckFromN ? last : m_lastAckFromN;
	//return m_lastAckFromN;
}
*/

/*
uint8_t
IPCopeNeighbor::SetAckMapFromN(uint8_t map)
{
	m_ackMapFromN = map;
	return m_ackMapFromN;
}
*/

/*
void
IPCopeNeighbor::SetLastAckToN(uint32_t last)
{
	NS_LOG_FUNCTION(this<<m_lastAckToN<<(int)m_ackMapToN<<last);
	if(last <= m_lastAckToN)
		return ;
	m_ackMapToN <<= 1;
	m_ackMapToN++;
	m_ackMapToN <<= (last - m_lastAckToN - 1);
	m_lastAckToN = last;
}
*/

/*
void
IPCopeNeighbor::SetAckMapToN(uint8_t map)
{
	m_ackMapToN = map;
}
*/

/*
bool
IPCopeNeighbor::DoUpdate(uint32_t lastPkt)
{
	if(lastPkt < m_lastPkt)
	{
		if(lastPkt < m_lastPkt - 7)
			return false;
		else
			m_recpMap = m_recpMap | (1 << (m_lastPkt - lastPkt));
	}
	else
	{
		m_recpMap = m_recpMap | 1;
		m_recpMap <<= lastPkt - m_lastPkt;
		m_lastPkt = lastPkt;
		m_update = true;
	}
	return true;
}

bool
IPCopeNeighbor::DoUpdate(uint32_t lastPkt, uint8_t map)
{
	NS_LOG_FUNCTION(this<<lastPkt<<m_lastPkt<<map<<m_recpMap);
	if(lastPkt < m_lastPkt)
	{
		if(lastPkt < m_lastPkt - 7)
			return false;
		else
		{
			m_recpMap = m_recpMap | (1 << (m_lastPkt - lastPkt));
			m_recpMap = m_recpMap | (map << (m_lastPkt - lastPkt));
		}
	}
	else
	{
		m_recpMap = m_recpMap | 1;
		m_recpMap <<= lastPkt - m_lastPkt;
		m_recpMap = map | m_recpMap;
		m_lastPkt = lastPkt;
		m_update = true;
		NS_LOG_FUNCTION(this<<"m_update is set to true");
	}
	return true;
}

bool
IPCopeNeighbor::IsUpdated() const
{
	NS_LOG_FUNCTION(this<<m_update);
	return m_update;
}

void
IPCopeNeighbor::ResetUpdate()
{
	m_update = false;
}
*/

bool
IPCopeNeighbor::HasIP(const Ipv4Address & ip) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); iter++)
	{
		if(iter->ip == ip)
			return true;
	}
	return false;

}

bool
IPCopeNeighbor::HasMac(const Mac48Address & mac) const
{
	NS_LOG_FUNCTION(this<<mac);
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); iter++)
	{
		if(iter->mac == mac)
			return true;
	}
	return false;
}

/*
void
IPCopeNeighbor::AddChannel(uint16_t channel)
{
	NS_LOG_FUNCTION(this<<channel);
	if(m_channels.find(channel) == m_channels.end())
		m_channels.insert(channel);
}
*/

void
IPCopeNeighbor::AddTrinity(AddressPair pair)
{
	AddTrinity(pair.ip, pair.mac, pair.channel);
}

void
IPCopeNeighbor::AddSoftTrinity(AddressPair pair)
{
	AddSoftTrinity(pair.ip, pair.mac, pair.channel);
}

void
IPCopeNeighbor::AddSoftTrinity(const Ipv4Address & ip, const Mac48Address & mac, uint16_t channel)
{
	NS_LOG_FUNCTION(this<<ip<<mac<<channel);
	AddressPair trinity = {ip, mac, channel};
	/*
	std::list<AddressPair>::iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); )
	{
		if (iter->ip == ip)
			iter = m_addressPairs.erase(iter);
		else if(iter->mac == mac)
			iter = m_addressPairs.erase(iter);
		else if (iter->channel == channel)
			iter = m_addressPairs.erase(iter);
		else iter++;
	}
	*/
	m_addressPairs.push_back(trinity);
	std::cout<<this;
	fflush(stdout);

}

void
IPCopeNeighbor::AddTrinity(const Ipv4Address & ip, const Mac48Address & mac, uint16_t channel)
{
	NS_LOG_FUNCTION(this<<ip<<mac<<channel);
	AddressPair trinity = {ip, mac, channel};
	std::list<AddressPair>::iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); )
	{
		if (iter->ip == ip)
			iter = m_addressPairs.erase(iter);
		else if(iter->mac == mac)
			iter = m_addressPairs.erase(iter);
		else if (iter->channel == channel)
			iter = m_addressPairs.erase(iter);
		else iter++;
	}
	m_addressPairs.push_back(trinity);
	std::cout<<this;
	fflush(stdout);
}

std::set<uint16_t>
IPCopeNeighbor::GetChannels() const
{
	std::set<uint16_t> channelSet;
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter != m_addressPairs.end(); iter++)
		channelSet.insert(iter->channel);
	return channelSet;
}

/*****************************************
 * IPCopeNeighbors
 * **************************************/

/*
bool
IPCopeNeighbors::ShareMac(const IPCopeNeighbor & neighbor1, const IPCopeNeighbor & neighbor2) const
{
	std::deque<Mac48Address> macs = neighbor1.GetMacs();
	std::deque<Mac48Address>::const_iterator iter;
	for(iter = macs.begin(); iter != macs.end(); iter++)
	{
		if(neighbor2.HasMac(*iter))
			return true;
	}
	return false;
}
*/

bool
IPCopeNeighbors::ShareAddress(const IPCopeNeighbor & neighbor1, const IPCopeNeighbor & neighbor2) const
{
	std::deque<Ipv4Address> ips = neighbor1.GetIPs();
	std::deque<Ipv4Address>::const_iterator ipIter;
	for(ipIter = ips.begin(); ipIter != ips.end(); ipIter++)
	{
		if (ipIter->IsEqual("0.0.0.0"))
			continue;
		if(neighbor2.HasIP(*ipIter))
			return true;
	}
	std::deque<Mac48Address> macs = neighbor1.GetMacs();
	std::deque<Mac48Address>::const_iterator maciter;
	for(maciter = macs.begin(); maciter != macs.end(); maciter++)
	{
		if(neighbor2.HasMac(*maciter))
			return true;
	}
	return false;
}

bool
IPCopeNeighbors::AddNeighbor(const IPCopeNeighbor& neighbor)
{
	NS_LOG_FUNCTION_NOARGS();
	std::deque<IPCopeNeighbor>::const_iterator iter;
	for(iter = m_neighbors.begin(); iter != m_neighbors.end(); iter++)
	{
		if(ShareAddress (neighbor, *iter))
		{
			NS_LOG_FUNCTION(this<<"Share address");
			return false;
		}
	}
	m_neighbors.push_back(neighbor);
	return true;
}

/*
bool
IPCopeNeighbors::AddNeighbor(IPCopeNeighbor& neighbor, Ipv4Address ipAddr, Ipv4Mask mask)
{
	NS_LOG_FUNCTION(this<<ipAddr);
	if(!ipAddr.IsMulticast() && !ipAddr.IsBroadcast() && !ipAddr.IsLocalMulticast() && !ipAddr.IsSubnetDirectedBroadcast(mask))
	{
		neighbor.AddIP(ipAddr);
		m_neighbors.push_back(neighbor);
		return true;
	}
	else
		return false;
}
*/

void
IPCopeNeighbors::RemoveNeighbor(const Ipv4Address & ip)
{
	std::deque<IPCopeNeighbor>::iterator iter;
	for(iter = m_neighbors.begin(); iter!=m_neighbors.end(); iter++)
		if(iter->HasIP(ip))
			m_neighbors.erase(iter);
}

void
IPCopeNeighbors::RemoveNeighbor(NeighborIterator iter)
{
	m_neighbors.erase(iter);
}

int32_t
IPCopeNeighbors::SearchNeighbor(const Ipv4Address & ip) const
{
	NS_LOG_FUNCTION_NOARGS();
	for(int32_t i = 0; i<(int)m_neighbors.size(); i++)
	{
		if(m_neighbors[i].HasIP(ip))
			return i;
	}
	return -1;
}

int32_t
IPCopeNeighbors::SearchNeighbor(const Mac48Address & mac) const
{
	NS_LOG_FUNCTION(this<<mac<<m_neighbors.size());
	std::deque<IPCopeNeighbor>::const_iterator neighborIter;
	int32_t pos = 0;
	for(neighborIter = m_neighbors.begin(); neighborIter != m_neighbors.end(); neighborIter++)
	{
		bool found = neighborIter->HasMac(mac);
		if(found)
		{
			std::cout<<(*neighborIter);
			return pos;
		}
		else
			pos++;
	}
	return -1;
}

void
IPCopeNeighbors::NeighborLearn(const IPCopeHello & hello)
{
	NS_LOG_FUNCTION_NOARGS();
	uint8_t pairNum = hello.GetLength();
	if(!pairNum)
		return;
	IPCopeNeighbor neighbor;
	for(uint8_t i = 0; i<pairNum; i++)
	{
		AddressPair pair = hello.Get(i);
		int32_t searchMac = SearchNeighbor(pair.mac);
		if(searchMac > -1)
		{
			NeighborIterator iter = At(searchMac);
			while(iter->GetVirtualQueueEntry())
				neighbor.AddVirtualQueueEntry(iter->RemoveVirtualQueueEntry());
			RemoveNeighbor(iter);
		}
		int32_t searchIp = SearchNeighbor(pair.ip);
		if(searchIp> -1)
		{
			NeighborIterator iter = At(searchIp);
			while(iter->GetVirtualQueueEntry())
				neighbor.AddVirtualQueueEntry(iter->RemoveVirtualQueueEntry());
			RemoveNeighbor(iter);
		}
		neighbor.AddTrinity(pair);
		/*
		neighbor.AddMac(pair.mac);
		neighbor.AddIP(pair.ip);
		neighbor.AddChannel(pair.channel);
		*/
	}
	AddNeighbor(neighbor);
}

std::list<AddressPair>
IPCopeNeighbor::GetTrinities() const
{
	return m_addressPairs;
}

Mac48Address
IPCopeNeighbor::Index(const uint16_t channel) const
{
	std::list<AddressPair>::const_iterator iter;
	for(iter = m_addressPairs.begin(); iter!= m_addressPairs.end(); iter++)
	{
		if(iter->channel == channel)
			return iter->mac;
	}
	NS_FATAL_ERROR("I think we should be able to index");
}

IPCopeNeighbors::NeighborIterator
IPCopeNeighbors::SMNeighbors(const Ipv4Address & ip, const Mac48Address & mac, const uint16_t channel)
{
	NS_LOG_FUNCTION(this<<ip<<mac);
	int32_t searchIp = SearchNeighbor(ip);
	int32_t searchMac = SearchNeighbor(mac);
	NeighborIterator iter;
	if (searchIp > -1 && searchMac < 0)
	{
		NS_LOG_LOGIC("found ip but not mac");
		iter = At(searchIp);
		iter->AddSoftTrinity(ip, mac, channel);
	}
	else if (searchIp < 0 && searchMac > -1)
	{
		NS_LOG_LOGIC("found mac but not ip");
		iter = At(searchMac);
		iter->AddTrinity(ip, mac, channel);
	}
	else if(searchIp > -1 && searchMac > -1)
	{
		NS_LOG_LOGIC("found both");
		if(searchIp == searchMac)
		{
			iter = At(searchIp);
			//iter->AddTrinity(ip, mac, channel);
		}
		else //merge 2 neighbors
		{
			NS_LOG_LOGIC(this<<"Merge");
			iter = At(searchIp);
			NeighborIterator iter2 = At(searchMac);
			iter2->AddTrinity(ip, mac, channel);
			std::list<AddressPair> trinities = iter2->GetTrinities();
			std::list<AddressPair>::const_iterator tri_iter;
			for(tri_iter = trinities.begin(); tri_iter != trinities.end(); tri_iter++)
				iter->AddSoftTrinity(*tri_iter);
			while(iter2->GetVirtualQueueEntry())
				iter->AddVirtualQueueEntry(iter2->RemoveVirtualQueueEntry());
			RemoveNeighbor(iter2);
			searchIp = SearchNeighbor(ip);
			iter = At(searchIp);
		}
	}
	else //new neighbor
	{
		NS_LOG_LOGIC("found neither");
		IPCopeNeighbor neighbor;
		neighbor.AddTrinity(ip, mac, channel);
		if(!AddNeighbor(neighbor))
			NS_FATAL_ERROR("New neighbor but can't be added");
		else
			iter = At(SearchNeighbor(ip));
	}
	return iter;
}

/*
IPCopeNeighbors::NeighborIterator
IPCopeNeighbors::SMNeighbors(const Ipv4Address ip, const Mac48Address mac)
{
	NS_LOG_FUNCTION(this<<ip<<mac);
	int32_t searchIp = SearchNeighbor(ip);
	int32_t searchMac = SearchNeighbor(mac);
	NeighborIterator iter;
	if (searchIp > -1 && searchMac < 0)
	{
		NS_LOG_LOGIC("found ip but not mac");
		iter = At(searchIp);
		if(!iter->AddMac(mac))
			NS_FATAL_ERROR("Can't add mac");
	}
	else if (searchIp < 0 && searchMac > -1)
	{
		NS_LOG_LOGIC("found mac but not ip");
		iter = At(searchMac);
		iter->AddIP(ip);
	}
	else if(searchIp > -1 && searchMac > -1)
	{
		NS_LOG_LOGIC("found both");
		if(searchIp == searchMac)
			iter = At(searchIp);
		else //merge 2 neighbors
		{
			NS_LOG_LOGIC(this<<"Merge");
			iter = At(searchIp);
			NeighborIterator iter2 = At(searchMac);
			std::deque<Mac48Address> macs = iter2->GetMacs();
			std::deque<Mac48Address>::const_iterator mac_iter;
			for(mac_iter = macs.begin(); mac_iter != macs.end(); mac_iter++)
				if(!iter->AddMac(*mac_iter))
					NS_FATAL_ERROR("Can't add mac");
			std::deque<Ipv4Address> ips = iter2->GetIPs();
			std::deque<Ipv4Address>::const_iterator ip_iter;
			for(ip_iter = ips.begin(); ip_iter!=ips.end(); ip_iter++)
				iter->AddIP(*ip_iter);
			std::set<uint16_t> chs = iter2->GetChannels();
			std::set<uint16_t>::const_iterator ch_iter;
			for(ch_iter = chs.begin(); ch_iter != chs.end(); ch_iter++)
				iter->AddChannel(*ch_iter);
			while(iter2->GetVirtualQueueEntry())
				iter->AddVirtualQueueEntry(iter2->RemoveVirtualQueueEntry());
			RemoveNeighbor(iter2);
		}
	}
	else //new neighbor
	{
		NS_LOG_LOGIC("found neither");
		IPCopeNeighbor neighbor;
		neighbor.AddIP(ip);
		if (!neighbor.AddMac(mac))
			NS_FATAL_ERROR("Can't add mac");
		if(!AddNeighbor(neighbor))
			NS_FATAL_ERROR("New neighbor but can't be added");
		else
			iter = At(SearchNeighbor(ip));
	}
	return iter;
}
*/


//std::set<IPCopeNeighbor>
/*
std::deque<IPCopeNeighbor>
IPCopeNeighbors::GetIPCopeNeighborSet() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_neighbors;
}
*/

std::ostream &
operator<<(std::ostream & os, const IPCopeNeighbor& neighbor)
{
	std::list<AddressPair>::const_iterator iter;
	std::list<AddressPair> pairs = neighbor.GetTrinities();
	for(iter = pairs.begin(); iter != pairs.end(); iter++)
	{
		os<<"MAC: "<<iter->mac<<" channel: "<<iter->channel;
		os<<" IP: "<<iter->ip<<std::endl;
	}
	os.flush();
	return os;
}

IPCopeNeighbors::NeighborIterator
IPCopeNeighbors::At(int32_t pos)
{
	NS_LOG_FUNCTION_NOARGS();
	return m_neighbors.begin()+pos;
}

/*
void
IPCopeNeighbor::PrintVirtualQueue() const
{
	NS_LOG_FUNCTION(this<<"The length of the long queue is "<<m_virtualQueue.size());
	std::deque<IPCopeQueueEntry *>::const_iterator iter;
	for(iter = m_virtualQueue.begin(); iter != m_virtualQueue.end(); iter++)
	{
		NS_LOG_FUNCTION(this<<"printing all virtual queue entry: "<<(*iter)->GetDestMac()<<(*iter)->GetPacketId());
	}
}
*/

/*
uint32_t
IPCopeNeighbor::GetLastPkt() const
{
	NS_LOG_FUNCTION(this<<m_lastPkt);
	return m_lastPkt;
}

uint8_t
IPCopeNeighbor::GetRecpMap() const
{
	NS_LOG_FUNCTION(this<<m_recpMap);
	return m_recpMap;
}
*/

}//namespace cope
}//namespace ns3
