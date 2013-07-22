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

#ifndef COPEHASH_H
#define COPEHASH_H

#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/mac48-address.h"
#include "ns3/packet.h"
//#include <mhash.h>

namespace ns3{
namespace ipcope{

//uint32_t Hash(Mac48Address address, uint16_t seq_no);
//uint32_t Hash(Ipv4Address address, uint32_t seq_no);
uint32_t Hash(Ptr<Packet> packet);
uint32_t Hash(uint32_t v1, uint32_t v2);

}
}

#endif
