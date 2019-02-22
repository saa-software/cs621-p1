/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

// Network topology
//
//       n0    n1   n2   n3
//       |     |    |    |
//       =================
//              LAN
//
// - UDP flows from n0 to n1 and back
// - DropTail queues 
// - Tracing of queues and packet receptions to file "cda.tr"

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Cda");

int 
main (int argc, char *argv[])
{
//
// Users may find it convenient to turn on explicit debugging
// for selected modules; the below lines suggest how to do this
//
  LogComponentEnable ("Cda", LOG_LEVEL_INFO);
  LogComponentEnable ("CdaClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("CdaServerApplication", LOG_LEVEL_ALL);

  CommandLine cmd;
  cmd.Parse (argc, argv);
//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer n;
  n.Create (4);

//
  NodeContainer n0n1 = NodeContainer (n.Get(0), n.Get(1));
  NodeContainer n1n2 = NodeContainer (n.Get(1), n.Get(2));
  NodeContainer n2n3 = NodeContainer (n.Get(2), n.Get(3));

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p0p1 = p2p.Install (n0n1);
  NetDeviceContainer p1p2 = p2p.Install (n1n2);
  NetDeviceContainer p2p3 = p2p.Install (n2n3);

  InternetStackHelper stack;
  stack.Install (n);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer i0i1 = address.Assign (p0p1);
  Ipv4InterfaceContainer i1i2 = address.Assign (p1p2);
  Ipv4InterfaceContainer i2i3 = address.Assign (p2p3);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create Applications.");
//
// Create a CdaServer application on node one.
//
  uint16_t port = 9;  // well-known echo port number
  CdaServerHelper server (port);
  ApplicationContainer apps = server.Install (n.Get (3));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

//
// Create a CdaClient application to send UDP datagrams from node zero to
// node one.
//
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds (1.);
  CdaClientHelper client (i2i3.GetAddress(1), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (n.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));


//
// Users may find it convenient to initialize echo packets with actual data;
// the below lines suggest how to do this
//
  client.SetFill (apps.Get (0), "Hello World");

  // client.SetFill (apps.Get (0), 0xa5, 1024);

  // uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  // client.SetFill (apps.Get (0), fill, sizeof(fill), 1024);


  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("cda.tr"));
  p2p.EnablePcapAll ("cda", false);

//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
