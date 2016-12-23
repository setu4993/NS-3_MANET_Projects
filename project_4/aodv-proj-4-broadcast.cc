#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/aodv-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/animation-interface.h"
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 1;
  uint32_t nWifi = 10;
  bool tracing = true;
  
  double total_time = 15.0, interval_start, interval_end, interval;
  
  //double pauseTime = 0.2;
  //double nodeSpeed = 20.0;
  double txrange = 150.0;

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
      //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
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
  channel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (txrange));
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

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
  
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
  positionAllocS->Add(Vector(0.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAllocS);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  positionAllocS->Add(Vector(-1.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAllocS);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install (csmaNodes);
  
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
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
  
  
  //mobility.Install(node);

  
  
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

  InternetStackHelper stack;
  AodvHelper aodv;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
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
  
  OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("10.1.3.255"), 9)));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  ApplicationContainer app = onoff.Install (p2pNodes.Get (0));
  app.Start (Seconds (0.5));
  app.Stop (Seconds (total_time));
  
  interval = 1.0;
  interval_start = 1.0;
  interval_end = 0.0; //interval_start + interval;
  
  uint16_t iter = total_time / interval;
  
  
  for (j = 0; j < iter; j++)
  {
	interval_start = interval_end + 1.0;
	interval_end = interval_start + interval;
	//std::cout << "\n" << j;
	
	for (i = 0; i < nWifi; i++)
	{
		PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), 9)));
	    app = sink.Install (wifiStaNodes.Get (i)); // or it should be 1, i don't know
	    //app.Add (sink.Install (csmaNodes.Get (i)));
	    
		//clientApps.Start (Seconds (interval_start));
		//clientApps.Stop (Seconds (interval_end));
	}
	
  }
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  
  uint32_t rxPacketsum = 0;
  double Delaysum = 0; 
  double rxTimeSum = 0, txTimeSum = 0;
  uint32_t txPacketsum = 0;
  uint32_t txBytessum = 0;
  uint32_t rxBytessum = 0;
  uint32_t txTimeFirst = 0;
  uint32_t rxTimeLast = 0;
  uint32_t lostPacketssum = 0;
  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll(); 
  
  Simulator::Stop (Seconds (total_time));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("aodv");
      phy.EnablePcap ("aodv-test", staDevices.Get (3));
      //csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }
    
  

  AnimationInterface anim ("proj4-adov.xml");
  //AnimationInterface::SetBoundary (0, 0, 1000, 1000);

  Simulator::Run ();
  
  monitor->CheckForLostPackets ();
  
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	  
	  rxPacketsum += i->second.rxPackets;
	  txPacketsum += i->second.txPackets;
	  txBytessum += i->second.txBytes;
	  rxBytessum += i->second.rxBytes;
	  Delaysum += i->second.delaySum.GetSeconds();
	  lostPacketssum += i->second.lostPackets;
	  
	  if(txTimeFirst == 0)
	  {
		  txTimeFirst = i->second.timeFirstTxPacket.GetSeconds();
	  }
	  
	  rxTimeLast = i->second.timeLastRxPacket.GetSeconds();
	  lostPacketssum += i->second.lostPackets;
	  Delaysum += i->second.delaySum.GetSeconds();
	  txTimeSum += i->second.timeFirstTxPacket.GetSeconds();
	  rxTimeSum += i->second.timeLastRxPacket.GetSeconds();
	  
      if ((t.destinationAddress == "10.1.1.1"))
      {
		  if((i->second.txBytes >= 1000)&&(i->second.rxBytes >= 1000))//&&(i->second.txBytes == i->second.rxBytes))
		  {
			  
			//rxPacketsum += i->second.rxPackets;
			//std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
			//std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
			//std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
			//std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
		  }
      }
     }
  
    //std::cout << "\n\n" << rxPacketsum << "\n";
    uint64_t timeDiff = (rxTimeLast - txTimeFirst);
    double timeDiff2 = (rxTimeSum - txTimeSum) / rxPacketsum;
  
  //uint64_t rcvd = sermon->GetReceived();
  //uint16_t lst = sermon->GetLost();
  //std::cout << k << "    " << "            " << rxTimeLast << "         " <<txTimeFirst<< "\n\n";
  //std::cout<<"rec pkts"<<rxPacketsum<<"\n";
  
  //rxBytessum = sink_sock->GetTotalRx();
  
  std::cout << "  Total Tx Packets: " << txPacketsum << "\n";
  std::cout << "  Total Rx Packets: " << rxPacketsum << "\n";
  std::cout << "  Total Packets Lost: " << (txPacketsum - rxPacketsum) << "\n";
  std::cout << "  Average Round trip time of Packet: " << timeDiff2 << "\n";
  std::cout << "  Throughput: " << ((rxBytessum * 8.0) / timeDiff)/1024<<" Kbps"<<"\n";
  std::cout << "  Packets Loss Ratio: " << (((txPacketsum - rxPacketsum) * 100) /txPacketsum) << "%" << "\n";
  std::cout << "  Packets Delivery Ratio: " << ((rxPacketsum * 100) /txPacketsum) << "%" << "\n";
  Simulator::Destroy ();
  return 0;
}
