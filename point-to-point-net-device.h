/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2007, 2008 University of Washington
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
*/

#ifndef POINT_TO_POINT_NET_DEVICE_H
#define POINT_TO_POINT_NET_DEVICE_H

#include <cstring>
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"

namespace ns3 {

    template<typename Item>
    class Queue;

    class PointToPointChannel;

    class ErrorModel;

    class PointToPointNetDevice : public NetDevice {
    public:
        static TypeId GetTypeId(void);

        PointToPointNetDevice();

        virtual ~PointToPointNetDevice();

        void SetDataRate(DataRate bps);

        void SetInterframeGap(Time t);

        bool Attach(Ptr <PointToPointChannel> ch);

        void SetQueue(Ptr <Queue<Packet>> queue);

        Ptr <Queue<Packet>> GetQueue(void) const;

        void SetReceiveErrorModel(Ptr <ErrorModel> em);

        void Receive(Ptr <Packet> p);

        // The remaining methods are documented in ns3::NetDevice*

        virtual void SetIfIndex(const uint32_t index);

        virtual uint32_t GetIfIndex(void) const;

        virtual Ptr <Channel> GetChannel(void) const;

        virtual void SetAddress(Address address);

        virtual Address GetAddress(void) const;

        virtual bool SetMtu(const uint16_t mtu);

        virtual uint16_t GetMtu(void) const;

        virtual bool IsLinkUp(void) const;

        virtual void AddLinkChangeCallback(Callback<void> callback);

        virtual bool IsBroadcast(void) const;

        virtual Address GetBroadcast(void) const;

        virtual bool IsMulticast(void) const;

        virtual Address GetMulticast(Ipv4Address multicastGroup) const;

        virtual bool IsPointToPoint(void) const;

        virtual bool IsBridge(void) const;

        virtual bool Send(Ptr <Packet> packet, const Address &dest, uint16_t protocolNumber);

        virtual bool SendFrom(Ptr <Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber);

        virtual Ptr <Node> GetNode(void) const;

        virtual void SetNode(Ptr <Node> node);

        virtual bool NeedsArp(void) const;

        virtual void SetReceiveCallback(NetDevice::ReceiveCallback cb);

        virtual Address GetMulticast(Ipv6Address addr) const;

        virtual void SetPromiscReceiveCallback(PromiscReceiveCallback cb);

        virtual bool SupportsSendFrom(void) const;

    protected:
        void DoMpiReceive(Ptr <Packet> p);

    private:

        PointToPointNetDevice &operator=(const PointToPointNetDevice &o);

        PointToPointNetDevice(const PointToPointNetDevice &o);

        virtual void DoDispose(void);

    private:

        Address GetRemote(void) const;

        void AddHeader(Ptr <Packet> p, uint16_t protocolNumber);

        bool ProcessHeader(Ptr <Packet> p, uint16_t &param);

        bool TransmitStart(Ptr <Packet> p);

        void TransmitComplete(void);

        void NotifyLinkUp(void);

        enum TxMachineState {
            READY,
            BUSY
        };
        TxMachineState m_txMachineState;

        DataRate m_bps;

        Time m_tInterframeGap;

        Ptr <PointToPointChannel> m_channel;

        Ptr <Queue<Packet>> m_queue;

        Ptr <ErrorModel> m_receiveErrorModel;

        TracedCallback <Ptr<const Packet>> m_macTxTrace;

        TracedCallback <Ptr<const Packet>> m_macTxDropTrace;

        TracedCallback <Ptr<const Packet>> m_macPromiscRxTrace;

        TracedCallback <Ptr<const Packet>> m_macRxTrace;

        TracedCallback <Ptr<const Packet>> m_macRxDropTrace;

        TracedCallback <Ptr<const Packet>> m_phyTxBeginTrace;

        TracedCallback <Ptr<const Packet>> m_phyTxEndTrace;

        TracedCallback <Ptr<const Packet>> m_phyTxDropTrace;

        TracedCallback <Ptr<const Packet>> m_phyRxBeginTrace;

        TracedCallback <Ptr<const Packet>> m_phyRxEndTrace;

        TracedCallback <Ptr<const Packet>> m_phyRxDropTrace;

        TracedCallback <Ptr<const Packet>> m_snifferTrace;

        TracedCallback <Ptr<const Packet>> m_promiscSnifferTrace;

        Ptr <Node> m_node;
        Mac48Address m_address;
        NetDevice::ReceiveCallback m_rxCallback;
        NetDevice::PromiscReceiveCallback m_promiscCallback;
        //   (promisc data)
        uint32_t m_ifIndex;
        bool m_linkUp;
        TracedCallback<> m_linkChangeCallbacks;

        static const uint16_t DEFAULT_MTU = 1500;

        uint32_t m_mtu;

        Ptr <Packet> m_currentPkt;

        static uint16_t PppToEther(uint16_t protocol);

        static uint16_t EtherToPpp(uint16_t protocol);
    };

} // namespace ns3

#endif /* POINT_TO_POINT_NET_DEVICE_H */