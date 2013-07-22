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

#ifndef COPEDEVICE_H
#define COPEDEVICE_H

#include "ns3/net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/wifi-net-device.h"
#include "ns3/ipv4-address.h"
#include "ns3/timer.h"
#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/wifi-net-device.h"
#include "ns3/dca-txop.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/arp-l3-protocol.h"

#include "IPCope-protocol.h"
#include "IPCope-header.h"
#include "IPCope-packet-info.h"
#include "IPCope-queue.h"
#include "IPCope-neighbor.h"
#include "IPCope-packet-pool.h"

#include <vector>
#include <set>
#include <map>
#include <cstdlib>

namespace ns3{
namespace ipcope{

class IPCopeDevice : public NetDevice
{
public:
	static TypeId GetTypeId();
	IPCopeDevice();
	virtual ~IPCopeDevice();

	virtual void SetIfIndex (const uint32_t index);
	virtual uint32_t GetIfIndex () const;
	virtual Ptr<Channel> GetChannel () const;
	virtual Address GetAddress () const;
	virtual void SetAddress (Address address);
	virtual bool SetMtu (const uint16_t mtu);
	virtual uint16_t GetMtu () const;
	virtual bool IsLinkUp () const;
	virtual void AddLinkChangeCallback (Callback<void> callback);
	virtual bool IsBroadcast () const;
	virtual Address GetBroadcast () const;
	virtual bool IsMulticast () const;
	virtual Address GetMulticast (Ipv4Address multicastGroup) const;
	virtual bool IsPointToPoint () const;
	virtual bool IsBridge () const;
	virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
	virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
	virtual Ptr<Node> GetNode () const;
	virtual void SetNode (Ptr<Node> node);
	virtual bool NeedsArp () const;
	virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
	virtual void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);
	virtual bool SupportsSendFrom () const;
	virtual Address GetMulticast (Ipv6Address addr) const;

	//not from net-device:

	void SetCopeIfIndex(const uint32_t index);
	uint32_t GetCopeIfIndex() const;
	void SetInterface (Ptr<NetDevice> iface);
	Ptr<NetDevice> GetInterface() const;
	//void SetIP(Ipv4Address ip);
	void SetUpIp();
	void SetUpIpFromWifi();
	Ipv4Address GetIP() const;
	void NotifyRx(Ptr<const Packet> packet);
	void NotifyPromiscRx(Ptr<const Packet> packet);
	void ForwardUp(Ptr<Packet> packet, uint16_t protocol, const Mac48Address & src, const Mac48Address & dest, PacketType packetType);
	void ForwardDown(Ptr<Packet> packet, const Mac48Address & dest, uint16_t protocolNumber);
	uint16_t GetChannelNumber() const;

	void SetIpv4Mask(const Ipv4Mask & mask);
	void SetIpv4Interface(uint32_t ipv4Interface);
	uint32_t GetIpv4Interface() const;
	void SetCopeProtocol(Ptr<IPCopeProtocol> cope);
	bool IsQueueFull() const;
	uint32_t GetMacQueueSize() const;

private:
	void ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& source, const Address& dest, PacketType packetType);

private:
	uint32_t m_ifIndex;
	uint32_t m_copeIfIndex;
	Ptr<Channel> m_channel;
	Ipv4Address m_ip;
	Mac48Address m_mac;
	uint16_t m_mtu;
	Ptr<Node> m_node;
	NetDevice::ReceiveCallback m_rxCallback;
	NetDevice::PromiscReceiveCallback m_promiscRxCallback;
	Ptr<NetDevice> m_iface;
	TracedCallback<Ptr<const Packet> > m_copeRxTrace;
	TracedCallback<Ptr<const Packet> > m_copePromiscRxTrace;
	Ptr<IPCopeProtocol> m_cope;
	Ipv4Mask m_mask;
	uint32_t m_ipv4Interface;
	uint16_t m_channelNumber;

};//class IPCopeDevice
}//namespace cope
}//namespace ns3

#endif
