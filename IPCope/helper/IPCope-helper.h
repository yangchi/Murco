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

#ifndef COPE_HELPER_H
#define COPE_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/IPCope-device.h"
#include "ns3/IPCope-protocol.h"
#include <vector>

namespace ns3{

class IPCopeHelper
{
public:
	IPCopeHelper();
	~IPCopeHelper();
	NetDeviceContainer InstallProtocol(NodeContainer nodes, bool mr, double rttime, double retry, double hello) ;
	//NetDeviceContainer InstallProtocol(NodeContainer nodes, bool mr, double rttime, double hello) ;
	void StartProtocol();

private:
	std::vector<Ptr<ipcope::IPCopeProtocol> > m_protocols;
};

}//namespace ns3
#endif
