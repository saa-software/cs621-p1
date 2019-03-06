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

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "point-to-point-net-device.h"
#include "point-to-point-channel.h"
#include "ppp-header.h"
#include "zlib.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointNetDevice");

NS_OBJECT_ENSURE_REGISTERED (PointToPointNetDevice);

TypeId
PointToPointNetDevice::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::PointToPointNetDevice")
          .SetParent<NetDevice> ()
          .SetGroupName ("PointToPoint")
          .AddConstructor<PointToPointNetDevice> ()
          .AddAttribute (
              "Mtu", "The MAC-level Maximum Transmission Unit", UintegerValue (DEFAULT_MTU),
              MakeUintegerAccessor (&PointToPointNetDevice::SetMtu, &PointToPointNetDevice::GetMtu),
              MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("Address", "The MAC address of this device.",
                         Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                         MakeMac48AddressAccessor (&PointToPointNetDevice::m_address),
                         MakeMac48AddressChecker ())
          .AddAttribute ("DataRate", "The default data rate for point to point links",
                         DataRateValue (DataRate ("32768b/s")),
                         MakeDataRateAccessor (&PointToPointNetDevice::m_bps),
                         MakeDataRateChecker ())
          .AddAttribute ("ReceiveErrorModel",
                         "The receiver error model used to simulate packet loss", PointerValue (),
                         MakePointerAccessor (&PointToPointNetDevice::m_receiveErrorModel),
                         MakePointerChecker<ErrorModel> ())
          .AddAttribute ("InterframeGap", "The time to wait between packet (frame) transmissions",
                         TimeValue (Seconds (0.0)),
                         MakeTimeAccessor (&PointToPointNetDevice::m_tInterframeGap),
                         MakeTimeChecker ())
          .AddAttribute ("CompressionEnabled", "Whether to compress data or not",
                         BooleanValue (false),
                         MakeBooleanAccessor (&PointToPointNetDevice::SetCompressionEnabled),
                         MakeBooleanChecker ())

          //
          // Transmit queueing discipline for the device which includes its own set
          // of trace hooks.
          //
          .AddAttribute ("TxQueue", "A queue to use as the transmit queue in the device.",
                         PointerValue (), MakePointerAccessor (&PointToPointNetDevice::m_queue),
                         MakePointerChecker<Queue<Packet>> ())

          //
          // Trace sources at the "top" of the net device, where packets transition
          // to/from higher layers.
          //
          .AddTraceSource ("MacTx",
                           "Trace source indicating a packet has arrived "
                           "for transmission by this device",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_macTxTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("MacTxDrop",
                           "Trace source indicating a packet has been dropped "
                           "by the device before transmission",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_macTxDropTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("MacPromiscRx",
                           "A packet has been received by this device, "
                           "has been passed up from the physical layer "
                           "and is being forwarded up the local protocol stack.  "
                           "This is a promiscuous trace,",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_macPromiscRxTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("MacRx",
                           "A packet has been received by this device, "
                           "has been passed up from the physical layer "
                           "and is being forwarded up the local protocol stack.  "
                           "This is a non-promiscuous trace,",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_macRxTrace),
                           "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was dropped "
                     "before being forwarded up the stack",
                     MakeTraceSourceAccessor (&PointToPointNetDevice::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
#endif
          //
          // Trace sources at the "bottom" of the net device, where packets transition
          // to/from the channel.
          //
          .AddTraceSource ("PhyTxBegin",
                           "Trace source indicating a packet has begun "
                           "transmitting over the channel",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_phyTxBeginTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("PhyTxEnd",
                           "Trace source indicating a packet has been "
                           "completely transmitted over the channel",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_phyTxEndTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("PhyTxDrop",
                           "Trace source indicating a packet has been "
                           "dropped by the device during transmission",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_phyTxDropTrace),
                           "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet has begun "
                     "being received by the device",
                     MakeTraceSourceAccessor (&PointToPointNetDevice::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
#endif
          .AddTraceSource ("PhyRxEnd",
                           "Trace source indicating a packet has been "
                           "completely received by the device",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_phyRxEndTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("PhyRxDrop",
                           "Trace source indicating a packet has been "
                           "dropped by the device during reception",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_phyRxDropTrace),
                           "ns3::Packet::TracedCallback")

          //
          // Trace sources designed to simulate a packet sniffer facility (tcpdump).
          // Note that there is really no difference between promiscuous and
          // non-promiscuous traces in a point-to-point link.
          //
          .AddTraceSource ("Sniffer",
                           "Trace source simulating a non-promiscuous packet sniffer "
                           "attached to the device",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_snifferTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("PromiscSniffer",
                           "Trace source simulating a promiscuous packet sniffer "
                           "attached to the device",
                           MakeTraceSourceAccessor (&PointToPointNetDevice::m_promiscSnifferTrace),
                           "ns3::Packet::TracedCallback");
  return tid;
}

PointToPointNetDevice::PointToPointNetDevice ()
    : m_txMachineState (READY),
      m_channel (0),
      m_linkUp (false),
      m_currentPkt (0),
      //default compressionEnable is false
      m_compressionEnabled (false)
{
  NS_LOG_FUNCTION (this);
}

PointToPointNetDevice::~PointToPointNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
PointToPointNetDevice::SetCompressionEnabled (bool compressionEnabled)
{
  NS_LOG_FUNCTION (this << compressionEnabled);
  m_compressionEnabled = compressionEnabled;
}

void
PointToPointNetDevice::AddHeader (Ptr<Packet> p, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << p << protocolNumber);
  PppHeader ppp;
  ppp.SetProtocol (EtherToPpp (protocolNumber));
  p->AddHeader (ppp);
}

bool
PointToPointNetDevice::ProcessHeader (Ptr<Packet> p, uint16_t &param)
{
  NS_LOG_FUNCTION (this << p << param);
  PppHeader ppp;
  p->RemoveHeader (ppp);
  param = PppToEther (ppp.GetProtocol ());
  return true;
}

void
PointToPointNetDevice::CompressionAddHeader (Ptr<Packet> p, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << p << protocolNumber);
  PppHeader ppp;
  ppp.SetProtocol (CompressionEtherToPpp (protocolNumber));
  p->AddHeader (ppp);
}

bool
PointToPointNetDevice::CompressionProcessHeader (Ptr<Packet> p, uint16_t &param)
{
  NS_LOG_FUNCTION (this << p << param);
  PppHeader ppp;
  p->RemoveHeader (ppp);
  param = CompressionPppToEther (ppp.GetProtocol ());
  return true;
}

void
PointToPointNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_channel = 0;
  m_receiveErrorModel = 0;
  m_currentPkt = 0;
  m_queue = 0;
  NetDevice::DoDispose ();
}

void
PointToPointNetDevice::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this);
  m_bps = bps;
}

void
PointToPointNetDevice::SetInterframeGap (Time t)
{
  NS_LOG_FUNCTION (this << t.GetSeconds ());
  m_tInterframeGap = t;
}

bool
PointToPointNetDevice::TransmitStart (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  //
  // This function is called to start the process of transmitting a packet.
  // We need to tell the channel that we've started wiggling the wire and
  // schedule an event that will be executed when the transmission is complete.
  //
  NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
  m_txMachineState = BUSY;
  m_currentPkt = p;
  m_phyTxBeginTrace (m_currentPkt);

  Time txTime = m_bps.CalculateBytesTxTime (p->GetSize ());
  Time txCompleteTime = txTime + m_tInterframeGap;

  NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << txCompleteTime.GetSeconds () << "sec");
  Simulator::Schedule (txCompleteTime, &PointToPointNetDevice::TransmitComplete, this);

  bool result = m_channel->TransmitStart (p, this, txTime);
  if (result == false)
    {
      m_phyTxDropTrace (p);
    }
  return result;
}

void
PointToPointNetDevice::TransmitComplete (void)
{
  NS_LOG_FUNCTION (this);

  //
  // This function is called to when we're all done transmitting a packet.
  // We try and pull another packet off of the transmit queue.  If the queue
  // is empty, we are done, otherwise we need to start transmitting the
  // next packet.
  //
  NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
  m_txMachineState = READY;

  NS_ASSERT_MSG (m_currentPkt != 0, "PointToPointNetDevice::TransmitComplete(): m_currentPkt zero");

  m_phyTxEndTrace (m_currentPkt);
  m_currentPkt = 0;

  Ptr<Packet> p = m_queue->Dequeue ();
  if (p == 0)
    {
      NS_LOG_LOGIC ("No pending packets in device queue after tx complete");
      return;
    }

  //
  // Got another packet off of the queue, so start the transmit process again.
  //
  m_snifferTrace (p);
  m_promiscSnifferTrace (p);
  TransmitStart (p);
}

bool
PointToPointNetDevice::Attach (Ptr<PointToPointChannel> ch)
{
  NS_LOG_FUNCTION (this << &ch);

  m_channel = ch;

  m_channel->Attach (this);

  //
  // This device is up whenever it is attached to a channel.  A better plan
  // would be to have the link come up when both devices are attached, but this
  // is not done for now.
  //
  NotifyLinkUp ();
  return true;
}

void
PointToPointNetDevice::SetQueue (Ptr<Queue<Packet>> q)
{
  NS_LOG_FUNCTION (this << q);
  m_queue = q;
}

void
PointToPointNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void
PointToPointNetDevice::Receive (Ptr<Packet> incomingPacket)
{
  // printf("enaled %d\n", m_compressionEnabled);
  PppHeader ppp;
  incomingPacket->PeekHeader (ppp);
  printf ("Recv protocol: %d\n", ppp.GetProtocol ());
  Ptr<Packet> packet;
  if (m_compressionEnabled == 1)
    {
      if (ppp.GetProtocol () == 16417)
        {
          printf ("########################## DECOMPRESS ##########################\n");
          printf ("Incoming Packet Size: %d\n", incomingPacket->GetSize ());
          packet = DecompressPacket (incomingPacket);
          printf ("Packet size:          %d\n", packet->GetSize ());
        }
      else if (ppp.GetProtocol () == 33)
        {
          printf ("Incoming Packet Size: %d\n", incomingPacket->GetSize ());
          printf ("##########################  COMPRESS  ##########################\n");
          printf ("Packet size:          %d\n", packet->GetSize ());
          packet = CompressPacket (incomingPacket);
        }
    }
  else
    {
      packet = incomingPacket->Copy ();
    }

  PppHeader ppp2;
  packet->PeekHeader (ppp2);
  printf ("Post action protocol: %d\n", ppp2.GetProtocol ());
  // PppHeader pppTest;
  // packet->PeekHeader (pppTest);
  // printf ("recv test decompress protocol: %d\n", pppTest.GetProtocol ());

  NS_LOG_FUNCTION (this << packet);
  uint16_t protocol = 0;

  // CompressPacket (packet);
  // uint32_t size = 1024;
  // Ptr<Packet> p = Create<Packet> (size);
  // printf ("test packet: %d\n", p->GetSize ());
  // CompressPacket (p);

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet))
    {
      //
      // If we have an error model and it indicates that it is time to lose a
      // corrupted packet, don't forward this packet up, let it go.
      //
      m_phyRxDropTrace (packet);
    }
  else
    {
      //
      // Hit the trace hooks.  All of these hooks are in the same place in this
      // device because it is so simple, but this is not usually the case in
      // more complicated devices.
      //
      m_snifferTrace (packet);
      m_promiscSnifferTrace (packet);
      m_phyRxEndTrace (packet);

      //
      // Trace sinks will expect complete packets, not packets without some of the
      // headers.
      //
      Ptr<Packet> originalPacket = packet->Copy ();

      //
      // Strip off the point-to-point protocol header and forward this packet
      // up the protocol stack.  Since this is a simple point-to-point link,
      // there is no difference in what the promisc callback sees and what the
      // normal receive callback sees.
      //
      if (m_compressionEnabled == 1) {
        CompressionProcessHeader (packet, protocol);
      } else {
        ProcessHeader (packet, protocol);
      }

      if (!m_promiscCallback.IsNull ())
        {
          m_macPromiscRxTrace (originalPacket);
          m_promiscCallback (this, packet, protocol, GetRemote (), GetAddress (),
                             NetDevice::PACKET_HOST);
        }

      m_macRxTrace (originalPacket);
      m_rxCallback (this, packet, protocol, GetRemote ());
    }
}

Ptr<Queue<Packet>>
PointToPointNetDevice::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

void
PointToPointNetDevice::NotifyLinkUp (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = true;
  m_linkChangeCallbacks ();
}

void
PointToPointNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this);
  m_ifIndex = index;
}

uint32_t
PointToPointNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel>
PointToPointNetDevice::GetChannel (void) const
{
  return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

void
PointToPointNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}

Address
PointToPointNetDevice::GetAddress (void) const
{
  return m_address;
}

bool
PointToPointNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_linkUp;
}

void
PointToPointNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChangeCallbacks.ConnectWithoutContext (callback);
}

//
// This is a point-to-point device, so every transmission is a broadcast to
// all of the devices on the network.
//
bool
PointToPointNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

//
// We don't really need any addressing information since this is a
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
Address
PointToPointNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
PointToPointNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
PointToPointNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("01:00:5e:00:00:00");
}

Address
PointToPointNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address ("33:33:00:00:00:00");
}

bool
PointToPointNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

bool
PointToPointNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
PointToPointNetDevice::Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_LOG_LOGIC ("p=" << packet << ", dest=" << &dest);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  PppHeader ppp;
  packet->PeekHeader (ppp);
  printf ("Send protocol: %d\n", ppp.GetProtocol ());
  // if (m_compressionEnabled == 1 && ppp.GetProtocol () == 33) {
  //   printf("HERE2_________________________2");
  //   packet = CompressPacket(packet);
  // }

  //
  // If IsLinkUp() is false it means there is no channel to send any packet
  // over so we just hit the drop trace on the packet and return an error.
  //
  if (IsLinkUp () == false)
    {
      m_macTxDropTrace (packet);
      return false;
    }

  //
  // Stick a point to point protocol header on the packet in preparation for
  // shoving it out the door.
  //
  if (m_compressionEnabled == 1) {
    CompressionAddHeader (packet, protocolNumber);
  } else {
    AddHeader (packet, protocolNumber);
  }
  PppHeader ppp2;
  packet->PeekHeader (ppp2);
  printf ("Send protocol: %d\n", ppp2.GetProtocol ());
  m_macTxTrace (packet);

  //
  // We should enqueue and dequeue the packet to hit the tracing hooks.
  //
  if (m_queue->Enqueue (packet))
    {
      //
      // If the channel is ready for transition we send the packet right now
      //
      if (m_txMachineState == READY)
        {
          packet = m_queue->Dequeue ();
          m_snifferTrace (packet);
          m_promiscSnifferTrace (packet);
          bool ret = TransmitStart (packet);
          return ret;
        }
      return true;
    }

  // Enqueue may fail (overflow)

  m_macTxDropTrace (packet);
  return false;
}

bool
PointToPointNetDevice::SendFrom (Ptr<Packet> packet, const Address &source, const Address &dest,
                                 uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  return false;
}

Ptr<Node>
PointToPointNetDevice::GetNode (void) const
{
  return m_node;
}

void
PointToPointNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
}

bool
PointToPointNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
PointToPointNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
PointToPointNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  m_promiscCallback = cb;
}

Ptr<Packet>
PointToPointNetDevice::CompressPacket (Ptr<Packet> packet)
{
  // printf ("Compress\n");
  // printf ("Size of packet: %d\n", packet->GetSize ());

  // PppHeader ppp;
  // packet->PeekHeader (ppp);
  // printf("protocol %d\n", ppp.GetProtocol ());

  uint32_t destSize;
  char *data;
  data = (char *) &packet;
  u_int32_t dataSize = packet->GetSize ();

  std::vector<uint8_t> buffer;

  const size_t BUFSIZE = 1024;
  uint8_t temp_buffer[BUFSIZE];

  z_stream strm;
  strm.zalloc = 0;
  strm.zfree = 0;
  strm.next_in = reinterpret_cast<uint8_t *> (data);
  strm.avail_in = dataSize;
  strm.next_out = temp_buffer;
  strm.avail_out = BUFSIZE;

  deflateInit (&strm, Z_BEST_COMPRESSION);

  while (strm.avail_in != 0)
    {
      int res = deflate (&strm, Z_NO_FLUSH);
      NS_ASSERT (res == Z_OK);
      if (strm.avail_out == 0)
        {
          //  buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
          //  strm.next_out = temp_buffer;
          //  strm.avail_out = BUFSIZE;
        }
    }

  int deflate_res = Z_OK;
  while (deflate_res == Z_OK)
    {
      // if (strm.avail_out == 0)
      // {
      //  buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
      //  strm.next_out = temp_buffer;
      //  strm.avail_out = BUFSIZE;
      // }
      deflate_res = deflate (&strm, Z_FINISH);
    }

  NS_ASSERT (deflate_res == Z_STREAM_END);
  // buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
  destSize = strm.total_out;
  deflateEnd (&strm);

  Ptr<Packet> compPacket = Create<Packet> (temp_buffer, destSize);
  // printf ("size of compressed packet: %d\n", compPacket->GetSize ());

  PppHeader ppp2;
  compPacket->RemoveHeader (ppp2);
  ppp2.SetProtocol (0x4021);\
  compPacket->AddHeader (ppp2);
  // printf ("after set protocol %d\n", ppp2.GetProtocol ());

  return compPacket;
}

Ptr<Packet>
PointToPointNetDevice::DecompressPacket (Ptr<Packet> packet)
{
  // printf ("Decompress:\n");
  // printf ("Size of packet: %d\n", packet->GetSize ());

  char *data;
  data = (char *) &packet;
  u_int32_t srcSize = packet->GetSize ();

  const size_t BUFSIZE = 1024;
  uint8_t temp_buffer[BUFSIZE];

  z_stream infstream;
  infstream.zalloc = Z_NULL;
  infstream.zfree = Z_NULL;
  infstream.opaque = Z_NULL;
  // setup "b" as the input and "c" as the compressed output
  // infstream.avail_in = (uInt)((char*)defstream.next_out - b); // size of input
  infstream.avail_in = srcSize;
  infstream.next_in = (Bytef *) data; // input char array
  infstream.avail_out = (uInt) sizeof (temp_buffer); // size of output
  infstream.next_out = (Bytef *) temp_buffer; // output char array

  // the actual DE-compression work.
  inflateInit (&infstream);
  inflate (&infstream, Z_NO_FLUSH);
  inflateEnd (&infstream);

  PppHeader ppp2;
  Ptr<Packet> decompPacket = Create<Packet> (temp_buffer, sizeof (temp_buffer));
  decompPacket->PeekHeader (ppp2);
  ppp2.SetProtocol (0x0021);
  // printf ("size of decompressed packet: %d\n", decompPacket->GetSize ());

  return decompPacket;
}

bool
PointToPointNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
PointToPointNetDevice::DoMpiReceive (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  Receive (p);
}

Address
PointToPointNetDevice::GetRemote (void) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_channel->GetNDevices () == 2);
  for (std::size_t i = 0; i < m_channel->GetNDevices (); ++i)
    {
      Ptr<NetDevice> tmp = m_channel->GetDevice (i);
      if (tmp != this)
        {
          return tmp->GetAddress ();
        }
    }
  NS_ASSERT (false);
  // quiet compiler.
  return Address ();
}

bool
PointToPointNetDevice::SetMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}

uint16_t
PointToPointNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}

uint16_t
PointToPointNetDevice::PppToEther (uint16_t proto)
{
  NS_LOG_FUNCTION_NOARGS ();
  switch (proto)
    {
    case 0x0021:
      return 0x0800; //IPv4
    case 0x0057:
      return 0x86DD; //IPv6
    default:
      NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
    }
  return 0;
}

uint16_t
PointToPointNetDevice::EtherToPpp (uint16_t proto)
{
  NS_LOG_FUNCTION_NOARGS ();
  switch (proto)
    {
    case 0x0800:
      return 0x0021; //IPv4
    case 0x86DD:
      return 0x0057; //IPv6
    default:
      NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
    }
  return 0;
}

uint16_t
PointToPointNetDevice::CompressionPppToEther (uint16_t proto)
{
  // NS_LOG_FUNCTION_NOARGS ();
  // switch (proto)
  //   {
  //   case 0x0021:
  //     return 0x0800; //IPv4
  //   case 0x0057:
  //     return 0x86DD; //IPv6
  //   default:
  //     NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
  //   }
  return 0x0800;
}

uint16_t
PointToPointNetDevice::CompressionEtherToPpp (uint16_t proto)
{
  // NS_LOG_FUNCTION_NOARGS ();
  // switch (proto)
  //   {
  //   case 0x0800:
  //     return 0x0021; //IPv4
  //   case 0x86DD:
  //     return 0x0057; //IPv6
  //   default:
  //     NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
  //   }
  return 0x4021;
}

} // namespace ns3
