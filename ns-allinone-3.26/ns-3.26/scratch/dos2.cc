/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 *
 * Author : Rahul Hada <hada.rahul@gmail.com>
 *
 * In this scenario create I used a LDosApp application for 
 *  sending packets to Sink Node through bottleneck link.
 *
 * Scenario Outcome
 * i)To understand the bottleneck situation using following conditions :-
 *      a) Making link-0 , link-2 as high bandwidth link and low bandwidth link-1.
 *      b) In previous step now change the MTU of link-1 from default to 512 (i.e less then link-0 & link-2)
 *ii)To introduce the netanim for animation output
 *
 * Topology
 *                       <link-0>                                <link-2>
 *		        nr0__________					     __________nl0
 *					   |		    <link-1>	    |
 *					   |       <bottleneck link>	    |
 *					   -nb1------------------------nb2-
 *				  	   |					    |
 *	  	   	     __________|					    |__________nl1
 *		       nr2
 *
 * Learning Outcome : Effect on throughput because of bottleneck links
 *			    The end-to-end throughput is even less then the bottleneck link-1 DataRate
 *                    Analyze the behaviour of TCp variants like Reno , Teho , NewReno in above 
 *                    scenario.
 */

#include "ns3/point-to-point-layout-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DosAttack");

class LDosApp:public Application
{
public :
	LDosApp();
	virtual ~LDosApp();
	static TypeId GetTypeId(void);
	void SetupModule(Ptr<Socket> socket , Address address,uint32_t packetSize,uint32_t nPackets,DataRate dataRate);
private :
	virtual void StartApp(void);
	virtual void EndApp(void);
	void ScheduleTx(void);
	void SendPacket(void);

	Ptr<Socket> m_socket;
	Address m_peer;
	uint32_t m_packetSize;
	uint32_t m_nPackets;
	DataRate m_dataRate;
	EventId m_sendEvent;
	bool m_running;
	uint32_t m_packetsSent;
};

LDosApp::LDosApp()
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
LDosApp::~LDosApp()
{
	m_socket=0;
}

TypeId LDosApp::GetTypeId()
{
	static TypeId tid = TypeId("LDosApp")
			.SetParent<Application>()
			.SetGroupName("Tutorial")
			.AddConstructor<LDosApp>()
			;
	return tid;

}
void LDosApp::SetupModule(Ptr<Socket> socket,Address address,uint32_t packetSize,uint32_t nPackets,DataRate dataRate)
{
	m_socket=socket;
	m_peer=address;
	m_packetSize=packetSize;
	m_nPackets=nPackets;
	m_dataRate=dataRate;
}
void LDosApp::StartApp(void)
{
	m_running = true;
	m_packetsSent=0;
	m_socket->Bind();
	m_socket->Connect(m_peer);
	SendPacket();
}

void LDosApp::EndApp(void)
{
	m_running=false;
	if(m_sendEvent.IsRunning())
	{
		Simulator::Cancel(m_sendEvent);
	}
	if(m_socket)
	{
		m_socket->Close();
	}

}
void LDosApp::SendPacket(void)
{
	Ptr<Packet> packet = Create<Packet> (m_packetSize);
	m_socket->Send(packet);
	if(++m_packetsSent <m_nPackets)
	{
		ScheduleTx();
	}
}

void LDosApp::ScheduleTx(void)
{
	if(m_running)
	{
		Time tNext(Seconds(m_packetSize *8/static_cast<double> (m_dataRate.GetBitRate())));
		m_sendEvent=Simulator::Schedule(tNext,&LDosApp::SendPacket,this);
	}
}

static void SendData (Ptr<const Packet> p)
{
  std::cout<<" Send Data at " << Simulator::Now ().GetSeconds()<<std::endl;
}

static void ReceiveData (Ptr<const Packet> p)
{
  std::cout<<" Receive Data at " << Simulator::Now ().GetSeconds()<<std::endl;
}
static void CwndTracer(uint32_t oldval, uint32_t newval)
{
  NS_LOG_INFO (Simulator::Now().GetSeconds()<<" "<<newval);
}
int main(int argc , char * argv[])
{
	  LogComponentEnable ("DosAttack", LOG_LEVEL_INFO);
	  std::string tcpType = "NewReno";
          std::string red = "RED";
	  std::string animFile = "dos.xml" ;  // Name of file for animation output
	  Config::SetDefault("ns3::TcpL4Protocol::SocketType",TypeIdValue(TypeId::LookupByName("ns3::Tcp" + tcpType)));
	  uint32_t    nLeftLeaf = 2;
	  uint32_t    nRightLeaf = 2;

          CommandLine cmd;
          cmd.AddValue ("tcpType", "tcpType", tcpType);
          cmd.AddValue ("animFile", "animFile", animFile);
          cmd.AddValue ("nLeftLeaf", "nLeftLeaf", nLeftLeaf);
          cmd.AddValue ("nRightLeaf", "nRightLeaf", nRightLeaf);
          cmd.AddValue ("red","red",red);
          cmd.Parse (argc, argv);

          if(red == "RED")
                Config::SetDefault("ns3::RedQueueDisc::ARED",BooleanValue(false));
          else if(red == "ARED")
                Config::SetDefault("ns3::RedQueueDisc::ARED",BooleanValue(true));

	  //Bottleneck Router link connecting left , right leaf Nodes
	  PointToPointHelper routers;
	  routers.SetDeviceAttribute  ("DataRate", StringValue ("1Mbps"));
	  routers.SetChannelAttribute ("Delay", StringValue ("2ms"));

	  // Leaf node on right , left side of the bottleneck routers
	  PointToPointHelper pointToPointLeaf;
	  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
	  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("1ms"));

	  PointToPointDumbbellHelper d (nLeftLeaf, pointToPointLeaf,
	                                nRightLeaf, pointToPointLeaf,
	                                routers);
	  InternetStackHelper stack;
	  d.InstallStack(stack);
	  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
              Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
              Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

	  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	  Address anyAddress,sinkAddress;
	  uint16_t sinkPort = 5000;
                
          TrafficControlHelper tchBottleneck;
          QueueDiscContainer queueDiscs;
          tchBottleneck.Uninstall(d.GetLeft ()->GetDevice (0));
          tchBottleneck.Uninstall (d.GetRight ()->GetDevice (0));
          tchBottleneck.SetRootQueueDisc ("ns3::RedQueueDisc");
          tchBottleneck.Install (d.GetLeft ()->GetDevice (0));
          queueDiscs = tchBottleneck.Install (d.GetRight ()->GetDevice (0));

	  anyAddress = InetSocketAddress (Ipv4Address::GetAny (), sinkPort);
	  sinkAddress = InetSocketAddress (d.GetRightIpv4Address(0), sinkPort);

	  std::cout<<"Sink IP Address--";
	  d.GetRightIpv4Address(0).Print(std::cout);
	  std::cout<<std::endl;

	  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", anyAddress);
	  ApplicationContainer sinkApps = packetSinkHelper.Install (d.GetRight(0));
	  sinkApps.Start (Seconds (1.));
	  sinkApps.Stop (Seconds (50.));

	  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (d.GetLeft(0), TcpSocketFactory::GetTypeId ());
	  std::cout<<"Source IP Address--";
	  d.GetLeftIpv4Address(0).Print(std::cout);
	  std::cout<<std::endl;

	  Ptr<LDosApp> app = CreateObject<LDosApp> ();
	  app->SetupModule (ns3TcpSocket, sinkAddress, 2000, 10000, DataRate ("50Mbps"));
	  d.GetLeft(0)->AddApplication(app);
	  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer));
	  ns3TcpSocket->TraceConnectWithoutContext ("SlowStartThreshold", MakeCallback (&CwndTracer));

	  app->SetStartTime (Seconds (2.));
	  app->SetStopTime (Seconds (30.));

	  UdpEchoServerHelper echoServer (9);
	  ApplicationContainer serverApps = echoServer.Install (d.GetRight(1));
	  serverApps.Start (Seconds (5.0));
	  serverApps.Stop (Seconds (20.0));

	  UdpEchoClientHelper echoClient (d.GetRightIpv4Address(1), 9);
	  echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
	  echoClient.SetAttribute ("Interval", TimeValue (Seconds (9)));// Packet arrival interval for low rate dos attack : Nikhil
	  echoClient.SetAttribute ("PacketSize", UintegerValue (2048));

	  ApplicationContainer clientApps = echoClient.Install (d.GetLeft(1));
	  clientApps.Start (Seconds (6.0));
	  clientApps.Stop (Seconds (20.0));

	  d.BoundingBox (1, 1, 100, 100);
	  // Create the animation object and configure for specified output
	  AnimationInterface anim (animFile);
	  anim.EnablePacketMetadata ();


	  Ptr<Object>traceObjClient=d.GetLeft(1)->GetDevice(0);
	  traceObjClient->TraceConnectWithoutContext("PhyTxBegin",MakeCallback(&SendData));
	  traceObjClient->TraceConnectWithoutContext("PhyTxEnd",MakeCallback(&SendData));
	  traceObjClient->TraceConnectWithoutContext("MacTx",MakeCallback(&SendData));


	  Ptr<Object> traceObjServer = d.GetRight(1)->GetDevice(0);
	  traceObjServer->TraceConnectWithoutContext("PhyRxBegin",MakeCallback(&ReceiveData));
	  traceObjServer->TraceConnectWithoutContext("PhyRxEnd",MakeCallback(&ReceiveData));
	  traceObjServer->TraceConnectWithoutContext("MacRx",MakeCallback(&ReceiveData));

	  FlowMonitorHelper flowmon;
	  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();


	  Simulator::Stop (Seconds (20));
	  Simulator::Run ();

	  monitor->CheckForLostPackets ();
	  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
	   {
	  	 Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	       if ((t.sourceAddress=="10.1.1.1" && t.destinationAddress == "10.2.1.1"))
	       {
	            std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
	            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
	            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
	        	std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
	        }
	   }
	  Simulator::Destroy ();

	  return 0;
}

