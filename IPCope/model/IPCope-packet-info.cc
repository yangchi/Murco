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

#include "IPCope-packet-info.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE("IPCopePacketInfo");

namespace ns3{
namespace ipcope{

IPCopePacketInfo::IPCopePacketInfo(){}
IPCopePacketInfo::~IPCopePacketInfo(){}

/*
uint32_t
IPCopePacketInfo::Hash(Ipv4Address address, uint32_t seq_no) const
{
	uint32_t addressInt = address.Get();
	uint64_t key = addressInt << 32 + seq_no;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}
*/

/*
double
IPCopePacketInfo::GetProbability(uint32_t packetId, Ipv4Address ip) const
{
}

double
IPCopePacketInfo::GetProbability(uint32_t packetId, Mac48Address mac) const
{}
*/
/*
double
IPCopePacketInfo::GetProbability(uint32_t packetId, Neighbor neighbor) const
{
	NS_LOG_FUNCTION_NOARGS();
	//ProbabilityDeque probDeque;
	Probability prob;
	std::map<uint32_t, Probability>::const_iterator iter = m_packetInfo.find(packetId);
	//std::map<uint32_t, ProbabilityDeque>::const_iterator iter = m_packetInfo.find(packetId);

	if(iter == m_packetInfo.end())
		return -1;
	else
		prob = (*iter).second;

	Probability::const_iterator probIter = prob.find(neighbor);
	if(probIter == prob.end())
		return -2;
	else
		return (*probIter).second;
}
*/

/*
void
IPCopePacketInfo::SetProbability(uint32_t packetId, Neighbor neighbor, double probability)
{
}
*/

bool
IPCopePacketInfo::GetItem(uint32_t packetId, const Mac48Address & add)
{
	NS_LOG_FUNCTION(this<<m_packetInfo.size()<<packetId<<add);
	//std::map<uint32_t, Mac48Address>::const_iterator iter = m_packetInfo.find(packetId);
	std::map<uint32_t, std::vector<Mac48Address> >::const_iterator iter = m_packetInfo.find(packetId);
	if(iter == m_packetInfo.end())
		return false;
	/*
	if (iter->second == add)
		return true;
	else
		return false;
		*/
	if(find(iter->second.begin(), iter->second.end(), add) == iter->second.end())
		return false;
	else return true;
}

void
IPCopePacketInfo::SetItem(uint32_t packetId, const Mac48Address & add)
{
	NS_LOG_FUNCTION(this<<packetId<<add<<m_packetInfo.size());
	//m_packetInfo.insert(std::make_pair(packetId, add));
	std::map<uint32_t, std::vector<Mac48Address> >::iterator iter = m_packetInfo.find(packetId);
	if(iter == m_packetInfo.end())
	{
		std::vector<Mac48Address> vec;
		vec.push_back(add);
		m_packetInfo.insert(std::make_pair(packetId, vec));
	}
	else
		if(find(iter->second.begin(), iter->second.end(), add) == iter->second.end())
			iter->second.push_back(add);
}

/*
void
IPCopePacketInfo::SetItem(IPCopeHeader header, Mac48Address add)
{
	std::map<Mac48Address, uint32_t> idnext = header.GetIdNexthops();
	std::map<Mac48Address, uint32_t>::const_iterator id_iter;
	for(id_iter = idnext.begin(); id_iter != idnext.end(); id_iter++)
	{
		m_packetInfo.insert(std::make_pair(id_iter->second, add));
	}
}
*/

}//namespace cope
}//namespace ns3
