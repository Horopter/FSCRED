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

// At 7.5 dBm, nodes should be about 80 m apart to ensure that one node's
// transmissions can only reach his immediate neighbors. This was determined
// empircally by putting nodes in a straight line, and looking at the packet
// captures.

#define WIFI_TX_POWER 7.5
#define UDP_PORT 9
#define TOTAL_TIME 200.0
#define UDP_SEND_START_TIME 15.0
#define UDP_PACKET_SIZE 256
#define UDP_PACKET_INTERVAL 0.1

using namespace ns3;


std::vector<Node> SelectRandomNode(NodeContainer nodes, int k=1)
{
    """Select 'k' random nodes from the NodeContainer 'nodes'"""
    std::vector<Node> nodelist;
    for(int i=0;i<k;i++)
	nodelist.push_back(nodes.Get(rand()%nodes.GetN()))
    return nodelist;
}


class ManetSimulator
{
	private:
	int num_nodes;
	int node_spacing;
	int node_placement;
	int protocol;
	int bytesTotal,bytesLast,packetsTotal,packetsLast;
	std::fstream csvfile;
	NodeContainer nodes;
	Ipv4InterfaceContainer ifaces;
	Ipv4AddressHelper addrs;
	
	
	public:
	ManetSimulator(int num_nodes, std::string node_spacing, double node_placement, std::string protocol)
	{
		this.setup(num_nodes, node_spacing, node_placement, protocol);
		this.bytesTotal = 0;
		this.bytesLast=0;
		this.packetsTotal = 0;
		this.packetsLast = 0;
		this.csvfile.open ("test.txt", std::fstream::in | std::fstream::out | std::fstream::app);
		this.csvfile << "time,bytes,bytes_per_sec,packets,packets_per_sec";
		this.csvfile.close();
	}
	ManetSimulator enter()
	{
		return this;
	}
	void exit()
	{
		this.close();
	}
	void close()
	{
		if(this.csvfile)
			this.csvfile.close();
	}


	void setup(int num_nodes, std::string node_spacing, double node_placement, std::string protocol)
	{
		// Create a container with the desired number of nodes
		nodes.Create(num_Nodes);

		// Set up Wifi devices
		NetDeviceContainer adhocDevices = this.setup_wifi();

		// Set up mobility
		setup_mobility(num_nodes, node_spacing, node_placement);

		// Set up routing
		setup_routing(protocol);

		// Assign IP addresses
		addrs.SetBase("10.1.1.0", "255.255.255.0");
		addrs.Assign(adhocDevices);
	
		ifaces = addrs.Assign(adhocDevices);

		// Randomly choose origin node (O) and destination node (D)
		//this.origin, this.destination = SelectRandomNode(this.nodes, 2)
		NodeContainer origin = nodes.Get(0);
		NodeContainer destination = nodes.Get(nodes.GetN() - 1);

		// Set up the sink node
		NodeContainer node = destination;
		Address address (InetSocketAddress (ifaces.GetAddress(node.GetId()), UDP_PORT));
		setup_packet_receive(address, node);

		// Source node
		UdpClientHelper client (address.GetIpv4(), server_sockaddr.GetPort());
		client.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
		client.SetAttribute("Interval", TimeValue(Seconds(UDP_PACKET_INTERVAL)));
		client.SetAttribute("PacketSize", UintegerValue(UDP_PACKET_SIZE));
		ApplicationContainer app = client.Install(NodeContainer(origin));
		app.Start(Seconds(0));
		app.Stop(Seconds(TOTAL_TIME));
		setup_flowmon();
	}

    NetDeviceContainer setup_wifi()
	{
		std::string phyMode = "DsssRate11Mbps";
		WifiHelper::SetStandard(WIFI_PHY_STANDARD_80211b);
		WifiHelper::SetRemoteStationManager("ns3::ConstantRateWifiManager",
		                             "DataMode", phyMode,
		                             "ControlMode", phyMode,
		                             "NonUnicastMode", phyMode);

		wifiPhy = setup_wifi_phy();
		wifiMac = setup_wifi_mac();
		NetDeviceContainer devices = WifiHelper::Install(wifiPhy, wifiMac, this.nodes);

		# Enable tracing
		wifiPhy.EnablePcapAll("manet", promiscuous=True);
		return devices;
	}


    YansWifiPhyHelper setup_wifi_phy(this){
        YansWifiChannelHelper chanhlp = new YansWifiChannelHelper();
        chanhlp.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel")
        chanhlp.AddPropagationLoss("ns3::FriisPropagationLossModel")

        YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
        phy.SetChannel(chanhlp.Create());

        double txp = DoubleValue(WIFI_TX_POWER);
        phy.Set("TxPowerStart", txp)
        phy.Set("TxPowerEnd", txp)

        return phy;
	}

    	NqosWifiMacHelper setup_wifi_mac(){
		//"""Set up non-QoS MAC"""
		NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
		mac.SetType("ns3::AdhocWifiMac");
		return mac;
	}

	void setup_mobility(int num_nodes, std::string node_spacing,double node_placement)
	{
		int grid_width;
		if (node_placement == "straight-line")
		    grid_width = 1;
		else if (node_placement == "grid")
		    grid_width = int(round(math.sqrt(num_nodes)));

		// Set up the grid
		// Objects are layed out starting from (-100, -100)
		MobilityHelper::SetPositionAllocator(
		        "ns3::GridPositionAllocator",
		        "MinX", DoubleValue(0),
		        "MinY", DoubleValue(0),
		        "DeltaX", DoubleValue(node_spacing),
		        "DeltaY", DoubleValue(node_spacing),
		        "GridWidth", UintegerValue(grid_width),
		        "LayoutType", StringValue("RowFirst"),
		        );

		// Objects will be in a fixed position throughout the simulation
		MobilityHelper::SetMobilityModel("ns3::ConstantPositionMobilityModel");
		MobilityHelper::Install(nodes);

		// Enable tracing
		MobilityHelper::EnableAsciiAll(AsciiTraceHelper::CreateFileStream("trace.mob"));
	}

    	void setup_routing(protocol_name){
		protocol = protocol_map[protocol_name];
		Ipv4ListRoutingHelper route_list = new Ipv4ListRoutingHelper();
		route_list.Add(protocol, 100);
		InternetStackHelper::SetRoutingHelper(route_list);
		InternetStackHelper::Install(this.nodes);
	}

    	void setup_packet_receive(sockaddr, node){
		TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
		Ptr<Socket> sink = Socket::CreateSocket(node, tid);
		sink->Bind(sockaddr);
		sink->SetRecvCallback(MakeCallback(&packet_rx_callback));
	}

    	void packet_rx_callback(ns3::Ptr<ns3::Socket> socket){
		while(1)
		{
		    Ptr<Packet> packet = socket->Recv();
		    if (packet == NULL)
			return;
		    this.bytesTotal    += packet->GetSize();
		    this.bytesLast     += packet->GetSize();
		    this.packetsTotal  += 1;
		    this.packetsLast   += 1;
		}
	}

    	void setup_flowmon(){
		// Set up FlowMonitor
		FlowMonitorHelper::InstallAll();
	}

    	void check_throughput(this){
		double interval = 1.0;
		this.csvfile.open ("test.txt", std::fstream::in | std::fstream::out | std::fstream::app);
		this.csvfile << Simulator::Now().GetSeconds() << "," <<
		    		bytesTotal<< "," <<
		    		bytesLast / interval << "," <<
		    		packetsTotal << "," <<
		    		packetsLast / interval<< std::endl;
		bytesLast     = 0;
		packetsLast   = 0;
		Simulator::Schedule(Seconds(interval),&check_throughput);
	}

    	void process_flowmon(std::string xml_filename){
		ns3::FlowMonitorHelper::GetMonitor().CheckForLostPackets().SerializeToXmlFile(xml_filename, true, true);
		return next(find_flow(server_sockaddr.GetIpv4(),
                                    server_sockaddr.GetPort()));//ToDo:figure out next
	}

	bool match(std::string a, std::string b)
	{
		if(a == "")
			return true;
		if(b != "" && a==b)
			return true;
		return false;
	}
	//ToDo:figure out
    	void find_flow(std::string srcAddr="",std::string srcPort="",std::string dstAddr="",std::string dstPort="")
	{
		flowmon = ns3::FlowMonitorHelper::GetMonitor();
		classifier = ns3::FlowMonitorHelper::GetMonitor().GetClassifier();
		//ToDo: yield
		for flow_id, flow_stats in ns3::FlowMonitorHelper::GetMonitor().GetFlowStats():
		    flow = ns3::FlowMonitorHelper::GetMonitor().GetClassifier().FindFlow(flow_id)

		    if (match(srcAddr, flow.sourceAddress) &&
		        match(srcPort, flow.sourcePort) &&
		        match(dstAddr, flow.destinationAddress) &&
		        match(dstPort, flow.destinationPort))
		        yield Flow(flow_id, flow, flow_stats)
	}

}


class Flow{
    private:
	int flowid;
	int flow;
	int stats;
    public:
    Flow(int flowid,int flow, int stats){
        this.id = flowid;
        this.flow = flow;
        this.stats = stats;
	}
    //ToDo: figure out
    void print(this){
        proto = {6: 'TCP', 17: 'UDP'}[this.flow.protocol]
        return "FlowID: {}  ({} {}/{} --> {}/{})".format(
            this.id, proto,
            this.flow.sourceAddress, this.flow.sourcePort,
            this.flow.destinationAddress, this.flow.destinationPort)
	}
	//ToDo: figure out
    void print_stats(this){
        print(this);
        int st = this.stats;
        print("  First Tx Time:     {} ms".format(st.timeFirstTxPacket.GetSeconds() * 1000))
        print("  First Rx Time:     {} ms".format(st.timeFirstRxPacket.GetSeconds() * 1000))
        print("  Tx Bytes:          {}".format(st.txBytes))
        print("  Rx Bytes:          {}".format(st.rxBytes))
        print("  Tx Packets:        {}".format(st.txPackets))
        print("  Rx Packets:        {}".format(st.rxPackets))
        print("  Lost Packets:      {}".format(st.lostPackets))
        if (st.rxPackets > 0){
            print("  Mean Delay:        {}".format(st.delaySum.GetSeconds() / st.rxPackets))
            print("  Mean Jitter:       {}".format(st.jitterSum.GetSeconds() / (st.rxPackets - 1)))
            print("  Mean Hop Count:    {}".format(float(st.timesForwarded) / (st.rxPackets + 1)))
	}}



double squared(double x){
        return x*x;
}

////ToDo: Declare triple and sqrt
void Distance3D(struct triple v1,struct triple v2)
{
	return math.sqrt(
        squared(v2.x - v1.x) +
        squared(v2.y - v1.y) +
        squared(v2.z - v1.z));
}

void GetPosition(node)
{
    MobilityModel mob = node.GetObject(MobilityModel::GetTypeId());
    return mob.GetPosition();
}

void FormatNode(node)
{
    Ipv4 ip4 = node.GetObject(Ipv4::GetTypeId());
    Ipv4Address ipaddr = ip4.GetAddress(1,0).GetLocal();
    Address macaddr = node.GetDevice(0).GetAddress();
    return '{:<3} {:<12} {} {}'.format(
            node.GetId(), ipaddr, macaddr, GetPosition(node))//ToDo: Use C++ format
}


void ShowAllNodes(nodes)
{
    for(int i =0; i<nodes.GetN();i++ )
	print(FormatNode(nodes.Get(i)));
}


AodvHelper SetupAodv(enable_hello)
{
    return AodvHelper::Set("EnableHello", BooleanValue(enable_hello));
}

//ToDo : dictfunction in C++?
protocol_map = {
    'AODV':     lambda: SetupAodv(True),
    'AODV-NH':  lambda: SetupAodv(False),
    'OLSR':     ns.olsr.OlsrHelper,
    'DSDV':     ns.dsdv.DsdvHelper,
}

int main(int argc, char* argv[])
{
	int num_nodes = 20;
	std::string placement = "grid";
	double spacing = 100.0;
	std::string protocol = "OLSR";

	CommandLine cmd;
	cmd.AddValue("num_nodes", "Number of nodes", num_nodes);
	cmd.AddValue("placement", "Controls the placement of nodes", placement);
	cmd.AddValue("spacing", "Controls the spacing of nodes (in meters)", spacing);
	cmd.AddValue("protocol","Routing protocol",protocol);
	cmd.Parse(argc, argv);


    ManetSimulator sim = new ManetSimulator(
            num_nodes = args.num_nodes,
            node_spacing = args.spacing,
            node_placement = args.placement,
            protocol = args.protocol,
            );

    ShowAllNodes(sim.nodes);
	//ToDo: Figure out format in C++
	print("Origin node:      {}".format(FormatNode(sim.origin)));
	print("Destination node: {}".format(FormatNode(sim.destination)));
	print("Distance:         {}".format(
		Distance3D(GetPosition(sim.origin), GetPosition(sim.destination)))
		);


    sim.check_throughput();

    // Run simulation
    Simulator::Stop(Seconds(TOTAL_TIME));
    Simulator::Run();

    flow = sim.process_flowmon("flowmon.xml");
    flow.print_stats();

    Simulator::Destroy();
	
	return 0;
}
