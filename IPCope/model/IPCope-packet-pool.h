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

#ifndef COPEPACKETPOOL_H
#define COPEPACKETPOOL_H

#include <tr1/unordered_map>
#include "ns3/packet.h"

namespace ns3{
namespace ipcope{

/*
struct PacketSequenceStruct
{
	Ptr<Packet> packet;
	uint16_t sequence;
};

typedef struct PacketSequenceStruct PacketSequence;
*/
class IPCopePacketPool
{
public:
	IPCopePacketPool();
	~IPCopePacketPool();
	void AddToPool(const uint32_t pid, const Ptr<Packet> pkt);
	//void AddToPool(const uint32_t pid, const Ptr<Packet> pkt, const uint16_t sequence);
	bool Find(uint32_t pid, Ptr<Packet> & pkt) const;
private:
	std::tr1::unordered_map<uint32_t, Ptr<Packet> > m_pool;
	//std::tr1::unordered_map<uint32_t, PacketSequence> m_pool;
};
}
}
#endif
