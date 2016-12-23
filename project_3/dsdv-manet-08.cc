#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsdv-helper.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/position-allocator.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/udp-server.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>

using namespace ns3;
using namespace std;

uint16_t port = 9;

NS_LOG_COMPONENT_DEFINE ("DsdvManetExample");

class DsdvManetExample
{
public:
  DsdvManetExample ();
  void CaseRun (uint32_t nWifis,
                uint32_t nSinks,
                double totalTime,
                std::string rate,
                std::string phyMode,
                uint32_t nodeSpeed,
                uint32_t periodicUpdateInterval,
                uint32_t settlingTime,
                double dataStart,
                bool printRoutes,
                std::string CSVfileName);

private:
  uint32_t m_nWifis;
  uint32_t m_nSinks;
  double m_totalTime;
  std::string m_rate;
  std::string m_phyMode;
  uint32_t m_nodeSpeed;
  uint32_t m_periodicUpdateInterval;
  uint32_t m_settlingTime;
  double m_dataStart;
  uint32_t bytesTotal;
  uint32_t packetsReceived;
  bool m_printRoutes;
  std::string m_CSVfileName;
  
  string topology = "scratch/manet100.csv";
  
  double txrange = 50;
  
  uint32_t interval = 10;
  
  uint32_t size = 100;


  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  Address serverAddress[50];

private:
  void CreateNodes ();
  void CreateDevices (std::string tr_name);
  void InstallInternetStack (std::string tr_name);
  void InstallApplications ();
  void SetupMobility ();
  void ReceivePacket (Ptr <Socket> );
  Ptr <Socket> SetupPacketReceive (Ipv4Address, Ptr <Node> );
  void CheckThroughput ();

};

int main (int argc, char **argv)
{
  DsdvManetExample test;
  uint32_t nWifis = 50;
  uint32_t nSinks = 50;
  double totalTime = 100.0;
  std::string rate ("8kbps");
  std::string phyMode ("DsssRate11Mbps");
  uint32_t nodeSpeed = 10; // in m/s
  std::string appl = "all";
  uint32_t periodicUpdateInterval = 15;
  uint32_t settlingTime = 6;
  double dataStart = 50.0;
  bool printRoutingTable = true;
  std::string CSVfileName = "DsdvManetExample.csv";

  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi nodes[Default:30]", nWifis);
  cmd.AddValue ("nSinks", "Number of wifi sink nodes[Default:10]", nSinks);
  cmd.AddValue ("totalTime", "Total Simulation time[Default:100]", totalTime);
  cmd.AddValue ("phyMode", "Wifi Phy mode[Default:DsssRate11Mbps]", phyMode);
  cmd.AddValue ("rate", "CBR traffic rate[Default:8kbps]", rate);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model[Default:10]", nodeSpeed);
  cmd.AddValue ("periodicUpdateInterval", "Periodic Interval Time[Default=15]", periodicUpdateInterval);
  cmd.AddValue ("settlingTime", "Settling Time before sending out an update for changed metric[Default=6]", settlingTime);
  cmd.AddValue ("dataStart", "Time at which nodes start to transmit data[Default=50.0]", dataStart);
  cmd.AddValue ("printRoutingTable", "print routing table for nodes[Default:1]", printRoutingTable);
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name[Default:DsdvManetExample.csv]", CSVfileName);
  cmd.Parse (argc, argv);

  std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  std::endl;
  out.close ();

  SeedManager::SetSeed (12345);

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));

  test = DsdvManetExample ();
  test.CaseRun (nWifis, nSinks, totalTime, rate, phyMode, nodeSpeed, periodicUpdateInterval,
                settlingTime, dataStart, printRoutingTable, CSVfileName);

  return 0;
}

DsdvManetExample::DsdvManetExample ()
  : bytesTotal (0),
    packetsReceived (0)
{
}

void
DsdvManetExample::ReceivePacket (Ptr <Socket> socket)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
  Ptr <Packet> packet;
  while ((packet = socket->Recv ()))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
    }
}

void
DsdvManetExample::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << "," << kbs << "," << packetsReceived << "," << m_nSinks << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &DsdvManetExample::CheckThroughput, this);
}

Ptr <Socket>
DsdvManetExample::SetupPacketReceive (Ipv4Address addr, Ptr <Node> node)
{

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr <Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback ( &DsdvManetExample::ReceivePacket, this));

  return sink;
}

void
DsdvManetExample::CaseRun (uint32_t nWifis, uint32_t nSinks, double totalTime, std::string rate,
                           std::string phyMode, uint32_t nodeSpeed, uint32_t periodicUpdateInterval, uint32_t settlingTime,
                           double dataStart, bool printRoutes, std::string CSVfileName)
{
  m_nWifis = nWifis;
  m_nSinks = nSinks;
  m_totalTime = totalTime;
  m_rate = rate;
  m_phyMode = phyMode;
  m_nodeSpeed = nodeSpeed;
  m_periodicUpdateInterval = periodicUpdateInterval;
  m_settlingTime = settlingTime;
  m_dataStart = dataStart;
  m_printRoutes = printRoutes;
  m_CSVfileName = CSVfileName;
  

  std::stringstream ss;
  ss << m_nWifis;
  std::string t_nodes = ss.str ();

  std::stringstream ss3;
  ss3 << m_totalTime;
  std::string sTotalTime = ss3.str ();

  std::string tr_name = "Dsdv_Manet_" + t_nodes + "Nodes_" + sTotalTime + "SimTime";
  std::cout << "Trace file generated is " << tr_name << ".tr\n";

  CreateNodes ();
  CreateDevices (tr_name);
  SetupMobility ();
  InstallInternetStack (tr_name);
  InstallApplications ();

  //std::cout << "\nStarting simulation for " << m_totalTime << " s ...\n";

  CheckThroughput ();

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

void
DsdvManetExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned) size << " nodes.\n";
  nodes.Create (size);
  NS_ASSERT_MSG (m_nWifis >= m_nSinks, "Sinks must be less or equal to the number of nodes in network");
}

void
DsdvManetExample::SetupMobility ()
{
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
  
  
  std::string line;
  ifstream file(topology);
  
  uint16_t i = 0																																																																																																																																																																																																																																																																														;
  double vec[3];
  
  if(file.is_open())
  {
	while(getline(file,line))
	{
		char seps[] = ",";
		char *token;
		token = strtok(&line[0], seps);
		while(token != NULL)
		{
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
  mobilityS.Install(nodes);
}

void
DsdvManetExample::CreateDevices (std::string tr_name)
{
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (txrange));
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (m_phyMode), "ControlMode",
                                StringValue (m_phyMode));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream (tr_name + ".tr"));
  wifiPhy.EnablePcapAll (tr_name);
}

void
DsdvManetExample::InstallInternetStack (std::string tr_name)
{
  DsdvHelper dsdv;
  dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (m_periodicUpdateInterval)));
  dsdv.Set ("SettlingTime", TimeValue (Seconds (m_settlingTime)));
  InternetStackHelper stack;
  stack.SetRoutingHelper (dsdv); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  if (m_printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ((tr_name + ".routes"), std::ios::out);
      dsdv.PrintRoutingTableAllAt (Seconds (m_periodicUpdateInterval), routingStream);
    }
    
  for(uint32_t i = 0; i < (size / 2); i++)
  {
	serverAddress[i] = Address (interfaces.GetAddress (i));
  }
}

void
DsdvManetExample::InstallApplications ()
{
/*
  for (uint32_t i = 0; i <= m_nSinks - 1; i++ )
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      Ptr<Socket> sink = SetupPacketReceive (nodeAddress, node);
    }

  for (uint32_t clientNode = 0; clientNode <= m_nWifis - 1; clientNode++ )
    {
      for (uint32_t j = 0; j <= m_nSinks - 1; j++ )
        {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (j), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

          if (j != clientNode)
            {
              ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              apps1.Start (Seconds (var->GetValue (m_dataStart, m_dataStart + 1)));
              apps1.Stop (Seconds (m_totalTime));
            }
        }
    }
    */
  uint16_t i = 0;
  uint16_t j = 0;
  uint16_t k = 0;
  //uint16_t n = 10;
  uint16_t port = 4000;
  UdpServerHelper server (port);
  ApplicationContainer apps;
  
  for(i = 0; i < (size / 2); i++)
  {
	apps = server.Install (nodes.Get (i));
  }
  
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (50.0));
  
  //Ptr<UdpServer> sermon = server.GetServer();
  
  // Create one UdpClient application to send UDP datagrams from node zero to
  // node one.
  
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
			apps = client.Install (nodes.Get (j));
		}
	}
  
	apps.Start (Seconds (interval_start));
	apps.Stop (Seconds (interval_end));
	interval_start = interval_end + 1.0;
	interval_end = interval_start + interval;
  }
  uint32_t rxPacketsum = 0;
  double Delaysum = 0; 
  uint32_t txPacketsum = 0;
  uint32_t txBytessum = 0;
  uint32_t rxBytessum = 0;
  uint32_t txTimeFirst = 0;
  uint32_t rxTimeLast = 0;
  uint32_t lostPacketssum = 0;
  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  Simulator::Stop (Seconds (10.0));
  //std::cout<<"Packets sent. \n";
  //if (tracing == true)
  //{
	//wifiPhy.EnablePcapAll (outputFilename);
	//wifiPhy.EnablePcap (outputFilename, nodes.Get (0));
	//csma.EnablePcap (outputFilename, csmaDevices.Get (0), true);
  //}
  Simulator::Run ();
  
  k = 0;
  
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
	  txTimeFirst += i->second.timeFirstTxPacket.GetSeconds();
	  rxTimeLast += i->second.timeLastRxPacket.GetSeconds();
	  
      
      if ((t.sourceAddress < "10.0.0.101") && (t.destinationAddress < "10.0.0.101"))
      {
		  if((i->second.txBytes >= 1000)||(i->second.rxBytes >= 1000))//&&(i->second.txBytes == i->second.rxBytes))
		  {
			  k++;
			  
			  
			  
		  }
		  
          //std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          //std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          //std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  //std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
      }
     }
  
  
  uint64_t timeDiff = (rxTimeLast - txTimeFirst);
  
  //uint64_t rcvd = sermon->GetReceived();
  //uint16_t lst = sermon->GetLost();
  //std::cout << k << "    " << "            " << rxTimeLast << "         " <<txTimeFirst<< "\n\n";
  //std::cout<<"rec pkts"<<rxPacketsum<<"\n";
  
  std::cout << "\n\n";
  //std::cout << "  Total Tx Packets: " << txPacketsum << "\n";
  //std::cout << "  Total Rx Packets: " << rxPacketsum << "\n";
  std::cout << "  Total Packets Lost: " << lostPacketssum << "\n";
  std::cout << "  Throughput: " << ((rxBytessum * 8.0) / timeDiff)/1024<<" Kbps"<<"\n";
  std::cout << "  Packets Delivery Ratio: " << (((txPacketsum - lostPacketssum) * 100) /txPacketsum) << "%" << "\n";
  Simulator::Destroy();
}

