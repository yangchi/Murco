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

#include "IPCope-packet-pool.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("IPCopePacketPool");

namespace ns3{
namespace ipcope{

IPCopePacketPool::IPCopePacketPool(){}
IPCopePacketPool::~IPCopePacketPool(){}

void
IPCopePacketPool::AddToPool(uint32_t pid, Ptr<Packet> pkt)
{
	NS_LOG_FUNCTION_NOARGS();
	m_pool.insert(std::make_pair(pid, pkt->Copy()));
}

/*
void
IPCopePacketPool::AddToPool(const uint32_t pid, const Ptr<Packet> packet, const uint16_t sequence)
{
	NS_LOG_FUNCTION_NOARGS();
	PacketSequence packetSeq  = {packet->Copy(), sequence};
	m_pool.insert(std::make_pair(pid, packetSeq));
}
*/

bool
IPCopePacketPool::Find(uint32_t pid, Ptr<Packet> & pkt) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::tr1::unordered_map<uint32_t, Ptr<Packet> >::const_iterator iter;
	iter = m_pool.find(pid);
	if(iter != m_pool.end())
	{
		NS_LOG_FUNCTION(this<<"Found it in pool");
		pkt = iter->second->Copy();
		NS_LOG_FUNCTION(this<<"Packet size: "<<pkt->GetSize());
		return true;
	}
	return false;
}

/*
 * return -1 if the pid is not found
 * otherwise, return sequence number
 */
/*
int32_t
IPCopePacketPool::Find(uint32_t pid, Ptr<Packet> & pkt) const
{
	NS_LOG_FUNCTION_NOARGS();
	int32_t seq = -1;
	std::tr1::unordered_map<uint32_t, PacketSequence>::const_iterator iter;
	iter = m_pool.find(pid);
	if(iter != m_pool.end())
	{
		NS_LOG_FUNCTION(this<<"Found it in pool");
		pkt = iter->second.packet->Copy();
		seq = iter->second.sequence;
	}
	return seq;
}
*/

}
}
