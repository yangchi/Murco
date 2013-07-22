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

#ifndef COPENEIGHBOR_H
#define COPENEIGHBOR_H

#include "IPCope-header.h"
#include "ns3/log.h"
#include <deque>
#include "ns3/ipv4-address.h"
#include "ns3/wifi-mac-header.h"
#include "IPCope-queue.h"
#include <set>
#include <list>

namespace ns3{
namespace ipcope{

class IPCopeNeighbor
{
public:
	IPCopeNeighbor();
	//IPCopeNeighbor(Mac48Address mac);
	//IPCopeNeighbor(Ipv4Address ip);
	~IPCopeNeighbor();
	void AddVirtualQueueEntry(IPCopeQueueEntry * vqe);
	void AddVirtualQueueEntryFront(IPCopeQueueEntry * vqe);
	IPCopeQueueEntry* GetVirtualQueueEntry() const;
	IPCopeQueueEntry* RemoveVirtualQueueEntry();
	bool RemoveVirtualQueueEntry(uint32_t packetId);
	Ipv4Address GetIp() const ;
	//uint32_t AddIP(const Ipv4Address & addr);
	std::deque<Ipv4Address> GetIPs() const;
	bool HasMac(const Mac48Address & mac) const;
	bool HasIP(const Ipv4Address & ip) const;
	//bool AddMac(const Mac48Address & mac) ;
	std::deque<Mac48Address> GetMacs() const;
	Mac48Address GetMac() const;
	//bool operator< (const IPCopeNeighbor& neighbor) const;
	//bool operator== (const IPCopeNeighbor& neighbor) const;

	//void AddChannel(uint16_t channel);
	std::set<uint16_t> GetChannels() const;
	std::list<AddressPair> GetTrinities() const;
	void AddTrinity(const Ipv4Address & ip, const Mac48Address & mac, uint16_t channel);
	void AddTrinity(AddressPair pair);
	void AddSoftTrinity(const Ipv4Address & ip, const Mac48Address & mac, uint16_t channel);
	void AddSoftTrinity(AddressPair pair);
	Mac48Address Index(const uint16_t channel) const;

private:
	void Init();
	friend std::ostream & operator<< (std::ostream & os, const IPCopeNeighbor& neighbor);
	/*
	std::deque<Ipv4Address> m_ips;
	std::deque<Mac48Address> m_macAddresses;
	std::set<uint16_t> m_channels;
	*/
	std::list<AddressPair> m_addressPairs;
	std::deque<IPCopeQueueEntry *> m_virtualQueue;
};

std::ostream & operator<< (std::ostream & os, const IPCopeNeighbor& neighbor);


class IPCopeNeighbors
{
public:
	IPCopeNeighbors();
	~IPCopeNeighbors();
	
	//typedef std::set<IPCopeNeighbor>::iterator NeighborIterator;
	typedef std::deque<IPCopeNeighbor>::iterator NeighborIterator;
	NeighborIterator SMNeighbors(const Ipv4Address & ip, const Mac48Address & mac, const uint16_t channel);
	//NeighborIterator SMNeighbors(const Ipv4Address ip, const Mac48Address mac);
	//NeighborIterator UpdateNeighbors(const Ipv4Address ip, const Mac48Address mac);
	bool AddNeighbor(const IPCopeNeighbor& neighbor);
	//bool AddNeighbor(IPCopeNeighbor& neighbor, Ipv4Address ipAddr, Ipv4Mask mask);
	//bool RemoveIPCopeNeighbor(const IPCopeNeighbor& neighbor);
	void RemoveNeighbor(const Ipv4Address & ip);
	void RemoveNeighbor(NeighborIterator iter);
	uint32_t Size() const { return m_neighbors.size(); }
	int32_t SearchNeighbor(const Ipv4Address & ip) const;
	int32_t SearchNeighbor(const Mac48Address & mac) const;
	NeighborIterator At(int32_t pos);

	void NeighborLearn(const IPCopeHello & hello);
	//std::deque<IPCopeNeighbor> GetIPCopeNeighborSet() const;
	

private:
	bool ShareAddress(const IPCopeNeighbor &, const IPCopeNeighbor &) const;
	
	//std::set<IPCopeNeighbor> m_neighbors;
	std::deque<IPCopeNeighbor> m_neighbors;
};

}//namespace cope
}//namespace ns3

#endif
