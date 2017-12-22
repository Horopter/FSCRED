#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h" 
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h" 
#include "ns3/animation-interface.h" 
#include "ns3/point-to-point-layout-module.h" 
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h" 
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-helper.h" 
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h" 
#include <iostream>
#include <fstream> 
#include <vector> 
#include <string>
#include <cstdlib>


using namespace ns3;
//using namespace std;

/*
 * 
 */

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

std::string GetRedFormat(std::string red)
{
	if(red == "RED")
		return "ns3::RedQueueDisc";
	if(red == "ARED")
		return "ns3::RedQueueDisc";
	if(red == "HRED")
		return "ns3::HeuristicRedQueueDisc";
	if(red == "FRED")
		return "ns3::FuzzyRedQueueDisc";
	if(red == "SFLRED")
		return "ns3::SflRedQueueDisc";
	if(red == "CSFLRED")
		return "ns3::CsflRedQueueDisc";
	return "";
}

void SetRedFormat(std::string red)
{
	if(red == "RED")
		Config::SetDefault("ns3::RedQueueDisc::ARED",BooleanValue(false));
	if(red == "ARED")
		Config::SetDefault("ns3::RedQueueDisc::ARED",BooleanValue(true));
	if(red == "HRED")
		Config::SetDefault("ns3::HeuristicRedQueueDisc::HRED",BooleanValue(true));
	if(red == "FRED")
		Config::SetDefault("ns3::FuzzyRedQueueDisc::FRED",BooleanValue(true));
	if(red == "SFLRED")
		Config::SetDefault("ns3::SflRedQueueDisc::SFLRED",BooleanValue(true));
	if(red == "CSFLRED")
		Config::SetDefault("ns3::CsflRedQueueDisc::CSFLRED",BooleanValue(true));
}

int main(int argc, char* argv[]) {

/* default parameters start */


  std::string tcp_variant = "TcpNewReno";
  std::string bandwidth = "5Mbps";
 std::string delay = "2ms";
  double error_rate = 0.000001;
  int simulation_time = 20; //seconds
  bool tracing = true;
  std::string red = "RED";



/* default parameters end */
	

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId()));
  
    NS_LOG_COMPONENT_DEFINE("net_layer");
    uint16_t num_Nodes = 12;
	
 /** configuration parameter for UDP flow only **/
   	
	uint32_t PacketSize = 1460; // bytes 
	std::string UDPRate ("5Mbps"); 


/** setting up default configuration **/

    CommandLine cmd;
   cmd.AddValue("PacketSize", "size of application packet sent", PacketSize);
    cmd.AddValue("DataRate", "rate of pacekts sent", UDPRate);
    cmd.AddValue("tracing", "turn on ascii and pcap tracing", tracing);
    cmd.AddValue("red","Red format",red);
    cmd.Parse(argc, argv);


	if(GetRedFormat(red)=="")
		{
			std::cout<<"Red format not found.\n";
			exit(1);
		}
	else
		SetRedFormat(red);

    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(PacketSize));
   Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(UDPRate));
    Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

    /* configuration for NETAnim Animator **/
    ns3::PacketMetadata::Enable();
    std::string animFile = "topology_"+red+".xml";

    

	
/** Network Topology Creation **/    
    NodeContainer nodes;
    NodeContainer link[num_Nodes-1];
    Ipv4AddressHelper ipv4;
    NetDeviceContainer d_link[num_Nodes-1];
    Ipv4InterfaceContainer iic[num_Nodes-1];
    int i, num_nodes_on_one_side;
    Ipv4Address address[num_Nodes];
    nodes.Create(num_Nodes);
    num_nodes_on_one_side = num_Nodes/2;
    address[0] = "10.1.1.0";
    address[1] = "10.1.2.0";
    address[2] = "10.1.3.0";
    address[3] = "10.1.4.0";
    address[4] = "10.1.5.0";
    address[5] = "10.1.6.0";
    address[6] = "10.1.7.0";
    address[7] = "10.1.8.0";
    address[8] = "10.1.9.0";
    address[9] = "10.1.10.0";
    address[10] = "10.1.11.0";
    for(i=0; i < num_nodes_on_one_side-1 ; i++)
    {
    	link[i] = NodeContainer(nodes.Get(i), nodes.Get(num_nodes_on_one_side-1));
    }
   
    link[i] = NodeContainer(nodes.Get(num_nodes_on_one_side-1), nodes.Get(num_Nodes-1));
    
    for(i = num_nodes_on_one_side; i < num_Nodes-1 ; i++)
    {
    	link[i] = NodeContainer(nodes.Get(i), nodes.Get(num_Nodes-1));

    }


/** Point to Point Link Helper which sets data rate and delay value for Link **/
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
    p2p.SetChannelAttribute ("Delay", StringValue (delay));

 /** Setting up each link between two nodes **/
    
    for(i=0; i < num_Nodes-1 ; i++)
    {
	   d_link[i] = p2p.Install(link[i]);
    }  

  /*  Error Model at Reciever end  */
 Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
 em->SetAttribute ("ErrorRate", DoubleValue (error_rate));
 d_link[9].Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 d_link[10].Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); 


    NS_LOG_INFO("Flow Monitor");
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4GlobalRoutingHelper globalRouting;
    Ipv4ListRoutingHelper list;
    list.Add(staticRouting, 0);
    list.Add(globalRouting, 10);
    // Install network stacks on the nodes 
    InternetStackHelper internet;
    internet.SetRoutingHelper(list);
    internet.Install(nodes);


  	TrafficControlHelper tchBottleneck;
          QueueDiscContainer queueDiscs;
          tchBottleneck.SetRootQueueDisc (GetRedFormat(red));
          queueDiscs = tchBottleneck.Install (d_link[6]);



/** Assigning Network Base Address to Each node **/

    for(i=0; i < num_Nodes-1 ; i++)
    {
	ipv4.SetBase(address[i], "255.255.255.0");
	//address  = htonl(ntohl(address) + 1);
 	iic[i] = ipv4.Assign(d_link[i]);
    	iic[i].SetMetric(0, 2);
	iic[i].SetMetric(1, 2);
		
    }
	
     Ipv4GlobalRoutingHelper::PopulateRoutingTables();


//Reciver setup
  uint16_t sinkPort = 8080;
  Address sinkAddress1 (InetSocketAddress (iic[num_Nodes-2].GetAddress (0), sinkPort));
  Address sinkAddress2 (InetSocketAddress (iic[num_Nodes-3].GetAddress (0), sinkPort));
  Address sinkAddress3 (InetSocketAddress (iic[num_Nodes-4].GetAddress (0), sinkPort));
  Address sinkAddress4 (InetSocketAddress (iic[num_Nodes-5].GetAddress (0), sinkPort));
  Address sinkAddress5 (InetSocketAddress (iic[num_Nodes-6].GetAddress (0), sinkPort));

  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps1 = packetSinkHelper.Install(nodes.Get(10));
  ApplicationContainer sinkApps2 = packetSinkHelper.Install(nodes.Get(9));
  ApplicationContainer sinkApps3 = packetSinkHelper.Install(nodes.Get(8));
  ApplicationContainer sinkApps4 = packetSinkHelper.Install(nodes.Get(7));
  ApplicationContainer sinkApps5 = packetSinkHelper.Install(nodes.Get(6));

  sinkApps1.Start (Seconds (0.));
  sinkApps1.Stop (Seconds (simulation_time));
  sinkApps2.Start (Seconds (0.));
  sinkApps2.Stop (Seconds (simulation_time));
  sinkApps3.Start (Seconds (0.));
  sinkApps3.Stop (Seconds (simulation_time));
  sinkApps4.Start (Seconds (0.));
  sinkApps4.Stop (Seconds (simulation_time));
  sinkApps5.Start (Seconds (0.));
  sinkApps5.Stop (Seconds (simulation_time));



//Sender Setup 
  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (nodes.Get(0), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (nodes.Get(1), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (nodes.Get(2), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (nodes.Get(3), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (nodes.Get(4), TcpSocketFactory::GetTypeId ());


  Ptr<MyApp> app1 = CreateObject<MyApp> ();
  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  Ptr<MyApp> app3 = CreateObject<MyApp> ();
  Ptr<MyApp> app4 = CreateObject<MyApp> ();
  Ptr<MyApp> app5 = CreateObject<MyApp> ();


  //configuration parameter for TCP Flow 
  app1->Setup (ns3TcpSocket1, sinkAddress1, 1460, 1000000, DataRate("5Mbps"));
  app2->Setup (ns3TcpSocket2, sinkAddress2, 1460, 1000000, DataRate("5Mbps"));
  app3->Setup (ns3TcpSocket3, sinkAddress3, 1460, 1000000, DataRate("5Mbps"));
  app4->Setup (ns3TcpSocket4, sinkAddress4, 1460, 1000000, DataRate("5Mbps"));
  app5->Setup (ns3TcpSocket5, sinkAddress5, 1460, 1000000, DataRate("5Mbps"));

  nodes.Get (0)->AddApplication (app1);
  nodes.Get (1)->AddApplication (app2);
  nodes.Get (2)->AddApplication (app3);
  nodes.Get (3)->AddApplication (app4);
  nodes.Get (4)->AddApplication (app5);

  app1->SetStartTime (Seconds (1.));
  app1->SetStopTime (Seconds (simulation_time));
  app2->SetStartTime (Seconds (1.));
  app2->SetStopTime (Seconds (simulation_time));
  app3->SetStartTime (Seconds (1.));
  app3->SetStopTime (Seconds (simulation_time));
  app4->SetStartTime (Seconds (1.));
  app4->SetStopTime (Seconds (simulation_time));
  app5->SetStartTime (Seconds (1.));
  app5->SetStopTime (Seconds (simulation_time));

  //trace cwnd
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("topology_"+red+".cwnd");
  ns3TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
  ns3TcpSocket2->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
  ns3TcpSocket3->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
  ns3TcpSocket4->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
  ns3TcpSocket5->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));




/** NetAnim Animator Setup **/

    AnimationInterface anim(animFile);
    Ptr<Node> n = nodes.Get(0);
    anim.SetConstantPosition(n, 0, 0);
    n = nodes.Get(1);
    anim.SetConstantPosition(n, 0, 2);
    n = nodes.Get(2);
    anim.SetConstantPosition(n, 0, 4);
    n = nodes.Get(3);
    anim.SetConstantPosition(n, 0, 6);
    n = nodes.Get(4);
    anim.SetConstantPosition(n, 0, 8);
    n = nodes.Get(5);
    anim.SetConstantPosition(n, 4, 4);
    n = nodes.Get(6);
    anim.SetConstantPosition(n, 10, 0);
    n = nodes.Get(7);
    anim.SetConstantPosition(n, 10, 2);
    n = nodes.Get(8);
    anim.SetConstantPosition(n, 10, 4);
    n = nodes.Get(9);
    anim.SetConstantPosition(n, 10, 6);
    n = nodes.Get(10);
    anim.SetConstantPosition(n, 10, 8);
    n = nodes.Get(11);
    anim.SetConstantPosition(n, 6, 4);


    if (tracing == true) {
        AsciiTraceHelper ascii;
        p2p.EnableAsciiAll(ascii.CreateFileStream("topology_"+red+".tr"));
        p2p.EnablePcapAll("topology_"+red);
    }




Ptr<OutputStreamWrapper> stream1 = Create<OutputStreamWrapper> ("Table_"+red, std::ios::out); 
Ipv4GlobalRoutingHelper helper2;
helper2.PrintRoutingTableAllAt(Seconds(2.0),stream1);





/**flow monitor **/


// 1. Install FlowMonitor on all nodes
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

// 2. Run simulation for 60 seconds
Simulator::Stop (Seconds (60));
Simulator::Run ();
// 3. Print per flow statistics
monitor->CheckForLostPackets ();
Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
{
Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
std::cout << "Flow " << i->first - 1 << " (" << t.sourceAddress << ":"<<t.sourcePort<< " -> " << t.destinationAddress << ":"<<t.destinationPort << ")\n";std::cout << " Tx Bytes: " << i->second.txBytes << "\n";
std::cout << " Rx Bytes: " << i->second.rxBytes << "\n";
std::cout << " Throughput: " << i->second.rxBytes * 8.0 /(i->second.timeLastRxPacket.GetSeconds()- i->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024 << " Mbps \n"; 
}

Simulator::Destroy ();
return 0;
}
