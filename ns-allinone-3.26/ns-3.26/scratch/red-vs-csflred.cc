#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"

#include <iostream>
#include <iomanip>
#include <map>

using namespace ns3;

std::string GetRandomVariableString(int scale, int shape)
{
        std::stringstream sstm;
        sstm << "ns3::WeibullRandomVariable[Scale="<<scale<<".|Shape="<<shape<<".]";
        return sstm.str();
}

std::string DoubleToString(double d)
{
        std::ostringstream strs;
        strs << d;
        return strs.str();
}

int main (int argc, char *argv[])
{
  uint32_t    nLeaf = 10;
  uint32_t    maxPackets = 100;
  bool        modeBytes  = false;
  uint32_t    queueDiscLimitPackets = 1000;
  double      minTh = 5;
  double      maxTh = 15;
  uint32_t    pktSize = 512;
  std::string appDataRate = "10Mbps";
  std::string queueDiscType = "RED";
  uint16_t port = 5001;
  std::string bottleNeckLinkBw = "1Mbps";
  std::string bottleNeckLinkDelay = "50ms";
  int scale = 1;
  int shape = 2;

  CommandLine cmd;
  cmd.AddValue ("nLeaf",     "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue ("maxPackets","Max Packets allowed in the device queue", maxPackets);
  cmd.AddValue ("queueDiscLimitPackets","Max Packets allowed in the queue disc", queueDiscLimitPackets);
  cmd.AddValue ("queueDiscType", "Set Queue disc type to FRED or ARED", queueDiscType);
  cmd.AddValue ("appPktSize", "Set OnOff App Packet Size", pktSize);
  cmd.AddValue ("appDataRate", "Set OnOff App DataRate", appDataRate);
  cmd.AddValue ("modeBytes", "Set Queue disc mode to Packets <false> or bytes <true>", modeBytes);
  cmd.AddValue ("redMinTh", "RED queue minimum threshold", minTh);
  cmd.AddValue ("redMaxTh", "RED queue maximum threshold", maxTh);
  cmd.AddValue ("shape", "Shape Parameter for Weibull distribution",shape);
  cmd.Parse (argc,argv);

  if ((queueDiscType != "RED") && (queueDiscType != "CSFLRED"))
    {
      std::cout << "Invalid queue disc type: Use --queueDiscType=RED or --queueDiscType=CSFLRED" << std::endl;
      exit (1);
    }

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (pktSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (appDataRate));

  Config::SetDefault ("ns3::Queue::Mode", StringValue ("QUEUE_MODE_PACKETS"));
  Config::SetDefault ("ns3::Queue::MaxPackets", UintegerValue (maxPackets));

  if (!modeBytes)
    {
      Config::SetDefault ("ns3::CsflRedQueueDisc::Mode", StringValue ("QUEUE_MODE_PACKETS"));
      Config::SetDefault ("ns3::CsflRedQueueDisc::QueueLimit", UintegerValue (queueDiscLimitPackets));
    }
  else
    {
      Config::SetDefault ("ns3::CsflRedQueueDisc::Mode", StringValue ("QUEUE_MODE_BYTES"));
      Config::SetDefault ("ns3::CsflRedQueueDisc::QueueLimit", UintegerValue (queueDiscLimitPackets * pktSize));
      minTh *= pktSize;
      maxTh *= pktSize;
    }

  Config::SetDefault ("ns3::CsflRedQueueDisc::MinTh", DoubleValue (minTh));
  Config::SetDefault ("ns3::CsflRedQueueDisc::MaxTh", DoubleValue (maxTh));
  Config::SetDefault ("ns3::CsflRedQueueDisc::LinkBandwidth", StringValue (bottleNeckLinkBw));
  Config::SetDefault ("ns3::CsflRedQueueDisc::LinkDelay", StringValue (bottleNeckLinkDelay));
  Config::SetDefault ("ns3::CsflRedQueueDisc::MeanPktSize", UintegerValue (pktSize));

  if (queueDiscType == "CSFLRED")
    {
      // Turn on CSFLRED
      Config::SetDefault ("ns3::CsflRedQueueDisc::CSFLRED", BooleanValue (true));
    }

  // Create the point-to-point link helpers
  PointToPointHelper bottleNeckLink;
  bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckLinkBw));
  bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckLinkDelay));

  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("1ms"));

  PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,
                                nLeaf, pointToPointLeaf,
                                bottleNeckLink);

  // Install Stack
  InternetStackHelper stack;
  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      stack.Install (d.GetLeft (i));
    }
  for (uint32_t i = 0; i < d.RightCount (); ++i)
    {
      stack.Install (d.GetRight (i));
    }

  stack.Install (d.GetLeft ());
  stack.Install (d.GetRight ());
  TrafficControlHelper tchBottleneck;
  QueueDiscContainer queueDiscs;
  tchBottleneck.SetRootQueueDisc ("ns3::CsflRedQueueDisc");
  tchBottleneck.Install (d.GetLeft ()->GetDevice (0));
  queueDiscs = tchBottleneck.Install (d.GetRight ()->GetDevice (0));

  // Assign IP Addresses
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

  // Install on/off app on all right side nodes
  OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue (GetRandomVariableString(scale,shape)));
  clientHelper.SetAttribute ("OffTime", StringValue (GetRandomVariableString(scale,shape)));
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApps;
  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      sinkApps.Add (packetSinkHelper.Install (d.GetLeft (i)));
    }
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (30.0));

  ApplicationContainer clientApps;
  for (uint32_t i = 0; i < d.RightCount (); ++i)
    {
      // Create an on/off app sending packets to the left side
      AddressValue remoteAddress (InetSocketAddress (d.GetLeftIpv4Address (i), port));
      clientHelper.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (clientHelper.Install (d.GetRight (i)));
    }
  clientApps.Start (Seconds (1.0)); // Start 1 second after sink
  clientApps.Stop (Seconds (15.0)); // Stop before the sink

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  std::cout << "Running the simulation" << std::endl;
  Simulator::Run ();

  CsflRedQueueDisc::Stats st = StaticCast<CsflRedQueueDisc> (queueDiscs.Get (0))->GetStats ();

  std::cout << "*** Stats from the bottleneck queue disc *** "+queueDiscType << std::endl;
  std::cout << "\t " << st.unforcedDrop << " drops due to prob mark" << std::endl;
  std::cout << "\t " << st.forcedDrop << " drops due to hard mark" << std::endl;
  std::cout << "\t " << st.qLimDrop << " drops due to queue full" << std::endl;
  std::ofstream output_file1("./results/Avg-Q-Len-v-Time-D_"+queueDiscType+"_Min_"+DoubleToString(minTh)+"_K_"+DoubleToString(shape)+".csv");
  std::ostream_iterator<qTimeCSFLRED> output_iterator1(output_file1, "\n");
  std::copy(st.avgQLen.begin(), st.avgQLen.end(), output_iterator1);
  std::ofstream output_file2("./results/Cur-Q-Len-v-Time-D_"+queueDiscType+"_Min_"+DoubleToString(minTh)+"_K_"+DoubleToString(shape)+".csv");
  std::ostream_iterator<qTimeCSFLRED> output_iterator2(output_file2, "\n");
  std::copy(st.curQLen.begin(), st.curQLen.end(), output_iterator2);
  std::cout << "Destroying the simulation" << std::endl;
  Simulator::Destroy ();
  return 0;
}
