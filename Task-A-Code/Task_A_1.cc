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

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
//#include "propagation-loss-model.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   *    *    *    *
//                                     LAN 10.1.2.0

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Task_A_1_Script");
Ptr<PacketSink> sink;     /* Pointer to the packet sink application */

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 10;
  bool tracing = false;
  uint32_t SentPackets = 0;
  uint32_t ReceivedPackets = 0;
  uint32_t LostPackets = 0;
  std::string dataRate = "30Mbps";      /* Application layer datarate. */
  uint32_t packetSize = 1024;           /* Transport layer payload size in bytes. */
  double n_pkts_sec = 10;
  
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  // Set the maximum wireless range to 5 meters in order to reproduce a hidden nodes scenario, i.e. the distance between hidden stations is larger than 5 meters
  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (10));

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer wifiStaNodes0;
  wifiStaNodes0.Create(nWifi);
  NodeContainer wifiApNode0 = p2pNodes.Get(0);

  NodeContainer wifiStaNodes1;
  wifiStaNodes1.Create(nWifi);
  NodeContainer wifiApNode1 = p2pNodes.Get(1);

  YansWifiChannelHelper channel0=YansWifiChannelHelper::Default();
  channel0.AddPropagationLoss ("ns3::RangePropagationLossModel"); //wireless range limited to 5 meters!
  YansWifiPhyHelper phy0;
  phy0.SetChannel(channel0.Create());
  phy0.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper channel1=YansWifiChannelHelper::Default();
  channel1.AddPropagationLoss ("ns3::RangePropagationLossModel"); //wireless range limited to 5 meters!
  YansWifiPhyHelper phy1;
  phy1.SetChannel(channel1.Create());
  phy1.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  WifiHelper wifi0;
  wifi0.SetRemoteStationManager("ns3::AarfWifiManager");

  WifiHelper wifi1;
  wifi1.SetRemoteStationManager("ns3::AarfWifiManager");

   WifiMacHelper mac0;
    WifiMacHelper mac1;
    Ssid ssid0 = Ssid("ns-3-ssid");
    mac0.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue(ssid0),
                 "ActiveProbing", BooleanValue(false));

    Ssid ssid1 = Ssid("ns-3-ssid");
    mac1.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue(ssid1),
                 "ActiveProbing", BooleanValue(false));

    NetDeviceContainer staDevices0;
    staDevices0 = wifi0.Install(phy0, mac0, wifiStaNodes0);

    NetDeviceContainer staDevices1;
    staDevices1 = wifi1.Install(phy1, mac1, wifiStaNodes1);

    mac0.SetType("ns3::ApWifiMac",
                 "Ssid", SsidValue(ssid0));

    mac1.SetType("ns3::ApWifiMac",
                 "Ssid", SsidValue(ssid1));

    NetDeviceContainer apDevices0;
    apDevices0 = wifi0.Install(phy0, mac0, wifiApNode0);

    NetDeviceContainer apDevices1;
    apDevices1 = wifi1.Install(phy1, mac1, wifiApNode1);

  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  // AP is between the two stations, each station being located at 5 meters from the AP.
  // The distance between the two stations is thus equal to 10 meters.
  // Since the wireless range is limited to 5 meters, the two stations are hidden from each other.
  positionAlloc->Add (Vector (10.0, 0.0, 0.0));
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (20.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiStaNodes0);
    mobility.Install(wifiStaNodes1);
    mobility.Install(wifiApNode0);
    mobility.Install(wifiApNode1);

    InternetStackHelper stack;
    stack.Install(wifiStaNodes0);
    stack.Install(wifiApNode0);
    stack.Install(wifiStaNodes1);
    stack.Install(wifiApNode1);

 Ipv4AddressHelper address;
     address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer wifiInterfaces0, apInterfaces0;
    wifiInterfaces0 = address.Assign(staDevices0);
    apInterfaces0 = address.Assign(apDevices0);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer wifiInterfaces1, apInterfaces1;
    wifiInterfaces1 = address.Assign(staDevices1);
    apInterfaces1 = address.Assign(apDevices1);

    /* Populate routing table */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


  int num_half_flows = 5;
  for(int i = 0; i < num_half_flows; i++) {
    // UdpEchoServerHelper echoServer (9);
  
    // ApplicationContainer serverApps = echoServer.Install (wifiStaNodes0.Get (i));
    // serverApps.Start (Seconds (1.0));
    // serverApps.Stop (Seconds (30.0));

    // UdpEchoClientHelper echoClient (wifiInterfaces0.GetAddress (i), 9);
    //  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
    // echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    // echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));

    
    // ApplicationContainer clientApps = echoClient.Install (wifiStaNodes1.Get (i));
    // clientApps.Start (Seconds (2.0));
    // clientApps.Stop (Seconds (30.0));

      PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
      ApplicationContainer sinkApp = sinkHelper.Install(wifiStaNodes0.Get(i));
      sink = StaticCast<PacketSink>(sinkApp.Get(0));

      /* Install TCP/UDP Transmitter on the station */
      OnOffHelper server("ns3::TcpSocketFactory", (InetSocketAddress(wifiInterfaces0.GetAddress(i), 9)));
      server.SetAttribute("PacketSize", UintegerValue(packetSize));
      server.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
      server.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
      //server.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
      server.SetAttribute("DataRate", DataRateValue(n_pkts_sec*packetSize*8));
      ApplicationContainer serverApp = server.Install(wifiStaNodes1.Get(i+1));

      /* Start Applications */
      sinkApp.Start(Seconds(0.0));
      serverApp.Start(Seconds(1.0));

      Simulator::Stop(Seconds(3.0));
  }
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  FlowMonitorHelper flowmon;
   Ptr<FlowMonitor> monitor = flowmon.InstallAll();
   //Simulator::Stop (Seconds (30.0));

  int j=0;
  float AvgThroughput = 0;
  Time Delay;

  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
  {
	  //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);      
    SentPackets = SentPackets +(iter->second.txPackets);
    ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
    LostPackets = LostPackets + (iter->second.txPackets-iter->second.rxPackets);
    AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
    Delay = Delay + (iter->second.delaySum);
    
    j++;

}

  AvgThroughput = AvgThroughput/j;
  NS_LOG_UNCOND("--------Total Results of the simulation----------"<<std::endl);
  NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
  NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
  NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
  NS_LOG_UNCOND("Packet Loss ratio =" << ((LostPackets*100)/SentPackets)<< "%");
  NS_LOG_UNCOND("Packet delivery ratio =" << ((ReceivedPackets*100)/SentPackets)<< "%");
  NS_LOG_UNCOND("Average Throughput =" << AvgThroughput<< "Kbps");
  NS_LOG_UNCOND("End to End Delay =" << Delay);


  Simulator::Destroy ();
  return 0;
}
