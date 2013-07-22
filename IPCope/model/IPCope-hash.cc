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

#include "IPCope-hash.h"
#include <string.h>
#include "ns3/log.h"
#include <iostream>
#include <stdlib.h>

namespace ns3{
namespace ipcope{

NS_LOG_COMPONENT_DEFINE("IPCopeHash");

/*
uint32_t Hash(Ipv4Address address, uint32_t seq_no) 
{
	uint32_t addressInt = address.Get();
	uint64_t key = addressInt;
	key = (key << 32) + seq_no;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}

uint32_t Hash(Mac48Address address, uint16_t seq_no) 
{
	uint8_t buffer[6];
	uint32_t addressInt;
	uint32_t seq = seq_no;
	//uint32_t addressInt = address.Get();
	address.CopyTo(buffer);
	for(int i = 0; i<4; i++)
		addressInt = (addressInt<<8) + buffer[i];
	for(int i = 4; i<6; i++)
		seq = (seq<<8) + buffer[i];
	uint64_t key = addressInt;
	key = (key << 32) + seq;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}
*/

uint32_t Hash(Ptr<Packet> packet)
{
	NS_LOG_FUNCTION_NOARGS();
	Ipv4Header ipHeader;
	Ptr<Packet> replica = packet->Copy();
	replica->RemoveHeader(ipHeader);
	uint32_t length = replica->GetSize();
	uint32_t size = length  + 4 - (length % 4);
	uint8_t *buffer = new uint8_t[size];
	memset(buffer, 0, size);
	replica->CopyData(buffer, size);
	uint32_t v1 = 0;
	uint32_t v2 = 0;
	for(uint32_t i = 0; i<=size-4; i++)
	{
		for(int j = 0; j<4; j++)
		{
			v1 = v1<<8;
			v1 = v1+buffer[i+j];
		}
		v2 = Hash(v1, v2);
	}
	NS_LOG_DEBUG(v2);
	return v2;
}

uint32_t Hash(uint32_t v1, uint32_t v2)
{
	uint64_t key = v1;
	key = (key << 32) + v2;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}

/*
uint32_t Hash(Ptr<Packet> packet)
{
	NS_LOG_FUNCTION_NOARGS();
	Ipv4Header ipHeader; //at this point we assume it's a Ipv4Header
	Ptr<Packet> replica = packet->Copy(); //the original packet should not be changed
	replica->RemoveHeader(ipHeader);
	replica->Print(std::cout);
	std::cout<<std::endl;
	fflush(stdout);
	uint32_t length = replica->GetSize();
	uint32_t size;
	if(length < 4)
		size = 4;
	else size = length;
	uint8_t *buffer = new uint8_t[size];
	memset(buffer, 0, size);
	replica->CopyData(buffer, size);

	for(uint32_t i = 0; i<size;i++)
	{
		std::cout<<(uint16_t)buffer[i]<<" ";
	}
	std::cout<<std::endl;
	fflush(stdout);
	
	MHASH hash_thread;
	hash_thread = mhash_init(MHASH_CRC32);
	if(hash_thread == MHASH_FAILED)
		NS_FATAL_ERROR("MHash doesn't work in ns-3?");
	mhash(hash_thread, &buffer, size);
	uint8_t *hash = new uint8_t[size];
	memset(hash, 0, size);
	mhash_deinit(hash_thread, hash);
	//only use the last 32 bits;
	uint32_t result = 0;
	for(int i = 0; i<4; i++)
	{
		result = result << 8;
		result += hash[4-i-1];
	}
	delete [] buffer;
	delete [] hash;
	return result;
}
*/

}
}
