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

#include "IPCope-protocol.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NS_LOG_COMPONENT_DEFINE ("IPCopeProtocol");

namespace ns3{
namespace ipcope{

const uint16_t IPCopeProtocol::PROT_NUMBER = 0xF117;

IPCopeProtocol::IPCopeProtocol() :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try(Timer::CANCEL_ON_DESTROY), m_helloTimer(Timer::CANCEL_ON_DESTROY)
{
	m_rtimeout = MilliSeconds(25.0);
	//m_arq = true;
	Init();
}

/*
IPCopeProtocol::IPCopeProtocol(double rttime, double hello) :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try (Timer::CANCEL_ON_DESTROY), m_helloTimer(Timer::CANCEL_ON_DESTROY)
{
	m_rtimeout = MilliSeconds(rttime);
	m_helloInterval = Seconds(hello);
	//m_arq = true;
	Init();
}

IPCopeProtocol::IPCopeProtocol(bool multi, double rttime, double hello) :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try (Timer::CANCEL_ON_DESTROY), m_helloTimer(Timer::CANCEL_ON_DESTROY)
{
	m_rtimeout = MilliSeconds(rttime);
	m_helloInterval = Seconds(hello);
	m_mr = multi;
	Init();
}
*/

IPCopeProtocol::IPCopeProtocol(bool multi, double rttime, double retrytime, double hello) :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try (Timer::CANCEL_ON_DESTROY), m_helloTimer(Timer::CANCEL_ON_DESTROY)
{
	m_rtimeout = MilliSeconds(rttime);
	m_ttimeout = MilliSeconds(retrytime);
	m_helloInterval = Seconds(hello);
	m_mr = multi;
	Init();
}

IPCopeProtocol::~IPCopeProtocol()
{
}

TypeId
IPCopeProtocol::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::ipcope::IPCopeProtocol")
		.SetParent<Object> ();
	return tid;
}

void
IPCopeProtocol::Init()
{
	NS_LOG_FUNCTION_NOARGS();
	m_isSending = false;
	//m_ttimeout = MilliSeconds(m_rtimeout.GetMilliSeconds());
	m_maxReports = 10;
	m_timer.SetDelay(m_rtimeout);
	m_timer.SetFunction(&IPCopeProtocol::Retransmit, this);
	m_try.SetDelay(m_ttimeout);
	m_try.SetFunction(&IPCopeProtocol::TrySend, this);
	m_try.Schedule();
	m_helloTimer.SetDelay(m_helloInterval);
	m_helloTimer.SetFunction(&IPCopeProtocol::HelloTimerExpire, this);
}

bool
IPCopeProtocol::Enqueue(Ptr<Packet> packet, const Mac48Address& src, const Mac48Address& dest, const uint16_t protocolNumber, const uint32_t index, const MessageType type)
{
	NS_LOG_FUNCTION(this<<src<<dest<<m_queue.Size()<<index<<protocolNumber);
	IPCopeQueueEntry entry(packet);
	entry.SetSrcMac(src);
	entry.SetDestMac(dest);
	entry.SetProtocolNumber(protocolNumber);
	entry.SetIface(index);
	if(type == HELLO)
	{
		entry.SetHello();
		m_queue.EnqueueFront(entry);
	}
	else
	{
		Ipv4Header ipHeader;
		if(!packet->PeekHeader(ipHeader))
			NS_FATAL_ERROR(this<<"Can't peek ip header");
		entry.SetIpHeader(ipHeader);
		entry.SetNexthop();
		entry.SetIPSrc(ipHeader.GetSource());
		entry.SetPacketId(Hash(packet));
		entry.SetData();
		if (m_queue.EnqueueBack(entry))
		{
			NS_ASSERT(entry.GetPacketId() == (*(m_queue.LastPosition())).GetPacketId());
			IPCopeNeighbors::NeighborIterator neighborIter;
			if(!dest.IsBroadcast())
			{
				int32_t neighborPos = m_neighbors.SearchNeighbor(dest);
				if(neighborPos < 0)
				{
					IPCopeNeighbor tmpNeighbor;
					Ipv4Address ip("0.0.0.0");
					uint16_t channel = m_devices[index]->GetChannelNumber();
					tmpNeighbor.AddTrinity(ip, dest, channel);
					tmpNeighbor.AddVirtualQueueEntry(m_queue.LastPosition());
					if(m_neighbors.AddNeighbor(tmpNeighbor))
						NS_LOG_FUNCTION(this<<"neighbor added");
					else
						NS_LOG_FUNCTION(this<<"neighbor not added");
				}
				else
				{
					neighborIter = m_neighbors.At(neighborPos);
					neighborIter->AddVirtualQueueEntry(m_queue.LastPosition());
				}
			}
			m_pool.AddToPool(entry.GetPacketId(), packet);
		}
	}
	TrySend();
	return true;
}

void
IPCopeProtocol::TrySend()
{
	Ipv4Address ip_address = GetIP();
	NS_LOG_FUNCTION(this<<ip_address<<m_queue.Size()<<m_rtqueue.Size()<<m_try.GetDelay().GetMilliSeconds()<<Simulator::Now().GetSeconds());
	if(m_isSending)// we don't want to change m_devicesIf when m_isSending = true
		return TrySendSchedule();
	m_devicesIf.clear();
	for(uint32_t i = 0; i<m_devices.size(); i++)
	{
		Ptr<IPCopeDevice> device = m_devices[i];
		if (!device->IsQueueFull())
		{
			Mac48Address mac_address = Mac48Address::ConvertFrom(device->GetAddress());
			NS_LOG_FUNCTION("Queue not full: "<<device->GetMacQueueSize()<<" and out queue: "<<m_queue.Size());
			if (device->GetMacQueueSize() == 0 && m_queue.Size() > 0)
				NS_LOG_FUNCTION(this<<"Warning: losing bandwidth "<<ip_address<<mac_address);
			if (device->GetMacQueueSize() == 0 && m_queue.Size() > 1)
				NS_LOG_FUNCTION(this<<"Warning: losing serious bandwidth "<<ip_address<<mac_address);
			if (device->GetMacQueueSize() == 0 && m_queue.Size() > 2)
				NS_LOG_FUNCTION(this<<"Warning: losing very serious bandwidth "<<ip_address<<mac_address);
			m_devicesIf.push_back(i);
		}
	}
	if(m_devicesIf.size())
	{
		if(!m_isSending && m_queue.Size())
			DoSend();
	}
	else
	{
		NS_LOG_FUNCTION(this<<"all mac queues are full");
	}
	TrySendSchedule();
}

void
IPCopeProtocol::TrySendSchedule()
{
	m_try.Cancel();
	m_try.Schedule();
}

uint32_t
IPCopeProtocol::Index(uint16_t channel) const
{
	NS_LOG_FUNCTION_NOARGS();
	for(uint32_t i = 0; i<m_devices.size(); i++)
	{
		Ptr<IPCopeDevice> device = m_devices[i];
		if(device->GetChannelNumber() == channel)
			return i;
	}
	return m_devices.size();
}

uint32_t
IPCopeProtocol::Index(const Mac48Address & src) const
{
	NS_LOG_FUNCTION_NOARGS();
	for(uint32_t i = 0; i<m_devices.size(); i++)
	{
		Ptr<IPCopeDevice> device = m_devices[i];
		if(device->GetAddress() == src)
			return i;
	}
	return m_devices.size();
}

void
IPCopeProtocol::DoSend()
{
	Ipv4Address ip_addr = GetIP();
	NS_LOG_FUNCTION(this<<m_queue.Size()<<m_rtqueue.Size()<<m_ackBlockList.size()<<m_devicesIf.size()<<ip_addr);
	if(m_queue.Size() > 1)
		NS_LOG_FUNCTION(this<<"Has chance to encode");
	m_isSending = true;
	//IPCopeQueueEntry entry;
	Ptr<Packet> packet ;//= Create<Packet>();
	uint32_t outIface;
	int32_t neighborPos;

	if(!m_queue.Size())
		NS_FATAL_ERROR("queue size zero");

	IPCopeQueueEntry entry = m_queue.Front();
	//send hello msg
	if(entry.IsHello())
	{
		NS_LOG_LOGIC("Get entry and it's Hello");
		outIface = entry.GetIface();
		if(find(m_devicesIf.begin(), m_devicesIf.end(), outIface) == m_devicesIf.end())
		{
			NS_LOG_LOGIC("the index we want is not active "<<outIface);
		}
		else{
			m_queue.Dequeue();
			IPCopeType type(HELLO);
			packet = entry.GetPacket()->Copy();
			packet->AddHeader(type);
			m_devices[outIface]->ForwardDown(packet, entry.GetDestMac(), entry.GetProtocolNumber());
		}
		return DoSendEnd();
	}
	else
	{
		IPCopeHeader header;
		header.SetIp(GetIP());
		header.AddIdNexthop(entry.GetDestMac(), entry.GetPacketId());
		if(!entry.GetDestMac().IsBroadcast())
		{
			neighborPos = m_neighbors.SearchNeighbor(entry.GetDestMac());
			NS_ASSERT(neighborPos > -1);
			if (! m_neighbors.At(neighborPos)->RemoveVirtualQueueEntry(entry.GetPacketId()))
				NS_FATAL_ERROR("Removed failed");
			if(m_packetInfo.GetItem(entry.GetPacketId(), m_neighbors.At(neighborPos)->GetMac()))
			{
				m_queue.Dequeue();
				return DoSendEnd();
			}
		}

		m_queue.Dequeue();

		bool encoded = false;
		if(m_queue.Size() > 0 && !entry.GetDestMac().IsBroadcast())
			encoded = Encode(entry, packet, header);
		if(!encoded)
		{
			packet = entry.GetPacket()->Copy();
			outIface = entry.GetIface();
			if(find(m_devicesIf.begin(), m_devicesIf.end(), outIface) == m_devicesIf.end())
			{
				NS_LOG_LOGIC("the index we want is not active "<<outIface);
				m_queue.EnqueueFront(entry);
				if(!entry.GetDestMac().IsBroadcast())
				{
					neighborPos = m_neighbors.SearchNeighbor(entry.GetDestMac());
					NS_ASSERT(neighborPos > -1);
					m_neighbors.At(neighborPos)->AddVirtualQueueEntryFront(m_queue.FirstPosition());
				}
				return DoSendEnd();
			}
		}
		else
			outIface = entry.GetIface();

		//header.SetLocalPktSeqNum(entry.GetSequence());

		NS_LOG_FUNCTION(this<<"add report: "<<m_recps.size());
		if(m_recps.size())
		{
			if(m_recps.size() > m_neighbors.Size())
			//if(m_recps.size() > m_maxReports)
			{
				for(uint32_t i = 0; i<m_neighbors.Size(); i++)
				//for(int i = 0; i<m_maxReports; i++)
				{
					header.AddRecpReport(m_recps.front());
					m_recps.pop_front();
				}
				//header.SetReportNum(m_maxReports);
			}
			else
			{
				//uint16_t num = m_recps.size();
				while(m_recps.size())
				{
					header.AddRecpReport(m_recps.front());
					m_recps.pop_front();
				}
				//header.SetReportNum(num);

			}
		}

		//Add acks to header
		NS_LOG_LOGIC("Add "<<m_ackBlockList.size()<<" acks to header");
		//header.SetAckNum(m_ackBlockList.size());
		if(m_ackBlockList.size())
		{
			/*
			NS_LOG_LOGIC("Ack added "<<m_ackBlockList.size());
			header.AddAckBlock(m_ackBlockList);
			m_ackBlockList.clear();
			*/
			if(m_ackBlockList.size() > m_neighbors.Size())
			{
				for(uint32_t i = 0; i<m_neighbors.Size(); i++)
				{
					header.AddAckBlock(m_ackBlockList.back());
					m_ackBlockList.pop_back();
				}
			}
			else
			{
				while(m_ackBlockList.size())
				{
					header.AddAckBlock(m_ackBlockList.back());
					m_ackBlockList.pop_back();
				}
			}
		}

		//forward down
		packet->AddHeader(header);
		IPCopeType typeHeader(DATA);
		packet->AddHeader(typeHeader);

		NS_LOG_FUNCTION(this<<"WifiNetDevice about to send:");
		packet->Print(std::cout);
		std::cout<<std::endl;
		fflush(stdout);

		m_devices[outIface]->ForwardDown(packet, entry.GetDestMac(), entry.GetProtocolNumber());
		DoSendEnd();
	}
}

void
IPCopeProtocol::DoSendEnd()
{
	NS_LOG_FUNCTION_NOARGS();
	if(m_timer.GetDelayLeft().IsZero() || !m_timer.IsRunning())
		m_timer.Schedule();
	m_isSending = false;
	//TrySend();
}

void
IPCopeProtocol::Recv(Ptr<NetDevice> netDevice, Ptr<const Packet> pkt, uint16_t protocol, const Address & sender, const Address & receiver, NetDevice::PacketType packetType, uint32_t index)
{
	Ipv4Address ip_address = GetIP();
	NS_LOG_FUNCTION(this<<m_queue.Size()<<netDevice->GetAddress()<<sender<<receiver<<ip_address);
	pkt->Print(std::cout);
	std::cout<<std::endl;
	fflush(stdout);
	uint32_t pid;
	Mac48Address sMac = Mac48Address::ConvertFrom(sender);
	Mac48Address destMac = Mac48Address::ConvertFrom(receiver);
	Mac48Address myMac = Mac48Address::ConvertFrom(netDevice->GetAddress());
	Ipv4Header ipHeader;
	IPCopeType typeHeader;
	uint16_t channelNumber;

	if( find(m_macs.begin(), m_macs.end(), sMac) != m_macs.end())
		return;

	Ptr<Packet> packet = pkt->Copy();
	packet->RemoveHeader(typeHeader);
	if(typeHeader.IsHello())
	{
		IPCopeHello helloHeader;
		packet->RemoveHeader(helloHeader);
		NS_LOG_LOGIC("Hello header removed");
		m_neighbors.NeighborLearn(helloHeader);
		TrySend();
		return;
	}
	else
	{
		IPCopeHeader header;
		packet->RemoveHeader(header);
		uint32_t encodedNum = header.GetEncodedNum();
		if((encodedNum == 1)  && header.AmINext(m_macs, pid) && !header.AmINext(myMac, pid))
			return;
		Ipv4Address ipAddr = header.GetIp();

		//update neighbor
		channelNumber = m_devices[index]->GetChannelNumber();
		IPCopeNeighbors::NeighborIterator neighborIter = m_neighbors.SMNeighbors(ipAddr, sMac, channelNumber);

		//update ack
		AckBlock ackBlock;
		std::vector<uint32_t> pids = header.Search(header.GetAckBlocks(), GetIP());
		std::vector<uint32_t>::const_iterator pidsIter;
		NS_LOG_FUNCTION(this<<"pids we retrieve from Search: "<<pids.size());
		for(pidsIter = pids.begin(); pidsIter != pids.end(); pidsIter++)
		{
			m_rtqueue.Erase(*pidsIter);
		}

		//Update packetInfo based on all acks in this header:
		std::vector<AckBlock> allAcks = header.GetAckBlocks();
		std::vector<AckBlock>::const_iterator allAckIter;
		for(allAckIter = allAcks.begin(); allAckIter != allAcks.end(); allAckIter++)
		{
			m_packetInfo.SetItem(allAckIter->pid, neighborIter->GetMac());
		}

		//update packet info based on recp report
		uint16_t reportNum = header.GetReportNum();
		if(reportNum)
		{
			std::vector<uint32_t> recpReports = header.GetRecpReports();
			NS_ASSERT(reportNum == recpReports.size());
			NS_LOG_FUNCTION("Assert passed"<<reportNum);
			std::vector<uint32_t>::const_iterator iter;
			for(iter = recpReports.begin(); iter != recpReports.end(); iter++)
			{
				pid = *iter;
				NS_LOG_FUNCTION(this<<pid);
				m_packetInfo.SetItem(pid, neighborIter->GetMac());
			}
		}
		NS_LOG_FUNCTION("Loop passed");

		if(encodedNum)
		{
			if(encodedNum > 1)
			{
				NS_LOG_LOGIC("It is encoded.");
				//Ptr<Packet> temppacket = pkt->Copy();

				int64_t isDecodable = Decode(header, packet);//, sequence);
				if(isDecodable >= 0 )
				{
					packet->Print(std::cout);
					std::cout<<std::endl;
					fflush(stdout);
					packet->PeekHeader(ipHeader);
					ipHeader.Print(std::cout);
					std::cout<<std::endl;
					fflush(stdout);
					packet->RemoveHeader(ipHeader);
					ipHeader.SetTtl(64);
					packet->AddHeader(ipHeader);
					if (header.IsBroadcast())
					{
						NS_LOG_LOGIC("Encoded and Broadcast");
						m_devices[index]->ForwardUp(packet, protocol, sMac, destMac, packetType);
					}
					else
					{
						if(header.AmINext(m_macs, pid))
						{
							NS_ASSERT((isDecodable - pid) == 0);
							NS_ASSERT(pid == Hash(packet));
							NS_LOG_LOGIC("I am next hop");
							AckBlock ackBlock;
							ackBlock.address = ipAddr;
							ackBlock.pid = pid;
							AddAck(ackBlock);
							m_devices[index]->ForwardUp(packet, protocol, sMac, destMac, packetType);
						}
						else
						{
							NS_LOG_LOGIC("I'm not nexthop.");
							pid = Hash(packet);
						}
						m_recps.push_back(pid);
						m_pool.AddToPool(pid, packet);
						m_packetInfo.SetItem(pid, neighborIter->GetMac());
					}
				}
				else{
					NS_LOG_FUNCTION(this<<"Warning: recved encoded pkts but unable to decode");
				}
			}
			else
			{
				packet->PeekHeader(ipHeader);
				NS_LOG_LOGIC("It's not encoded.");
				if (header.IsBroadcast())
				{
					m_devices[index]->ForwardUp(packet, protocol, sMac, destMac, packetType);
				}
				else
				{
					if(header.AmINext(m_macs, pid))
					{
						NS_LOG_LOGIC("I am next hop");
						NS_ASSERT(pid == Hash(packet));
						m_devices[index]->ForwardUp(packet, protocol, sMac, destMac, packetType);
					}
					else
					{
						NS_LOG_LOGIC("No, i'm not");
						pid = Hash(packet);
					}
					m_recps.push_back(pid);
					m_pool.AddToPool(pid, packet);
					m_packetInfo.SetItem(pid, neighborIter->GetMac());
				}
			}
		}
		return TrySend();
	}
	TrySend();
}

/*
 * \returns -1 if we need more packets to decode it. 0 if we have every packet. a positive pid if it's decodable and it's decoded.
 */
int64_t
IPCopeProtocol::Decode(const IPCopeHeader & header, Ptr<Packet> & packet)
{
	NS_LOG_FUNCTION(this);
	uint32_t pid = 0;

	std::map<Mac48Address, uint32_t> encodedInfo = header.GetIdNexthops();
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	uint8_t missing = 0;
	uint8_t found = 0;
	for(iter = encodedInfo.begin(); iter != encodedInfo.end(); iter++)
	{
		Ptr<Packet> foundPkt;
		if(!m_pool.Find(iter->second, foundPkt))
		{
			if(++missing > 1)
			{
				NS_LOG_FUNCTION(this<<"Decoding failed: more than one pkt missing");
				return -1;
			}
			pid = iter->second;
		}
		else //find this pid, XOR it with the coded mess
		{
			NS_LOG_FUNCTION(this<<"found in pool: "<<iter->second<<foundPkt->GetSize());
			if(++found < encodedInfo.size())
				packet = XOR(foundPkt, packet);
			else
				break;
		}
	}
	NS_LOG_FUNCTION(this<<"after loop"<<(uint16_t)missing<<(uint16_t)found<<encodedInfo.size());
	if(missing == 0 && found  == encodedInfo.size())
	{
		NS_LOG_FUNCTION(this<<"Decoding failed: all pkts found");
		return -2;
	}
	else
	{
		NS_LOG_FUNCTION(this<<"Decoding succeeded."<<pid);
		return pid;
	}
}

void
IPCopeProtocol::Retransmit()
{
	if(!m_rtqueue.Size())
		return;
	NS_LOG_FUNCTION(this<<m_rtqueue.Size());
	IPCopeQueueEntry entry = m_rtqueue.Dequeue();
	entry.Retry();
	if (m_queue.EnqueueFront(entry))
	{
		NS_ASSERT((*(m_queue.FirstPosition())).GetPacketId() == entry.GetPacketId());
		int32_t neighborPos = m_neighbors.SearchNeighbor(entry.GetDestMac());
		NS_ASSERT(neighborPos >= 0);
		IPCopeNeighbors::NeighborIterator neighborIter = m_neighbors.At(neighborPos);
		neighborIter->AddVirtualQueueEntryFront(m_queue.FirstPosition());
	}
	m_timer.Schedule();
	TrySend();
}

/*
bool
IPCopeProtocol::GetChannelNumber(Mac48Address & mac, uint16_t & channel) const
{
	for(uint32_t i = 0; i< m_neighborNICs.size(); i++)
	{
		if(m_neighborNICs[i].mac == mac)
		{
			channel = m_neighborNICs[i].channel;
			return true;
		}
	}
	return false;
}
*/

bool
IPCopeProtocol::Encode(IPCopeQueueEntry & entry, Ptr<Packet> & packet, IPCopeHeader & copeHeader)
{
	Ipv4Address ip_address = GetIP();
	NS_LOG_FUNCTION(this<<m_queue.Size()<<ip_address);
	IPCopeQueueEntry newEntry = entry;
	std::set<Mac48Address> m_nexthops;
	std::set<uint32_t> m_natives;
	std::set<uint16_t> channels;
	IPCopeNeighbors::NeighborIterator neighborIter;
	bool isEncoded = false;
	IPCopeQueueEntry* virtualQueueEntry;
	uint32_t packetId = entry.GetPacketId();
	bool capable = true;
	//uint16_t channel;

	int32_t neighborPos = m_neighbors.SearchNeighbor(entry.GetDestMac());
	std::cout<<(*m_neighbors.At(neighborPos));
	std::cout<<std::endl;
	fflush(stdout);

	if(neighborPos < 0)
		return false;

	neighborIter = m_neighbors.At(neighborPos);
	if(m_packetInfo.GetItem(packetId, neighborIter->GetMac()))
		return false;
	m_nexthops.insert(neighborIter->GetMac());
	m_natives.insert(packetId);
	channels = neighborIter->GetChannels();

	uint32_t neighborSize = m_neighbors.Size();
	for(uint32_t i= 0; i<neighborSize; i++)
	{
		neighborIter = m_neighbors.At(i);
		capable = true;
		NS_LOG_FUNCTION(this<<"Actually in the loop");
		std::set<Ipv4Address>::const_iterator iter;
		if(m_nexthops.find(neighborIter->GetMac()) != m_nexthops.end())
			continue;

		virtualQueueEntry = neighborIter->GetVirtualQueueEntry();
		if(!virtualQueueEntry)
			continue;
		if(m_packetInfo.GetItem(virtualQueueEntry->GetPacketId(), neighborIter->GetMac()))
			continue;
		NS_LOG_DEBUG("packet id retrived through virtual queue entry: "<<virtualQueueEntry->GetPacketId());

		if(virtualQueueEntry->GetDestMac().IsBroadcast())
			NS_FATAL_ERROR("virtual queue entry from neighbor shouldn't have broadcast addr as dest");

		std::set<Mac48Address>::const_iterator iterNexthops;
		for(iterNexthops = m_nexthops.begin(); iterNexthops != m_nexthops.end(); iterNexthops++)
		{
			NS_LOG_FUNCTION(this<<"In the first inner loop to check if nexthops have the new pkt");
			if(!m_packetInfo.GetItem(virtualQueueEntry->GetPacketId(), *iterNexthops))
			{
				capable = false;
				break;
			}
		}
		if(!capable)
			continue;
		std::set<uint32_t>::iterator id_iter;
		for(id_iter = m_natives.begin(); id_iter !=m_natives.end(); id_iter++)
		{
			NS_LOG_FUNCTION(this<<"In the second inner loop to check if this neighbor has all natives");
			if(!m_packetInfo.GetItem(*id_iter, neighborIter->GetMac()))
			{
				capable = false;
				break;
			}
		}
		if(!capable)
			continue;

		NS_LOG_FUNCTION("Successfully passed first two tests");
		//examine channel intersection
		std::set<uint16_t> tempChannel = neighborIter->GetChannels();
		std::set<uint16_t> intersection;
		set_intersection(channels.begin(), channels.end(), tempChannel.begin(), tempChannel.end(), std::insert_iterator<std::set<uint16_t> > (intersection, intersection.begin()));
		if(!intersection.size())
		{
			NS_LOG_FUNCTION(this<<"Can't encode due to channel");
			capable = false;
		}
		else{
			channels = intersection;
		}

		//newEntry.SetChannel(*(intersection.begin()));
		std::set<uint16_t>::const_iterator channel_iter;
		bool hasInterface = false;
		for(channel_iter = channels.begin(); channel_iter != channels.end(); channel_iter++)
		{
			uint32_t index = Index(*channel_iter);
			if(find(m_devicesIf.begin(), m_devicesIf.end(), index) != m_devicesIf.end())
			{
				hasInterface = true;
				newEntry.SetIface(index);
				newEntry.SetDestMac(neighborIter->Index(*channel_iter));
			}
		}
		if(!hasInterface)
			capable = false;
		if(!capable)
			continue;

		NS_LOG_DEBUG("Before enter XOR:");

		newEntry.SetPacket(XOR(entry.GetPacket(), (*virtualQueueEntry).GetPacket()));

		m_nexthops.insert(neighborIter->GetMac());
		m_natives.insert(virtualQueueEntry->GetPacketId());
		IPCopeQueueEntry rte = *virtualQueueEntry;
		if(!rte.HitMax())
			m_rtqueue.EnqueueBack(rte);
		isEncoded = true;
		NS_LOG_FUNCTION(this<<"ENCODED!!"<<Simulator::Now().GetSeconds());
		if (!copeHeader.AddIdNexthop(virtualQueueEntry->GetDestMac(), virtualQueueEntry->GetPacketId()))
			NS_FATAL_ERROR("IdNexthop not added "<<virtualQueueEntry->GetDestMac());
		if (! m_queue.Erase(virtualQueueEntry->GetPacketId()))
			NS_FATAL_ERROR("Failed to erase from queue");
		neighborIter->RemoveVirtualQueueEntry();

		capable = true;
	}
	packet = newEntry.GetPacket()->Copy();
	
	if(isEncoded)
		if(!entry.HitMax())
			m_rtqueue.EnqueueBack(entry);
	entry = newEntry;
	return isEncoded;
}

/*
void
IPCopeProtocol::AddNeighborNIC(Mac48Address mac, uint16_t channel)
{
	bool found = false;
	int32_t neighborPos = m_neighbors.SearchNeighbor(mac);
	if(neighborPos < 0)
	{
		IPCopeNeighbor tempNeighbor(mac);
		tempNeighbor.AddChannel(channel);
		m_neighbors.AddNeighbor(tempNeighbor);
	}
	NIC nic = {mac, channel};
	for(uint32_t i = 0; i< m_neighborNICs.size(); i++)
	{
		if(m_neighborNICs[i].mac == mac)
		{
			m_neighborNICs[i].channel = channel;
			found = true;
			break;
		}
	}
	if(!found)
		m_neighborNICs.push_back(nic);
}
*/

Ptr<Packet>
IPCopeProtocol::XOR(Ptr<const Packet> p1, Ptr<const Packet> p2)
{
	NS_LOG_FUNCTION(this<<p1->GetSize()<<p2->GetSize());
	uint32_t len1 = p1->GetSize(), len2 = p2->GetSize();
	uint32_t big, small;

	big = (len1 > len2) ? len1 : len2;
	small = (len1 > len2) ? len2: len1;
	
	uint8_t *buffer = new uint8_t[big];
	uint8_t *buf1 = new uint8_t[len1];
	uint8_t *buf2 = new uint8_t[len2];

	memset(buffer, 0, big);
	memset(buf1, 0, len1);
	memset(buf2, 0, len2);

	NS_LOG_DEBUG("Before copy");
	p1->CopyData(buf1, len1);
	p2->CopyData(buf2, len2);

	for(uint32_t i = 0; i<small; i++)
		buffer[i] = buf1[i]^buf2[i];
	for(uint32_t i = small; i<big; i++)
	{
		if(len1 > len2)
			buffer[i] = buf1[i]^0;
		else 
			buffer[i] = buf2[i]^0;
	}
	NS_LOG_DEBUG("Before XOR");
	uint32_t true_len= big-1;
	while(!buffer[true_len])
		true_len--;
	Ptr<Packet> packet = Create<Packet>(buffer, true_len+1);
	delete [] buffer;
	delete [] buf1;
	delete [] buf2;
	NS_LOG_FUNCTION(this<<packet->GetSize());
	return packet;
}

std::vector<Ptr<IPCopeDevice> >
IPCopeProtocol::GetDevices() const
{
	return m_devices;
}

Ptr<IPCopeDevice>
IPCopeProtocol::GetDevice(uint32_t index) const
{
	NS_LOG_FUNCTION(this<<index<<m_devices.size());
	return m_devices[index];
}

uint32_t
IPCopeProtocol::CreateNDevices(uint32_t n)
{
	for(uint32_t i = 0; i<n; i++)
	{
		Ptr<IPCopeDevice> copeDevice = CreateObject<IPCopeDevice>();
		copeDevice->SetCopeIfIndex(m_devices.size());
		//copeDevice->SetIfIndex(m_devices.size());
		copeDevice->SetCopeProtocol(this);
		copeDevice->SetNode(m_node);
		m_devices.push_back(copeDevice);
	}
	NS_LOG_FUNCTION(this<<m_devices.size());
	return m_devices.size();
}

void 
IPCopeProtocol::SetIpv4Mask(const Ipv4Mask & mask)
{
	m_mask = mask;
}

bool
IPCopeProtocol::IsUnicast(const Ipv4Address & ipaddr, const Ipv4Mask & mask) const
{
	NS_LOG_FUNCTION_NOARGS();
	if(!ipaddr.IsMulticast() && !ipaddr.IsBroadcast() && !ipaddr.IsLocalMulticast() && !ipaddr.IsSubnetDirectedBroadcast(mask))
		return true;
	else
		return false;

}

bool
IPCopeProtocol::IsUnicast(const Ipv4Address & ipaddr) const
{
	return IsUnicast(ipaddr, m_mask);
}

void
IPCopeProtocol::AddIP(Ipv4Address & ip)
{
	NS_LOG_FUNCTION_NOARGS();
	if(find(m_ips.begin(), m_ips.end(), ip) == m_ips.end())
	{
		NS_LOG_LOGIC("add ip");
		m_ips.push_back(ip);
	}
}

/*
void
IPCopeProtocol::AddAddressPair(Ipv4Address & ip, Mac48Address & mac, uint16_t channel)
{
	NS_LOG_FUNCTION_NOARGS();
	AddressPair addP = {ip, mac, channel};
	m_addPair.push_back(addP);
}
*/

Ipv4Address
IPCopeProtocol::GetIP(uint32_t index) const
{
	if(index > m_ips.size())
		NS_FATAL_ERROR("Can't get IP for index "<<index);
	return m_ips[index];
}

Ipv4Address
IPCopeProtocol::GetIP() const
{
	NS_LOG_FUNCTION_NOARGS();
	if(!m_ips.size())
		return NULL;
	std::vector<Ipv4Address>::const_iterator iter;
	Ipv4Address min = m_ips[0];
	for(iter = m_ips.begin(); iter != m_ips.end(); iter++)
	{
		if(*iter < min)
			min = (*iter);
	}
	return min;

}

/*
void
IPCopeProtocol::AddAck(AckBlock ack)
{
	bool found = false;
	std::vector<AckBlock>::iterator iter;
	for(iter = m_ackBlockList.begin(); iter != m_ackBlockList.end(); iter++)
	{
		if(iter->address == ack.address)
		{
			NS_ASSERT(!found);
			found = true;
			if(iter->lastAck <= ack.lastAck)
			{
				iter->lastAck = ack.lastAck;
				iter->ackMap = ack.ackMap;
			}
		}
	}
	if(!found)
		m_ackBlockList.push_back(ack);
}
*/

void
IPCopeProtocol::AddAck(AckBlock ack)
{
	NS_LOG_FUNCTION(this<<m_ackBlockList.size());
	m_ackBlockList.push_back(ack);
	NS_LOG_FUNCTION(this<<m_ackBlockList.size());
}

void
IPCopeProtocol::SetNode(Ptr<Node> node)
{
	m_node = node;
}

Ptr<Node>
IPCopeProtocol::GetNode() const
{
	return m_node;
}

void
IPCopeProtocol::PrintAllAddress() const
{
	NS_LOG_FUNCTION(this<<"Print out all address of this IPCopeProtocol");
	std::vector<Ipv4Address>::const_iterator ip_iter;
	for(ip_iter = m_ips.begin(); ip_iter != m_ips.end(); ip_iter++)
		NS_LOG_FUNCTION(this<<(*ip_iter));
	std::vector<Mac48Address>::const_iterator mac_iter;
	for(mac_iter = m_macs.begin(); mac_iter != m_macs.end(); mac_iter++)
		NS_LOG_FUNCTION(this<<(*mac_iter));
}

void
IPCopeProtocol::AddMac(const Mac48Address & mac)
{
	NS_LOG_FUNCTION_NOARGS();
	if(find(m_macs.begin(), m_macs.end(), mac) == m_macs.end())
		m_macs.push_back(mac);
}

void
IPCopeProtocol::SendHello() 
{
	NS_LOG_FUNCTION_NOARGS();
	IPCopeHello helloHeader;
	for(uint32_t i = 0; i<m_devices.size(); i++)
	{
		Ipv4Address ip = m_devices[i]->GetIP();
		Mac48Address mac = Mac48Address::ConvertFrom(m_devices[i]->GetAddress());
		uint16_t channel = m_devices[i]->GetChannelNumber();
		helloHeader.Add(ip, mac, channel);
		NS_LOG_FUNCTION("Add to hello header");
	}
	//send hello from every and each IPCopeDevice
	for(uint32_t i = 0; i<m_devices.size(); i++)
	{
		Ptr<Packet> packet = Create<Packet>();
		packet->AddHeader(helloHeader);
		const Mac48Address src = Mac48Address::ConvertFrom(m_devices[i]->GetAddress());
		const Mac48Address dest = Mac48Address::ConvertFrom(m_devices[i]->GetBroadcast());
		uint32_t index = m_devices[i]->GetCopeIfIndex();
		Enqueue(packet, src, dest, PROT_NUMBER, index, HELLO);
	}
}

	void
IPCopeProtocol::HelloTimerExpire () 
{
	NS_LOG_FUNCTION_NOARGS();
	SendHello();
	Time toBeSchedule = (Seconds)(m_helloInterval.GetSeconds() + rand()%10);
	NS_LOG_FUNCTION(this<<"original scheduled time: "<<m_helloInterval.GetSeconds()<<" new Scheduled time: "<<toBeSchedule.GetSeconds());
	m_helloTimer.Schedule(toBeSchedule);
}

void
IPCopeProtocol::StartHello()
{
	Time toBeSchedule = (Seconds)(m_helloInterval.GetSeconds() + rand()%10);
	NS_LOG_FUNCTION(this<<"original scheduled time: "<<m_helloInterval.GetSeconds()<<" new Scheduled time: "<<toBeSchedule.GetSeconds());
	m_helloTimer.Schedule(toBeSchedule);
}

}//namespace cope
}//namespace ns3
