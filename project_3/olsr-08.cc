
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/udp-server.h"
#include "ns3/propagation-loss-model.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("SimplePointToPointOlsrExample");

int
main (int argc, char *argv[])
{   uint32_t size=30;
    double simTime=10;
    string topology = "scratch/manet100.csv";
	double txrange = 50;
	uint32_t interval = 10;

  
  bool tracing = true;
  
  char* outputFilename = (char *)"manet";

  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if 0
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

  // Set up some default values for the simulation.  Use the

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  Address serverAddress[50];
  UdpServer sermon;
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  WifiMacHelper wifiMac;

  nodes.Create (size);
  Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
    
    
    std::string line;
    ifstream file(topology);
    
    uint16_t i = 0;
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
    mobilityS.Install(nodes);



  // Enable OLSR
  NS_LOG_INFO ("Enabling OLSR Routing.");
  OlsrHelper olsr;
  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                   "MaxRange", DoubleValue (txrange));
    wifiPhy.SetChannel (wifiChannel.Create ());
    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
    devices = wifi.Install (wifiPhy, wifiMac, nodes);
	AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("olsr.tr");
  wifiPhy.EnableAsciiAll (stream);
  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
    InternetStackHelper stack;
    stack.SetRoutingHelper (olsr); // has effect on the next Install ()
    stack.Install (nodes);
    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.0.0.0");
    interfaces = address.Assign (devices);
    for(uint32_t i = 0; i < (size / 2); i++)
    {
        serverAddress[i] = Address (interfaces.GetAddress (i));
    }
    
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
    apps.Stop (Seconds (simTime));

  // Create a packet sink to receive these packets
 // PacketSinkHelper sink ("ns3::UdpSocketFactory",
           //              InetSocketAddress (Ipv4Address::GetAny (), port));

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
    if (tracing == true)
    {
        wifiPhy.EnablePcapAll (outputFilename);
        //wifiPhy.EnablePcap (outputFilename, nodes.Get (0));
        //csma.EnablePcap (outputFilename, csmaDevices.Get (0), true);
    }
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
            
            std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
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
    Simulator::Destroy ();

  return 0;
}
