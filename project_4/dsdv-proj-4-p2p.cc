#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/dsdv-helper.h"
#include "ns3/dsdv-module.h"
#include "ns3/flow-monitor-module.h"
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 0;
  uint32_t nWifi = 100;
  bool tracing = true;
  
  double total_time = 150.0, interval_start, interval_end, interval;
  
  //double pauseTime = 0.2;
  //double nodeSpeed = 20.0;
  //double txrange = 150.0;
  
  //uint32_t bytesTotal;
  //uint32_t packetsReceived;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses 
  // soon become an issue
  if (nWifi > 250 || nCsma > 250)
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (0));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  //channel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (txrange));
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");

  WifiMacHelper mac;
  //Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  
  //MobilityHelper mobility;
  
  /*
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  */
  
  MobilityHelper mobility;
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=20.0]"),
                                  //"Speed", StringValue (speedUniformRandomVariableStream.str ()),
                                  "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.5]"),
                                  //"Pause", StringValue (pauseConstantRandomVariableStream.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc)
                                  );
  
  mobility.Install (wifiStaNodes);

  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
  positionAllocS->Add(Vector(0.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAllocS);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  //mobility.Install(node);

  mobility.Install (wifiApNode);
  
  /*
  MobilityHelper adhocMobility;
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();

  std::ostringstream speedUniformRandomVariableStream;
  speedUniformRandomVariableStream << "ns3::UniformRandomVariable[Min=0.0|Max="
                                   << nodeSpeed
                                   << "]";

  std::ostringstream pauseConstantRandomVariableStream;
  pauseConstantRandomVariableStream << "ns3::ConstantRandomVariable[Constant="
                                   << pauseTime
                                   << "]";

  adhocMobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  //                                  "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=nodeSpeed]"),
                                  "Speed", StringValue (speedUniformRandomVariableStream.str ()),
                                  "Pause", StringValue (pauseConstantRandomVariableStream.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc)
                                  );
  adhocMobility.Install (wifiStaNodes);
  */

  
  DsdvHelper dsdv;
  InternetStackHelper stack;
  
  dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (100)));
  dsdv.Set ("SettlingTime", TimeValue (Seconds (50)));
  stack.SetRoutingHelper (dsdv); // has effect on the next Install ()
  /*
  Ipv4ListRoutingHelper list;
  std::string m_protocolName;
  list.Add (dsdv, 100);
  m_protocolName = "DSDV";
  stack.SetRoutingHelper (list);
  */
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  Ipv4InterfaceContainer wifiInterfaces;
  address.SetBase ("10.1.3.0", "255.255.255.0");
  wifiInterfaces = address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);
  
  uint32_t i = 0, j = 0;
  
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  
  for (i = 0; i < nWifi; i++)
  {
	ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (i));
	serverApps.Start (Seconds (0.1));
	serverApps.Stop (Seconds (total_time));
	/*
	Ptr<Node> node = NodeList::GetNode (i);
    Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
    InetSocketAddress local = InetSocketAddress (nodeAddress, 9);
    Ptr<Socket> sink = Socket::CreateSocket (node, tid);
    sink->Bind (local);
    //sink->SetRecvCallback (MakeCallback ( &DsdvManetExample::ReceivePacket, this));
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
	Ptr <Packet> packet;
	while ((packet = sink->Recv ()))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
    }
	*/
	
  }
  
  interval = 9.0;
  interval_start = 1.0;
  interval_end = 0.0; //interval_start + interval;
  
  for (j = 0; j < 10; j++)
  {
	interval_start = interval_end + 1.0;
	interval_end = interval_start + interval;
	//std::cout << "\n" << j;
	
	for (i = 0; i < nWifi; i++)
	{
		UdpEchoClientHelper echoClient (wifiInterfaces.GetAddress (i), 9);
		echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
		echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
		echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
	
		ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));  
		clientApps.Start (Seconds (interval_start));
		clientApps.Stop (Seconds (interval_end));
	}
	
  }
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  
  uint32_t rxPacketsum = 0;
  uint32_t rxPacketsum2 = 0;
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  Simulator::Stop (Seconds (total_time));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }
    
  //AnimationInterface::SetBoundary (0, 0, 1000, 1000);

  //AnimationInterface anim ("proj4-dsdv.xml");
  

  Simulator::Run ();
  
  monitor->CheckForLostPackets ();
  
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	        
      if ((t.destinationAddress == "10.1.1.1"))
      {
		  if((i->second.txBytes >= 1000)&&(i->second.rxBytes >= 1000))//&&(i->second.txBytes == i->second.rxBytes))
		  {
			rxPacketsum += i->second.rxPackets;
			std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
			std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
			std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
			//std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
		  }
      }
      if ((t.destinationAddress < "10.1.3.100"))
      {
		  //if((i->second.txBytes >= 1000)&&(i->second.rxBytes >= 1000))//&&(i->second.txBytes == i->second.rxBytes))
		  //{
			rxPacketsum2 += i->second.rxPackets;
			std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
			std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
			std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
			//std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
		  //}
      }
     }
  
    std::cout << "\n\n" << rxPacketsum << "\n";
    std::cout << "\n\n" << rxPacketsum2 << "\n";
  Simulator::Destroy ();
  return 0;
}
