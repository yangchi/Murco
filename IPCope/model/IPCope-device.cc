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


#include "IPCope-device.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <stdlib.h>

NS_LOG_COMPONENT_DEFINE ("IPCopeDevice");

namespace ns3{
namespace ipcope{

IPCopeDevice::IPCopeDevice()
{
}

IPCopeDevice::~IPCopeDevice(){}

TypeId
IPCopeDevice::GetTypeId()
{
	static TypeId tid = TypeId ("ns3::cope::IPCopeDevice")
		.SetParent<NetDevice>()
		.AddConstructor<IPCopeDevice> ()
		.AddAttribute ("Mtu", "The Mac-level Maximum Transimission Unit",
						UintegerValue(0xffff),
						MakeUintegerAccessor (&IPCopeDevice::SetMtu,
											  &IPCopeDevice::GetMtu),
						MakeUintegerChecker<uint16_t> () )
		.AddTraceSource("IPCopeRx",
						"Packet received by NC device. Being forwarded. Non-promisc.",
						MakeTraceSourceAccessor(&IPCopeDevice::m_copeRxTrace))
		.AddTraceSource("IPCopePromiscRx",
						"Packet received by NC device. Being forwarded. Promisc mode.",
						MakeTraceSourceAccessor(&IPCopeDevice::m_copePromiscRxTrace))
		;
	return tid;
}

void
IPCopeDevice::SetCopeProtocol(Ptr<IPCopeProtocol> cope)
{
	NS_LOG_FUNCTION_NOARGS();
	m_cope = cope;
}

void
IPCopeDevice::SetCopeIfIndex (const uint32_t index)
{
	NS_LOG_FUNCTION(this<<index);
	m_copeIfIndex = index;
}

uint32_t
IPCopeDevice::GetCopeIfIndex () const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_copeIfIndex;
}


void
IPCopeDevice::SetIfIndex (const uint32_t index)
{
	NS_LOG_FUNCTION_NOARGS ();
	m_ifIndex = index;
}

uint32_t
IPCopeDevice::GetIfIndex () const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_ifIndex;
}

Ptr<Channel>
IPCopeDevice::GetChannel() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_channel;
}

bool
IPCopeDevice::IsQueueFull() const
{
	Ptr<WifiNetDevice> wifiNetDevice = m_iface->GetObject<WifiNetDevice>();
	Ptr<RegularWifiMac> wifiMac = wifiNetDevice->GetMac()->GetObject<RegularWifiMac>();
	//Ptr<DcaTxop> txop = wifiMac->GetDcaTxop();
	PointerValue ptr;
	Ptr<DcaTxop> txop;
	wifiMac->GetAttribute("DcaTxop", ptr);
	txop = ptr.Get<DcaTxop>();
	//return txop->IsQueueFull();
	Ptr<WifiMacQueue> wifiMacQueue = txop->GetQueue();
	//return wifiMacQueue->IsFull();
	return (wifiMacQueue->GetSize() == wifiMacQueue->GetMaxSize());
}

uint32_t
IPCopeDevice::GetMacQueueSize() const
{
	Ptr<WifiNetDevice> wifiNetDevice = m_iface->GetObject<WifiNetDevice>();
	Ptr<RegularWifiMac> wifiMac = wifiNetDevice->GetMac()->GetObject<RegularWifiMac>();
	//Ptr<DcaTxop> txop = wifiMac->GetDcaTxop();
	//return txop->GetQueueSize();
	PointerValue ptr;
	wifiMac->GetAttribute("DcaTxop", ptr);
	Ptr<DcaTxop> txop = ptr.Get<DcaTxop>();
	Ptr<WifiMacQueue> wifiMacQueue = txop->GetQueue();
	return wifiMacQueue->GetSize();
}

Address
IPCopeDevice::GetAddress() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_mac;
}

Ipv4Address
IPCopeDevice::GetIP() const
{
	return m_ip;
}

void
IPCopeDevice::SetAddress (Address address)
{
	NS_LOG_FUNCTION(this<<"Why should one ever call this function?");
	m_mac= Mac48Address::ConvertFrom(address);
}

bool
IPCopeDevice::SetMtu(const uint16_t mtu)
{
	NS_LOG_FUNCTION_NOARGS();
	m_mtu = mtu;
	return true;
}

uint16_t
IPCopeDevice::GetMtu() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_mtu;
}

bool
IPCopeDevice::IsLinkUp () const
{
	NS_LOG_FUNCTION_NOARGS();
	return true;
}

void
IPCopeDevice::AddLinkChangeCallback (Callback<void> callback)
{
	//do nothing
}

bool
IPCopeDevice::IsBroadcast() const
{
	NS_LOG_FUNCTION_NOARGS();
	return true;
}

Address
IPCopeDevice::GetBroadcast() const
{
	NS_LOG_FUNCTION_NOARGS();
	return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
IPCopeDevice::IsMulticast() const
{
	NS_LOG_FUNCTION_NOARGS();
	return true;
}

Address
IPCopeDevice::GetMulticast (Ipv4Address multicastGroup) const
{
	NS_LOG_FUNCTION(this << multicastGroup);
	Mac48Address multicast = Mac48Address::GetMulticast (multicastGroup);
	return multicast;
}

Address
IPCopeDevice::GetMulticast(Ipv6Address addr) const
{
	NS_LOG_FUNCTION (this << addr);
	return Mac48Address::GetMulticast(addr);
}

bool
IPCopeDevice::IsPointToPoint () const
{
	NS_LOG_FUNCTION_NOARGS();
	return false;
}

bool
IPCopeDevice::IsBridge() const
{
	NS_LOG_FUNCTION_NOARGS();
	return false;
}

bool
IPCopeDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION_NOARGS();
	const Mac48Address dest_mac = Mac48Address::ConvertFrom(dest);

	if(protocolNumber != Ipv4L3Protocol::PROT_NUMBER && protocolNumber != IPCopeProtocol::PROT_NUMBER)
	{
		m_iface->Send(packet, dest, protocolNumber);
		return true;
	}
	else
		return m_cope->Enqueue(packet, m_mac, dest_mac, protocolNumber, m_copeIfIndex, DATA);
}

bool
IPCopeDevice::SendFrom(Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION(dest);
	const Mac48Address src_mac = Mac48Address::ConvertFrom(src);
	const Mac48Address dest_mac = Mac48Address::ConvertFrom(dest);
	if(protocolNumber != Ipv4L3Protocol::PROT_NUMBER && protocolNumber != IPCopeProtocol::PROT_NUMBER)
	{
		m_iface->Send(packet, dest, protocolNumber);
		return true;
	}
	else
		return m_cope->Enqueue(packet, src_mac, dest_mac, protocolNumber, m_copeIfIndex, DATA);
}

void
IPCopeDevice::ForwardDown(Ptr<Packet> packet, const Mac48Address & dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION_NOARGS();
	m_iface->Send(packet, dest, protocolNumber);
}

Ptr<Node>
IPCopeDevice::GetNode() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_node;
}

void
IPCopeDevice::SetNode(Ptr<Node> node)
{
	NS_LOG_FUNCTION(this<<node);
	m_node = node;
}

bool
IPCopeDevice::NeedsArp() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return true;
}

void
IPCopeDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
	NS_LOG_FUNCTION_NOARGS();
	m_rxCallback = cb;
}

void
IPCopeDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
	NS_LOG_FUNCTION_NOARGS();
	m_promiscRxCallback = cb;
}

bool
IPCopeDevice::SupportsSendFrom() const
{
	NS_LOG_FUNCTION_NOARGS();
	return false;
}

void
IPCopeDevice::SetInterface (Ptr<NetDevice> iface)
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(iface != this);
	if(!Mac48Address::IsMatchingType(iface->GetAddress()))
		NS_FATAL_ERROR ("EUI 48 Address support required.");
	SetAddress(iface->GetAddress());
	m_cope->AddMac(m_mac);
	Ptr<WifiNetDevice> wifiNetDevice = iface->GetObject<WifiNetDevice>();
	if(wifiNetDevice == 0)
		NS_FATAL_ERROR("Device is not a Wifi NIC");

	Ptr<RegularWifiMac> wifiMac = wifiNetDevice->GetMac()->GetObject<RegularWifiMac>();
	//Ptr<DcaTxop> txop = wifiMac->GetDcaTxop();
	//txop->SetMaxQueueSize(10);
	PointerValue ptr;
	wifiMac->GetAttribute("DcaTxop", ptr);
	Ptr<DcaTxop> txop = ptr.Get<DcaTxop>();
	Ptr<WifiMacQueue> wifiMacQueue = txop->GetQueue();
	wifiMacQueue->SetMaxSize(10);

	Ptr<WifiPhy> phy = wifiNetDevice->GetPhy();
	m_channelNumber = phy->GetChannelNumber();
	m_node -> RegisterProtocolHandler (MakeCallback (&IPCopeDevice::ReceiveFromDevice, this), 0x0, iface, true);
	NS_LOG_FUNCTION_NOARGS();

	//m_ifaces.push_back(iface);
	m_iface = iface;
	//m_cope->Init();
}

uint16_t
IPCopeDevice::GetChannelNumber() const
{
	return m_channelNumber;
}

Ptr<NetDevice>
IPCopeDevice::GetInterface() const
{
	return m_iface;
}

void
IPCopeDevice::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& source, const Address& dest, PacketType packetType)
{
	NS_LOG_FUNCTION(this<<source<<dest<<protocol);
	packet->Print(std::cout);
	std::cout<<std::endl;
	fflush(stdout);

	Ptr<Packet> p = packet->Copy();
	if(protocol != Ipv4L3Protocol::PROT_NUMBER && protocol != IPCopeProtocol::PROT_NUMBER)
		ForwardUp(p, protocol, Mac48Address::ConvertFrom(source), Mac48Address::ConvertFrom(dest), packetType);
	else
		m_cope->Recv(device, p, protocol, source, dest, packetType, m_copeIfIndex);

}

void
IPCopeDevice::ForwardUp(Ptr<Packet> packet, uint16_t protocol, const Mac48Address & src, const Mac48Address & dest, PacketType packetType)
{
	NS_LOG_FUNCTION(this);
	if(protocol == Ipv4L3Protocol::PROT_NUMBER)
	{
		Ipv4Header ipHeader;
		packet->PeekHeader(ipHeader);
		ipHeader.Print(std::cout);
		std::cout<<std::endl;
		fflush(stdout);
	}
	std::vector<Mac48Address>::iterator iter;
	enum NetDevice::PacketType type;
	Ptr<Packet> packet_copy = packet->Copy();
	NS_LOG_FUNCTION(this<<src<<dest<<protocol<<packetType);

	if(dest.IsBroadcast())
		type = NetDevice::PACKET_BROADCAST;
	else if (dest.IsGroup())
		type = NetDevice::PACKET_MULTICAST;
	else if (dest == m_mac)
		type = NetDevice::PACKET_HOST;
	else
		type = NetDevice::PACKET_OTHERHOST;

	if(type != NetDevice::PACKET_OTHERHOST)
	{
		NS_LOG_LOGIC("IPCopeDevice recv callback");
		//NotifyRx(packet);
		m_rxCallback(this, packet_copy, protocol, src);

	}
	if(!m_promiscRxCallback.IsNull())
	{
		NS_LOG_LOGIC("IPCopeDevice promisc recv callback");
		//NotifyPromiscRx(packet);
		m_promiscRxCallback(this, packet_copy, protocol, src, dest, type);
	}
}

void
IPCopeDevice::NotifyRx(Ptr<const Packet> packet)
{
	m_copeRxTrace (packet);
}

void
IPCopeDevice::NotifyPromiscRx(Ptr<const Packet> packet)
{
	m_copePromiscRxTrace(packet);
}

void 
IPCopeDevice::SetUpIpFromWifi()
{
	NS_LOG_FUNCTION_NOARGS();
	Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
	Ipv4Mask mask;
	int32_t interface = ipv4->GetInterfaceForDevice(m_iface);
	Ipv4InterfaceAddress ipv4InterAddr = ipv4->GetAddress(interface, 0);
	Ipv4Address ipv4Addr = ipv4InterAddr.GetLocal();
	mask = ipv4InterAddr.GetMask();
	m_ip = ipv4Addr;
	m_cope->AddIP(m_ip);
	SetIpv4Mask(mask);
}

void 
IPCopeDevice::SetUpIp()
{
	NS_LOG_FUNCTION_NOARGS();
	Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
	Ipv4Mask mask;
	int32_t interface = ipv4->GetInterfaceForDevice(this);
	SetIpv4Interface(interface);
	Ipv4InterfaceAddress ipv4InterAddr = ipv4->GetAddress(interface, 0);
	Ipv4Address ipv4Addr = ipv4InterAddr.GetLocal();
	mask = ipv4InterAddr.GetMask();
	m_ip = ipv4Addr;
	m_cope->AddIP(m_ip);
	m_cope->PrintAllAddress();
	SetIpv4Mask(mask);
	NS_LOG_DEBUG("mac: "<<m_mac<<" index in ipv4: "<<GetIfIndex());
	NS_LOG_DEBUG("ip: "<<ipv4Addr<<" interface index: "<<interface);
}

void 
IPCopeDevice::SetIpv4Mask(const Ipv4Mask & mask)
{
	m_mask = mask;
	m_cope->SetIpv4Mask(mask);
}

void
IPCopeDevice::SetIpv4Interface(uint32_t ipv4Interface)
{
	m_ipv4Interface = ipv4Interface;
}

uint32_t
IPCopeDevice::GetIpv4Interface() const
{
	return m_ipv4Interface;
}

}//namespace cope
}//namespace ns3
