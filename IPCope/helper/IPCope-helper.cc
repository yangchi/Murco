/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Yang CHI <chiyg@mail.uc.edu>
 */

#include "IPCope-helper.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("IPCopeHelper");

namespace ns3{
IPCopeHelper::IPCopeHelper(){}
IPCopeHelper::~IPCopeHelper(){}

NetDeviceContainer
IPCopeHelper::InstallProtocol(NodeContainer nodes, bool mr, double rttime, double retry, double hello) 
//IPCopeHelper::InstallProtocol(NodeContainer nodes, bool mr, double rttime, double hello) 
{
	NetDeviceContainer devices;
	for(NodeContainer::Iterator iter = nodes.Begin(); iter!= nodes.End(); iter++)
	{
		Ptr<Node> node = *iter;
		Ptr<ipcope::IPCopeProtocol> protocol = CreateObject<ipcope::IPCopeProtocol>(mr, rttime, retry, hello);
		protocol->SetNode(node);
		node->AggregateObject(protocol);
		uint32_t ifNum = node->GetNDevices();
		uint32_t iface;
		protocol->CreateNDevices(ifNum);
		for(iface = 0; iface < ifNum; iface++)
		{
			Ptr<ipcope::IPCopeDevice> copeDevice = protocol->GetDevice(iface);
			copeDevice->SetInterface(node->GetDevice(iface));
			devices.Add(copeDevice);
		}
		for(iface = 0; iface < ifNum; iface++)
		{
			node->AddDevice(protocol->GetDevice(iface));
		}
		m_protocols.push_back(protocol);
	}
	return devices;
}

void
IPCopeHelper::StartProtocol()
{
	std::vector<Ptr<ipcope::IPCopeProtocol> >::iterator iter;
	for(iter = m_protocols.begin(); iter != m_protocols.end(); iter++)
	{
		(*iter)->StartHello();
	}
}

}//namespace ns3
