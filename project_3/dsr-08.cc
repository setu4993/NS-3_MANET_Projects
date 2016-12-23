#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsr-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/udp-server.h"
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DsrTest");

int
main (int argc, char *argv[])
{
#if 0
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_ALL);
  LogComponentEnable ("NetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv4EndPointDemux", LOG_LEVEL_ALL);
#endif

#if 0
  LogComponentEnable ("DsrOptions", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrRouting", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrOptionHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrFsHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrGraReplyTable", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrSendBuffer", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrRouteCache", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrMaintainBuffer", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrRreqTable", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrErrorBuffer", LOG_LEVEL_ALL);
  LogComponentEnable ("DsrNetworkQueue", LOG_LEVEL_ALL);
#endif

  NS_LOG_INFO ("creating the nodes");

  // General parameters
  uint32_t nWifis = 10;
  uint32_t nSinks = 10;
  double TotalTime = 100.0;
  //double dataTime = 500.0;
  //double ppers = 1;
  uint32_t packetSize = 64;
  //double dataStart = 100.0; // start sending data at 100s
  string topology = "scratch/manet100.csv";
  
  uint32_t size = nWifis + nSinks;
  
  //mobility parameters
  double pauseTime = 0.0;
  double nodeSpeed = 20.0;
  double txpDistance = 500;
  
  Address serverAddress[50];

  std::string rate = "0.512kbps";
  std::string dataMode ("DsssRate11Mbps");
  std::string phyMode ("DsssRate11Mbps");

  //Allow users to override the default parameters and set it to new ones from CommandLine.
  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi nodes", nWifis);
  cmd.AddValue ("nSinks", "Number of SINK traffic nodes", nSinks);
  cmd.AddValue ("rate", "CBR traffic rate(in kbps), Default:8", rate);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model, Default:20", nodeSpeed);
  cmd.AddValue ("packetSize", "The packet size", packetSize);
  cmd.AddValue ("txpDistance", "Specify node's transmit range, Default:300", txpDistance);
  cmd.AddValue ("pauseTime", "pauseTime for mobility model, Default: 0", pauseTime);
  cmd.Parse (argc, argv);

  //SeedManager::SetSeed (10);
  //SeedManager::SetRun (1);

  NodeContainer adhocNodes;
  adhocNodes.Create (size);
  NetDeviceContainer allDevices;

  //std::cout << "Creating " << (unsigned)size << " nodes with transmission range " << txrange << "m.\n";
  
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
  
  
  std::string line;
  ifstream file(topology);
  
  uint16_t i = 0																																																																																																																																																																																																																																																																														;
  double vec[3];
  
  if(file.is_open())
  {
	while(getline(file,line))
	{
		
		//std::cout<<line<< '\n';
		char seps[] = ",";
		char *token;

		token = strtok(&line[0], seps);
		
		//std::cout << token << "\n";
		
		while(token != NULL)
		{
			//printf("[%s]\n", token);
			vec[i] = atof(token);
			i++;
			token = strtok (NULL, ",");
			if(i == 3)
			{
				//std::cout << "\n" << vec[0] << "  " << vec[1] << "   " << vec[2] << "\n";
				positionAllocS->Add(Vector(vec[1], vec[2], 0.0));
				i = 0;
			}
        }

	  }
	  file.close();
	}
	else
	{
	  std::cout<<"Error in csv file"<< '\n';
	}

  MobilityHelper mobilityS;
  mobilityS.SetPositionAllocator(positionAllocS);
  mobilityS.SetMobilityModel("ns3::ConstantPositionMobilityModel"); //whatever it is
  mobilityS.Install (adhocNodes);

  /* NS_LOG_INFO ("setting the default phy and channel parameters");
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));

  NS_LOG_INFO ("setting the default phy and channel parameters "); */
  
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (txpDistance));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (dataMode), "ControlMode",
                                StringValue (phyMode));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  allDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);

  NS_LOG_INFO ("Configure Tracing.");
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("dsrtest.tr");
  wifiPhy.EnableAsciiAll (stream);

  //AsciiTraceHelper ascii;
  //Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("dsrtest.tr");
  //wifiPhy.EnableAsciiAll (stream);
/*
  MobilityHelper adhocMobility;
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
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
  adhocMobility.Install (adhocNodes);
*/
  InternetStackHelper internet;
  DsrMainHelper dsrMain;
  DsrHelper dsr;
  internet.Install (adhocNodes);
  dsrMain.Install (dsr, adhocNodes);

  NS_LOG_INFO ("assigning ip address");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer allInterfaces;
  allInterfaces = address.Assign (allDevices);

  uint16_t port = 9;
  //double randomStartTime = (1 / ppers) / nSinks; //distributed btw 1s evenly as we are sending 4pkt/s

 /* for (uint32_t i = 0; i < nSinks; ++i)
    {
      PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer apps_sink = sink.Install (adhocNodes.Get (i));
      apps_sink.Start (Seconds (0.0));
      apps_sink.Stop (Seconds (TotalTime));

      OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (allInterfaces.GetAddress (i), port)));
      onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
      onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
      onoff1.SetAttribute ("DataRate", DataRateValue (DataRate (rate)));

      ApplicationContainer apps1 = onoff1.Install (adhocNodes.Get (i + nWifis - nSinks));
      apps1.Start (Seconds (dataStart + i * randomStartTime));
      apps1.Stop (Seconds (dataTime + i * randomStartTime));
    }
*/


  uint16_t j = 0;
  uint16_t k = 0;
  //uint16_t n = 10;
  //uint16_t port = 4000;
  UdpServerHelper server (port);
  ApplicationContainer apps;
  
  for(i = 0; i < nSinks; i++)
  {
	apps = server.Install (adhocNodes.Get (i));
	serverAddress[i] = Address (allInterfaces.GetAddress (i));
  }
  
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (TotalTime));
  
  
  uint32_t interval = 5;
  uint32_t MaxPacketSize = 1024;
  Time interPacketInterval = Seconds (0.01);
  uint32_t maxPacketCount = 3;
  double interval_start = 2.0, interval_end = interval_start + interval;
  //std::cout << "Sending packets now.\n\n";
  for(k = 1; k <= (size / 2); k++)
  {
	for(i = 0; i < k; i++)
	{
		UdpClientHelper client (serverAddress[i], port);
		client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
		client.SetAttribute ("Interval", TimeValue (interPacketInterval));
		client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
		for(j = (size / 2); j < ((size / 2) + k); j++)
		{
			apps = client.Install (adhocNodes.Get (j));
		}
	}
  
	apps.Start (Seconds (interval_start));
	apps.Stop (Seconds (interval_end));
	interval_start = interval_end + 1.0;
	interval_end = interval_start + interval;
  }
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (TotalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

