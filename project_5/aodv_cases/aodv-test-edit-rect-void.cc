/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-client.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/random-direction-2d-mobility-model.h"
#include "ns3/energy-module.h"
#include "ns3/netanim-module.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/basic-energy-source.h"
#include "ns3/buildings-module.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <cmath>

using namespace ns3;

class GpsrExample
{
public:
  GpsrExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  void Report (std::ostream & os);

private:
  ///\name parameters
  //\{
  /// Number of nodes
  uint32_t size;
  /// Width of the Node Grid
  uint32_t gridWidth;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  //\}

  ///\name network
  //\{
  NodeContainer nodes;
  NodeContainer servernodes;
  NetDeviceContainer devices;
  NetDeviceContainer serverdevice;
  Ipv4InterfaceContainer interfaces;
  Ipv4InterfaceContainer servinterface;
  //\}

private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
};

int main (int argc, char **argv)
{
  GpsrExample test;
  if (! test.Configure(argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");


  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
GpsrExample::GpsrExample () :
  // Number of Nodes
  size (144),
  // Grid Width
  gridWidth(12),
  // Distance between nodes
  step (50), //TODO Distance changed to the limit between nodes: test to see if there are transmitions
  // Simulation time
  totalTime (10),
  // Generate capture files for each node
  pcap (false)

{
}

bool
GpsrExample::Configure (int argc, char **argv)
{
  // Enable GPSR logs by default. Comment this if too noisy
  // LogComponentEnable("GpsrRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed(12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);
  
  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  cmd.Parse (argc, argv);
  return true;
}

void
GpsrExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";
  
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  
  Simulator::Stop (Seconds (totalTime));
  

  
  AnimationInterface anim ("proj5-aodv.xml");
  //AnimationInterface anim2 ("proj5-gpsr-2.xml");
  anim.EnablePacketMetadata (true);
  //anim.UpdateNodeSize(servernodes.Get (0), 3.0, 3.0);
  anim.UpdateNodeColor(servernodes.Get (0), 0, 0, 255);
  //anim.EnableIpv4RouteTracking ("proj5-gpsr-2.xml", Seconds (1.0), Seconds (totalTime), Seconds(0.025));
  //anim.AddSourceDestination (3, "10.0.0.9");
  Simulator::Run ();
  
  uint32_t rxPacketsum = 0;
  double lastdelaysum = 0; 
  uint32_t txPacketsum = 0;
  uint32_t txBytessum = 0;
  uint32_t rxBytessum = 0;
  uint32_t txTimeFirst = 0;
  uint32_t rxTimeLast = 0;
  uint32_t lostPacketssum = 0;
  double jittersum = 0;
  
  monitor->SerializeToXmlFile("proj5-aodv-fm.xml", true, true);
  
  monitor->CheckForLostPackets ();
  
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	  if ((t.sourceAddress == "10.0.0.1") || (t.destinationAddress == "10.0.0.1"))
	  {
		  rxPacketsum += i->second.rxPackets;
		  txPacketsum += i->second.txPackets;
		  txBytessum += i->second.txBytes;
		  rxBytessum += i->second.rxBytes;
		  jittersum += i->second.jitterSum.GetSeconds();
		  lastdelaysum += i->second.lastDelay.GetSeconds();
		  lostPacketssum += i->second.lostPackets;
		  txTimeFirst += i->second.timeFirstTxPacket.GetSeconds();
		  rxTimeLast += i->second.timeLastRxPacket.GetSeconds();
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
  std::cout << "  Total Jitter: " << jittersum << "\n";
  std::cout << "  Total Delay: " << lastdelaysum << "\n";
  //double pollInterval = 15.0;
  
  
  Simulator::Destroy ();
}

void
GpsrExample::Report (std::ostream &)
{
}

void
GpsrExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes ";// << step << " m apart.\n";
  nodes.Create (size);
  
  servernodes.Create (1);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
     {
       std::ostringstream os;
       os << "node-" << i;
       Names::Add (os.str (), nodes.Get (i));
     }
    /* 
     double x_min = 70.0;
double x_max = 80.0;
double y_min = 	90.0;
double y_max = 100.0;
double z_min = 0.0;
double z_max = 10.0;
Ptr<Building> b = CreateObject <Building> ();
b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
b->SetBuildingType (Building::Residential);
b->SetExtWallsType (Building::ConcreteWithWindows);
//b->SetNFloors (3);
//b->SetNRoomsX (3);
//b->SetNRoomsY (2);
*/

  // Create static grid
  
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                "MinX", DoubleValue (0.0),
                                "MinY", DoubleValue (0.0),
                                "DeltaX", DoubleValue (step),
                                "DeltaY", DoubleValue (step),
                                "GridWidth", UintegerValue (gridWidth),
                                "LayoutType", StringValue ("RowFirst"));
  ObjectFactory pos1;
  pos1.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos1.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=575.0]"));
  pos1.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=270.0]"));
  Ptr<PositionAllocator> taPositionAlloc1 = pos1.Create ()->GetObject<PositionAllocator> ();
  
  mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue (Rectangle (0.0, 565.0, 0.0, 270.0)));
  
  uint32_t k = 0;
  for(k = 0; k < (size / 2); k++)
  {
	mobility.Install (nodes.Get (k));
  }
  
  //MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                "MinX", DoubleValue (635.0),
                                "MinY", DoubleValue (0.0),
                                "DeltaX", DoubleValue (step),
                                "DeltaY", DoubleValue (step),
                                "GridWidth", UintegerValue (gridWidth),
                                "LayoutType", StringValue ("RowFirst"));
  
  ObjectFactory pos2;
  pos2.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos2.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=625.0|Max=1200.0]"));
  pos2.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=270.0]"));
  Ptr<PositionAllocator> taPositionAlloc2 = pos2.Create ()->GetObject<PositionAllocator> ();
  
  mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue (Rectangle (635.0, 1200.0, 0.0, 270.0)));
  for(k = 0; k < (size / 2); k++)
  {
	mobility.Install (nodes.Get ((size / 2) + k));
  }
  
  //BuildingsHelper::Install (nodes);
  
  //BuildingsHelper::MakeMobilityModelConsistent ();
  
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
  positionAllocS->Add(Vector(600.0, 135.0, 0.0));
  mobility.SetPositionAllocator(positionAllocS);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install (servernodes);
  /*
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=20.0]"),
                                  //"Speed", StringValue (speedUniformRandomVariableStream.str ()),
                                  "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.5]"),
                                  //"Pause", StringValue (pauseConstantRandomVariableStream.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc)
                                  );
                                  
  
   
  
  MobilityHelper mobility;
  
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
  //positionAllocS->Add(Vector(0.0, 0.0, 0.0));
  //mobility.SetPositionAllocator(positionAllocS);
  mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel");
  mobility.Install (nodes);
  //positionAllocS->Add(Vector(-1.0, 0.0, 0.0));
  //mobility.SetPositionAllocator(positionAllocS);
  //mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  //mobility.Install (nodes.Get(0));
  */
  
}

void
GpsrExample::CreateDevices ()
{
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (104.0));
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);
  
  serverdevice = wifi.Install(wifiPhy, wifiMac, servernodes);

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("gpsr"));
    }
}

/*
void
DepletionHandler (void)
{
  NS_LOG_UNCOND ("Energy depleted!");
}
*/

/// Trace function for remaining energy at node.
void
RemainingEnergy (std::string context, double oldValue, double remainingEnergy)
{
	if(remainingEnergy == 0)
	{
		NS_LOG_UNCOND ("At " << Simulator::Now ().GetSeconds () << "s Node " << context << " died.");
	}
  //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Node " << context << " Current remaining energy = " << remainingEnergy << "J");
}

/// Trace function for total energy consumption at node.
void
TotalEnergy (std::string context, double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Node " << context << " Total energy consumed by radio = " << totalEnergy << "J");
}

void
GpsrExample::InstallInternetStack ()
{
  AodvHelper gpsr;
  // you can configure GPSR attributes here using gpsr.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (gpsr);
  stack.Install (nodes);
  stack.Install (servernodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.0.0");
  servinterface = address.Assign (serverdevice);
  interfaces = address.Assign (devices);
  
   /////////////-------- Energy Source and Device Energy Model  configuration -----------------------------------


  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (3600)); //1W-h
  // install source
  EnergySourceContainer sources = basicSourceHelper.Install (nodes);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
  // install device model
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, sources);
  
  //nodes.Get(0)->AggregateObject(sourceEnergy.Get(0));
  /***************************************************************************/

  //iterate on the node container an asign Energy model and Device to each Node
  int index = 0;
  for(NodeContainer::Iterator j= nodes.Begin(); j!= nodes.End(); ++j)
    {

//      Ptr<BasicEnergySource> energySource = CreateObject<BasicEnergySource>();
//      Ptr<WifiRadioEnergyModel> energyModel = CreateObject<WifiRadioEnergyModel>();
//
//      energySource->SetInitialEnergy (300);
//      energyModel->SetEnergySource (energySource);
//      energySource->AppendDeviceEnergyModel (energyModel);
//      energyModel->SetTxCurrentA(0.0174);
//
//      // aggregate energy source to node
//      Ptr<Node> object = *j;
//      object->AggregateObject (energySource);


      // adding tracing functions


      std::string context = static_cast<std::ostringstream*>( &(std::ostringstream() << index) )->str();
      //std::cout << "Connecting node " << context << std::endl;
      Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get (index));
      //nodes.Get(index) -> AggregateObject(basicSourcePtr);
      basicSourcePtr->TraceConnect ("RemainingEnergy", context, MakeCallback (&RemainingEnergy));

      // device energy model
      Ptr<DeviceEnergyModel> basicRadioModelPtr =
        basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);



      // device energy model
      // Ptr<DeviceEnergyModel> basicRadioModelPtr =
      //   basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
      NS_ASSERT (basicSourcePtr != NULL);
      basicSourcePtr->TraceConnect ("TotalEnergyConsumption", context, MakeCallback (&TotalEnergy));
      //nodes.Get (index)->AggregateObject (sources.Get (index));
      index++;
  }
  ////////////////////////////////////////////////////////////////////////////
    
    
  //server energy module
  /*
  Ptr<BasicEnergySource> energySource2 = CreateObject<BasicEnergySource>();
  Ptr<SimpleDeviceEnergyModel> energyModel2 = CreateObject<SimpleDeviceEnergyModel>();

  energySource2->SetInitialEnergy (300);
  energyModel2->SetEnergySource (energySource2);
  energySource2->AppendDeviceEnergyModel (energyModel2);
  energyModel2->SetCurrentA (20);

  // aggregate energy source to node
  servernodes.Get (0)->AggregateObject (energySource2);
  */
}

void
GpsrExample::InstallApplications ()
{

  uint16_t port = 9;  // well-known echo port number
  uint32_t packetSize = 1024; // size of the packets being transmitted
  uint32_t maxPacketCount = 100; // number of packets to transmit
  uint32_t timeInterval[size];
  timeInterval[0] = 15.2;
  //Time interPacketInterval = Seconds (5.0); // interval between packet transmissions

  // Set-up a server Application on the bottom-right node of the grid
  UdpEchoServerHelper server1 (port);
  //uint16_t server1Position = size-1; //bottom right
  ApplicationContainer apps = server1.Install (servernodes.Get(0));
  apps.Start (Seconds (2));
  apps.Stop (Seconds (totalTime));

	UdpEchoClientHelper client (servinterface.GetAddress (0), port);
	client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
//	client.SetAttribute ("Interval", TimeValue (Seconds (timeInterval)));
	client.SetAttribute ("PacketSize", UintegerValue (packetSize));

  // Set-up a client Application, connected to 'server', to be run on the top-left node of the grid
  for(uint16_t i = 0; i < size; i++)
  {
	timeInterval[i] = timeInterval[0] + (0.5 * i);
  }
  
  for(uint16_t i = 0; i < size; i++)
  {
	//uint16_t clientPosition = 0; //top left
	client.SetAttribute ("Interval", TimeValue (Seconds (timeInterval[i])));
	apps = client.Install (nodes.Get (i));
	apps.Start (Seconds (2.1 + (0.1 * i)));
	apps.Stop (Seconds (totalTime));
	
	//timeInterval = timeInterval + i;
	
  }
  
}
