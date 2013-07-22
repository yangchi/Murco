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


#ifndef COPEHEADER_H
#define COPEHEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include <map>
#include <vector>
#include "ns3/log.h"

namespace ns3{
namespace ipcope{

enum MessageType {
	HELLO = 1,
	DATA = 2,
};

struct AckBlockStruct
{
	Ipv4Address address;
	uint32_t pid;
};

typedef struct AckBlockStruct AckBlock;

class IPCopeType : public Header
{
public:
	IPCopeType();
	IPCopeType(MessageType type);
	virtual ~IPCopeType();
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual void Print(std::ostream &os) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual uint32_t GetSerializedSize (void) const;

	bool IsHello() const;
	bool IsData() const;
private:
	MessageType m_type;
};

struct AddressStruct
{
	Ipv4Address ip;
	Mac48Address mac;
	uint16_t channel;
};

typedef struct AddressStruct AddressPair;

class IPCopeHello : public Header
{
public:
	IPCopeHello();
	virtual ~IPCopeHello();
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual void Print(std::ostream &os) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual uint32_t GetSerializedSize (void) const;

	uint8_t GetLength() const;
	void Add(const Ipv4Address & ip, const Mac48Address & mac, uint16_t channel);
	AddressPair Get(uint8_t index) const;
private:
	std::vector<AddressPair> m_addPair;
};

class IPCopeHeader : public Header
{
public:
	IPCopeHeader();
	virtual ~IPCopeHeader();
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual void Print(std::ostream &os) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual uint32_t GetSerializedSize (void) const;

	bool AddIdNexthop(const Mac48Address & nexthop, uint32_t pktId);

	uint16_t GetEncodedNum() const;
	void SetEncodedNum(uint16_t encodedNum);
	std::map<Mac48Address, uint32_t> GetIdNexthops() const;

	bool AmINext(const Mac48Address & mac, uint32_t &pid) const;
	bool AmINext(std::vector<Mac48Address> macs, uint32_t &pid) const;

	bool AddRecpReport(uint32_t pid);
	bool AddAckBlock(const Ipv4Address & address, uint32_t pid);
	void AddAckBlock(std::vector<AckBlock> ackBlock);
	void AddAckBlock(AckBlock ackblock);

	bool Search(const std::vector<AckBlock> ackVec, const Ipv4Address & mac, AckBlock* ackblock) const;
	std::vector<uint32_t> Search(const std::vector<AckBlock> ackVec, const Ipv4Address & addr);

	std::vector<uint32_t> GetRecpReports() const ;
	uint16_t GetReportNum() const; 
	void SetReportNum(uint16_t reportNum);
	void SetAckNum(uint16_t ackNum);
	std::vector<AckBlock> GetAckBlocks() const { return m_ackBlocks; }
	bool IsBroadcast() const;
	void SetIp(const Ipv4Address & ip);
	Ipv4Address GetIp() const;

private:
	Ipv4Address m_ip;
	uint16_t m_encodedNum;
	std::map<Mac48Address, uint32_t> m_pidNexthops;

	uint16_t m_reportNum;
	//std::vector<RecpReport> m_receptionReports;
	std::vector<uint32_t> m_recps;
	uint16_t m_ackNum;
	//uint16_t m_localPktSeqNum;
	std::vector<AckBlock> m_ackBlocks;

};

}//namespace cope
}//namespace ns3

#endif
