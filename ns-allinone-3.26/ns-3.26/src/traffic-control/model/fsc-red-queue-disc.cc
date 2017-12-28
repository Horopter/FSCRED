#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "fsc-red-queue-disc.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FscRedQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (FscRedQueueDisc);

FscRedQueueDisc::Stats m_fscstats;

struct FSCResult
{
	std::string name;
	double value;
};

bool is_equal(double a, double b)
{
	if((0 <= a-b && a-b < 0.001)||(0 <= b-a && b-a < 0.001))
		return true;
	return false;
}

double getMembershipforTrapezoid(double param, double low1, double peak1, double peak2, double low2)
{
	if(param < low1)
		return 0;
	else if (low1 <= param && param < peak1)
	{
		if(is_equal(peak1,low1))
			return 1;
		return (param - low1)/(peak1-low1);
	}
	else if(peak1 <= param && param <= peak2)
		return 1;
	else if(peak2 <= param && param <= low2)
	{
		if(is_equal(peak2,low2))
			return 1;
		return (param - peak2)/(low2 - peak2);
	}
	return 1;
}

double getCentroidforTrapezoid(double slope, double low1, double peak1, double peak2, double low2)
{
	return (slope*(peak1+peak2) + (low1+low2)*(2 - slope))/4;
}

std::vector<FSCResult> memberShipOfAQL(double aql)
{
	double conservative_low1 = 0;
	double conservative_peak1 = 0.05;
	double conservative_peak2 = 0.25;
	double conservative_low2 = 0.30;
	double middle_low1 = 0.20;
	double middle_peak1 = 0.33;
	double middle_peak2 = 0.66;
	double middle_low2 = 0.8;
	double aggressive_low1 = 0.70;
	double aggressive_peak1 = 0.9;
	double aggressive_peak2 = 1;
	double aggressive_low2 = 1;
	double conservative_score = getMembershipforTrapezoid(aql,conservative_low1,conservative_peak1,conservative_peak2,conservative_low2);
	double middle_score = getMembershipforTrapezoid(aql,middle_low1,middle_peak1,middle_peak2,middle_low2);
	double aggressive_score = getMembershipforTrapezoid(aql,aggressive_low1,aggressive_peak1,aggressive_peak2,aggressive_low2);
	std::vector<FSCResult> result;
	FSCResult fr;
	fr.name = "conservative";
	fr.value = conservative_score;
	result.push_back(fr);
	fr.name = "middle";
	fr.value = middle_score;
	result.push_back(fr);
	fr.name = "aggressive";
	fr.value = aggressive_score;
	result.push_back(fr);
	return result;
}

std::vector<FSCResult> membershipOfMaxP(double maxp)
{
	double few_low1 = 0;
	double few_peak1 = 0.0;
	double few_peak2 = 0.006;
	double few_low2 = 0.009;
	double medium_low1 = 0.004;
	double medium_peak1 = 0.02;
	double medium_peak2 = 0.06;
	double medium_low2 = 0.1;
	double alot_low1 = 0.091;
	double alot_peak1 = 0.1;
	double alot_peak2 = 1;
	double alot_low2 = 1;
	double few_score = getMembershipforTrapezoid(maxp,few_low1,few_peak1,few_peak2,few_low2);
	double medium_score = getMembershipforTrapezoid(maxp,medium_low1,medium_peak1,medium_peak2,medium_low2);
	double alot_score = getMembershipforTrapezoid(maxp,alot_low1,alot_peak1,alot_peak2,alot_low2);
	std::vector<FSCResult> result;
	FSCResult fr;
	fr.name = "few";
	fr.value = few_score;
	result.push_back(fr);
	fr.name = "medium";
	fr.value = medium_score;
	result.push_back(fr);
	fr.name = "alot";
	fr.value = alot_score;
	result.push_back(fr);
	return result;
}

double membershipOfDropP(std::vector<FSCResult> fr)
{
	double zero_low1 = 0;
	double zero_peak1 = 0.0;
	double zero_peak2 = 0.0;
	double zero_low2 = 0.001;
	double low_low1 = 0.0;
	double low_peak1 = 0.002;
	double low_peak2 = 0.008;
	double low_low2 = 0.01;
	double moderate_low1 = 0.0075;
	double moderate_peak1 = 0.05;
	double moderate_peak2 = 0.1;
	double moderate_low2 = 0.2;
	double high_low1 = 0.2;
	double high_peak1 = 0.3;
	double high_peak2 = 1;
	double high_low2 = 1;
	FSCResult zero,low,moderate,high;
	for(std::vector<FSCResult>::iterator it = fr.begin() ; it != fr.end(); ++it)
	{
		if(it->name == "zero")
		{
			zero.name = "zero";
			zero.value = it->value;
		}
		if(it->name == "low")
		{
			low.name = "low";
			low.value = it->value;
		}
		if(it->name == "moderate")
		{
			moderate.name = "moderate";
			moderate.value = it->value;
		}
		if(it->name == "high")
		{
			high.name = "high";
			high.value = it->value;
		}
		
	}
	double zero_centroid = getCentroidforTrapezoid(zero.value,zero_low1,zero_peak1,zero_peak2,zero_low2);
	double low_centroid = getCentroidforTrapezoid(low.value,low_low1,low_peak1,low_peak2,low_low2);
	double moderate_centroid = getCentroidforTrapezoid(moderate.value,moderate_low1,moderate_peak1,moderate_peak2,moderate_low2);
	double high_centroid = getCentroidforTrapezoid(high.value,high_low1,high_peak1,high_peak2,high_low2);
	
	int count = 0;
	if(zero_centroid > 0) count++;
	if(low_centroid > 0) count++;
	if(moderate_centroid > 0) count++;
	if(high_centroid > 0) count++;
	
	return (zero_centroid+low_centroid+moderate_centroid+high_centroid)/count;
}

FSCResult firb(FSCResult aql, FSCResult maxP)
{
	FSCResult fr;
	if((aql.name == "conservative") || (aql.name == "middle" && maxP.name != "alot") || (aql.name == "aggressive" && maxP.name == "few"))
		fr.name = "zero";
	else if (aql.name == "middle" && maxP.name == "alot")
		fr.name = "low";
	else if (aql.name == "aggressive" && maxP.name == "medium")
		fr.name = "moderate";
	else if (aql.name == "aggressive" && maxP.name == "alot")
		fr.name = "high";
	fr.value = std::min(aql.value,maxP.value);
	return fr;	
}

std::vector<FSCResult> avgClassifier(std::vector<FSCResult> fr)
{
	double zero=0, low=0, mod=0, high=0;
	double zc =0, lc =0, mc =0, hc =0;
	for(std::vector<FSCResult>::iterator it = fr.begin() ; it != fr.end(); ++it)
	{
		if(it->name == "zero")
		{
			zero += it->value;
			zc++;
		}
		if(it->name == "low")
		{
			low += it->value;
			lc++;
		}
		if(it->name == "moderate")
		{
			mod += it->value;
			mc++;
		}
		if(it->name == "high")
		{
			high += it->value;
			hc++;
		}
	}
	zero = zc == 0 ? zero : zero/zc;
	low = lc == 0 ? low : low/lc;
	mod = mc == 0 ? mod : mod/mc;
	high = hc == 0 ? high : high/hc;
	
	FSCResult zf,lf,mf,hf;
	zf.name = "zero"; zf.value = zero;
	lf.name = "low"; lf.value = low;
	mf.name = "moderate"; mf.value = mod;
	hf.name = "high"; hf.value = high;
	
	std::vector<FSCResult> f;
	f.push_back(zf);
	f.push_back(lf);
	f.push_back(mf);
	f.push_back(hf);

	return f;
}

double inputDefuzzifier(std::vector<FSCResult> aql, std::vector<FSCResult> maxP)
{
	std::vector<FSCResult> result;
	for (std::vector<FSCResult>::iterator it1 = aql.begin() ; it1 != aql.end(); ++it1)
		for (std::vector<FSCResult>::iterator it2 = maxP.begin() ; it2 != maxP.end(); ++it2)
		{
			FSCResult f = firb(*it1, *it2);
			if(f.value>0)
				result.push_back(f);
		}
	result = avgClassifier(result);
	return membershipOfDropP(result);	
}


TypeId FscRedQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FscRedQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName("TrafficControl")
    .AddConstructor<FscRedQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (Queue::QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&FscRedQueueDisc::SetMode),
                   MakeEnumChecker (Queue::QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    Queue::QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (500),
                   MakeUintegerAccessor (&FscRedQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("IdlePktSize",
                   "Average packet size used during idle times. Used when m_cautions = 3",
                   UintegerValue (0),
                   MakeUintegerAccessor (&FscRedQueueDisc::m_idlePktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Wait",
                   "True for waiting between dropped packets",
                   BooleanValue (true),
                   MakeBooleanAccessor (&FscRedQueueDisc::m_isWait),
                   MakeBooleanChecker ())
    .AddAttribute ("Gentle",
                   "True to increases dropping probability slowly when average queue exceeds maxthresh",
                   BooleanValue (true),
                   MakeBooleanAccessor (&FscRedQueueDisc::m_isGentle),
                   MakeBooleanChecker ())
    .AddAttribute ("ARED",
                   "True to enable ARED",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FscRedQueueDisc::m_isARED),
                   MakeBooleanChecker ())
    .AddAttribute ("AdaptMaxP",
                   "True to adapt m_curMaxP",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FscRedQueueDisc::m_isAdaptMaxP),
                   MakeBooleanChecker ())
    .AddAttribute ("FSCRED",
                   "True to enable FSC RED",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FscRedQueueDisc::m_isFSC),
                   MakeBooleanChecker ()) 
    .AddAttribute ("MinTh",
                   "Minimum average length threshold in packets/bytes",
                   DoubleValue (5),
                   MakeDoubleAccessor (&FscRedQueueDisc::m_minTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxTh",
                   "Maximum average length threshold in packets/bytes",
                   DoubleValue (15),
                   MakeDoubleAccessor (&FscRedQueueDisc::m_maxTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (25),
                   MakeUintegerAccessor (&FscRedQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QW",
                   "Queue weight related to the exponential weighted moving average (EWMA)",
                   DoubleValue (0.002),
                   MakeDoubleAccessor (&FscRedQueueDisc::m_qW),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("LInterm",
                   "The maximum probability of dropping a packet",
                   DoubleValue (50),
                   MakeDoubleAccessor (&FscRedQueueDisc::m_lInterm),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("TargetDelay",
                   "Target average queuing delay in ARED",
                   TimeValue (Seconds (0.005)),
                   MakeTimeAccessor (&FscRedQueueDisc::m_targetDelay),
                   MakeTimeChecker ())
    .AddAttribute ("Interval",
                   "Time interval to update m_curMaxP",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&FscRedQueueDisc::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("Top",
                   "Upper bound for m_curMaxP in ARED",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&FscRedQueueDisc::m_top),
                   MakeDoubleChecker <double> (0, 1))
    .AddAttribute ("Bottom",
                   "Lower bound for m_curMaxP in ARED",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&FscRedQueueDisc::m_bottom),
                   MakeDoubleChecker <double> (0, 1))
    .AddAttribute ("Alpha",
                   "Increment parameter for m_curMaxP in ARED",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&FscRedQueueDisc::SetAredAlpha),
                   MakeDoubleChecker <double> (0, 1))
    .AddAttribute ("Beta",
                   "Decrement parameter for m_curMaxP in ARED",
                   DoubleValue (0.9),
                   MakeDoubleAccessor (&FscRedQueueDisc::SetAredBeta),
                   MakeDoubleChecker <double> (0, 1))
    .AddAttribute ("FSCBeta",
                   "Increment parameter for m_curMaxP in FSC's Adaptive RED",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&FscRedQueueDisc::SetFSCB),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("FSCAlpha",
                   "Decrement parameter for m_curMaxP in FSC's Adaptive RED",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&FscRedQueueDisc::SetFSCA),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("LastSet",
                   "Store the last time m_curMaxP was updated",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&FscRedQueueDisc::m_lastSet),
                   MakeTimeChecker ())
    .AddAttribute ("Rtt",
                   "Round Trip Time to be considered while automatically setting m_bottom",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&FscRedQueueDisc::m_rtt),
                   MakeTimeChecker ())
    .AddAttribute ("Ns1Compat",
                   "NS-1 compatibility",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FscRedQueueDisc::m_isNs1Compat),
                   MakeBooleanChecker ())
    .AddAttribute ("LinkBandwidth", 
                   "The RED link bandwidth",
                   DataRateValue (DataRate ("1.5Mbps")),
                   MakeDataRateAccessor (&FscRedQueueDisc::m_linkBandwidth),
                   MakeDataRateChecker ())
    .AddAttribute ("LinkDelay", 
                   "The RED link delay",
                   TimeValue (MilliSeconds (20)),
                   MakeTimeAccessor (&FscRedQueueDisc::m_linkDelay),
                   MakeTimeChecker ())
  ;

  return tid;
}

FscRedQueueDisc::FscRedQueueDisc () :
  QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<WeibullRandomVariable> ();
}

FscRedQueueDisc::~FscRedQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
FscRedQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  QueueDisc::DoDispose ();
}

void
FscRedQueueDisc::SetMode (Queue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

Queue::QueueMode
FscRedQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
FscRedQueueDisc::SetAredAlpha (double alpha)
{
  NS_LOG_FUNCTION (this << alpha);
  m_alpha = alpha;

  if (m_alpha > 0.01)
    {
      NS_LOG_WARN ("Alpha value is above the recommended bound!");
    }
}

double
FscRedQueueDisc::GetAredAlpha (void)
{
  NS_LOG_FUNCTION (this);
  return m_alpha;
}

void
FscRedQueueDisc::SetAredBeta (double beta)
{
  NS_LOG_FUNCTION (this << beta);
  m_beta = beta;

  if (m_beta < 0.83)
    {
      NS_LOG_WARN ("Beta value is below the recommended bound!");
    }
}

double
FscRedQueueDisc::GetAredBeta (void)
{
  NS_LOG_FUNCTION (this);
  return m_beta;
}

double
FscRedQueueDisc::GetFSCA (void)
{
  NS_LOG_FUNCTION (this);
  return m_a;
}

void
FscRedQueueDisc::SetFSCA (double a)
{
  NS_LOG_FUNCTION (this << a);
  m_a = a;

  if (m_a != 3)
    {
      NS_LOG_WARN ("Alpha value does not follow the recommendations!");
    }
}

double
FscRedQueueDisc::GetFSCB (void)
{
  NS_LOG_FUNCTION (this);
  return m_b;
}

void
FscRedQueueDisc::SetFSCB (double b)
{
  NS_LOG_FUNCTION (this << b);
  m_b = b;

  if (m_b != 2)
    {
      NS_LOG_WARN ("Beta value does not follow the recommendations!");
    }
}

void
FscRedQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

void
FscRedQueueDisc::SetTh (double minTh, double maxTh)
{
  NS_LOG_FUNCTION (this << minTh << maxTh);
  NS_ASSERT (minTh <= maxTh);
  m_minTh = minTh;
  m_maxTh = maxTh;
  std::cout << "Minimum Threshold : "<< minTh << "Minimum Threshold : " << maxTh;
}

FscRedQueueDisc::Stats
FscRedQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_fscstats;
}

int64_t 
FscRedQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
FscRedQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  uint32_t nQueued = 0;

  if (GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      NS_LOG_DEBUG ("Enqueue in bytes mode");
      nQueued = GetInternalQueue (0)->GetNBytes ();
    }
  else if (GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      NS_LOG_DEBUG ("Enqueue in packets mode");
      nQueued = GetInternalQueue (0)->GetNPackets ();
    }

  // simulate number of packets arrival during idle period
  uint32_t m = 0;

  if (m_idle == 1)
    {
      NS_LOG_DEBUG ("RED Queue Disc is idle.");
      Time now = Simulator::Now ();

      if (m_cautious == 3)
        {
          double ptc = m_ptc * m_meanPktSize / m_idlePktSize;
          m = uint32_t (ptc * (now - m_idleTime).GetSeconds ());
        }
      else
        {
          m = uint32_t (m_ptc * (now - m_idleTime).GetSeconds ());
        }

      m_idle = 0;
    }

  qTimeFSCRED aql;
  aql.time = Simulator :: Now();
  aql.qlen = m_qAvg;
  aql.minTh = m_minTh;
  aql.maxTh = m_maxTh;
  m_fscstats.avgQLen.push_back(aql);

  m_qAvg = Estimator (nQueued, m + 1, m_qAvg, m_qW);

  aql.qlen = m_qAvg;
  m_fscstats.curQLen.push_back(aql);

  NS_LOG_DEBUG ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes () << "\tQavg " << m_qAvg);
  NS_LOG_DEBUG ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets () << "\tQavg " << m_qAvg);

  m_count++;
  m_countBytes += item->GetPacketSize ();

  uint32_t dropType = DTYPE_NONE;
  if (m_qAvg >= m_minTh && nQueued > 1)
    {
      if ((!m_isGentle && m_qAvg >= m_maxTh) ||
          (m_isGentle && m_qAvg >= 2 * m_maxTh))
        {
          NS_LOG_DEBUG ("adding DROP FORCED MARK");
          dropType = DTYPE_FORCED;
        }
      else if (m_old == 0)
        {
          /* 
           * The average queue size has just crossed the
           * threshold from below to above "minthresh", or
           * from above "minthresh" with an empty queue to
           * above "minthresh" with a nonempty queue.
           */
          m_count = 1;
          m_countBytes = item->GetPacketSize ();
          m_old = 1;
        }
      else if (DropEarly (item, nQueued))
        {
          NS_LOG_LOGIC ("DropEarly returns 1");
          dropType = DTYPE_UNFORCED;
        }
    }
  else 
    {
      // No packets are being dropped
      m_vProb = 0.0;
      m_old = 0;
    }

  if ((GetMode () == Queue::QUEUE_MODE_PACKETS && nQueued >= m_queueLimit) ||
      (GetMode () == Queue::QUEUE_MODE_BYTES && nQueued + item->GetPacketSize() > m_queueLimit))
    {
      NS_LOG_DEBUG ("\t Dropping due to Queue Full " << nQueued);
      dropType = DTYPE_FORCED;
      m_fscstats.qLimDrop++;
    }

  if (dropType == DTYPE_UNFORCED)
    {
      NS_LOG_DEBUG ("\t Dropping due to Prob Mark " << m_qAvg);
      m_fscstats.unforcedDrop++;
      Drop (item);
      return false;
    }
  else if (dropType == DTYPE_FORCED)
    {
      NS_LOG_DEBUG ("\t Dropping due to Hard Mark " << m_qAvg);
      m_fscstats.forcedDrop++;
      Drop (item);
      if (m_isNs1Compat)
        {
          m_count = 0;
          m_countBytes = 0;
        }
      return false;
    }

  GetInternalQueue (0)->Enqueue (item);

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return true;
}

/*
 * Note: if the link bandwidth changes in the course of the
 * simulation, the bandwidth-dependent RED parameters do not change.
 * This should be fixed, but it would require some extra parameters,
 * and didn't seem worth the trouble...
 */
void
FscRedQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Initializing RED params.");

  m_cautious = 0;
  m_ptc = m_linkBandwidth.GetBitRate () / (8.0 * m_meanPktSize);

  std::cout << "InitParams :: RedMinth : " << m_minTh << " RedMaxTh : " << m_maxTh << " m_isFSC :  "<< m_isFSC <<std::endl;

  if (m_isARED)
    {
      // Set m_minTh, m_maxTh and m_qW to zero for automatic setting
      m_minTh = 0;
      m_maxTh = 0;
      m_qW = 0;

      // Turn on m_isAdaptMaxP to adapt m_curMaxP
      m_isAdaptMaxP = true;
    }

  if (m_minTh == 0 && m_maxTh == 0)
    {
      m_minTh = 5.0;

      // set m_minTh to max(m_minTh, targetqueue/2.0) [Ref: http://www.icir.org/floyd/papers/adaptiveRed.pdf]
      double targetqueue = m_targetDelay.GetSeconds() * m_ptc;

      if (m_minTh < targetqueue / 2.0 )
        {
          m_minTh = targetqueue / 2.0;
        }
      if (GetMode () == Queue::QUEUE_MODE_BYTES)
        {
          m_minTh = m_minTh * m_meanPktSize;
        }

      // set m_maxTh to three times m_minTh [Ref: http://www.icir.org/floyd/papers/adaptiveRed.pdf]
      m_maxTh = 3 * m_minTh;
    }

  NS_ASSERT (m_minTh <= m_maxTh);
  m_fscstats.forcedDrop = 0;
  m_fscstats.unforcedDrop = 0;
  m_fscstats.qLimDrop = 0;

  m_qAvg = 0.0;
  m_count = 0;
  m_countBytes = 0;
  m_old = 0;
  m_idle = 1;

  double th_diff = (m_maxTh - m_minTh);
  if (th_diff == 0)
    {
      th_diff = 1.0; 
    }
  m_vA = 1.0 / th_diff;
  m_curMaxP = 1.0 / m_lInterm;
  m_vB = -m_minTh / th_diff;

  if (m_isGentle)
    {
      m_vC = (1.0 - m_curMaxP) / m_maxTh;
      m_vD = 2.0 * m_curMaxP - 1.0;
    }
  m_idleTime = NanoSeconds (0);

/*
 * If m_qW=0, set it to a reasonable value of 1-exp(-1/C)
 * This corresponds to choosing m_qW to be of that value for
 * which the packet time constant -1/ln(1-m)qW) per default RTT 
 * of 100ms is an order of magnitude more than the link capacity, C.
 *
 * If m_qW=-1, then the queue weight is set to be a function of
 * the bandwidth and the link propagation delay.  In particular, 
 * the default RTT is assumed to be three times the link delay and 
 * transmission delay, if this gives a default RTT greater than 100 ms. 
 *
 * If m_qW=-2, set it to a reasonable value of 1-exp(-10/C).
 */
  if (m_qW == 0.0)
    {
      m_qW = 1.0 - std::exp (-1.0 / m_ptc);
    }
  else if (m_qW == -1.0)
    {
      double rtt = 3.0 * (m_linkDelay.GetSeconds () + 1.0 / m_ptc);

      if (rtt < 0.1)
        {
          rtt = 0.1;
        }
      m_qW = 1.0 - std::exp (-1.0 / (10 * rtt * m_ptc));
    }
  else if (m_qW == -2.0)
    {
      m_qW = 1.0 - std::exp (-10.0 / m_ptc);
    }

  if (m_bottom == 0)
    {
      m_bottom = 0.01;
      // Set bottom to at most 1/W, where W is the delay-bandwidth
      // product in packets for a connection.
      // So W = m_linkBandwidth.GetBitRate () / (8.0 * m_meanPktSize * m_rtt.GetSeconds())
      double bottom1 = (8.0 * m_meanPktSize * m_rtt.GetSeconds()) / m_linkBandwidth.GetBitRate();
      if (bottom1 < m_bottom)
        {
          m_bottom = bottom1;
        }
    }

  NS_LOG_DEBUG ("\tm_delay " << m_linkDelay.GetSeconds () << "; m_isWait " 
                             << m_isWait << "; m_qW " << m_qW << "; m_ptc " << m_ptc
                             << "; m_minTh " << m_minTh << "; m_maxTh " << m_maxTh
                             << "; m_isGentle " << m_isGentle << "; th_diff " << th_diff
                             << "; lInterm " << m_lInterm << "; va " << m_vA <<  "; cur_max_p "
                             << m_curMaxP << "; v_b " << m_vB <<  "; m_vC "
                             << m_vC << "; m_vD " <<  m_vD);
}

// Updating m_curMaxP, following the pseudocode 
// from: A Self-Configuring RED Gateway, INFOCOMM '99.
// They recommend m_a = 3, and m_b = 2.
void
FscRedQueueDisc::UpdateMaxPFSC (double newAve,Time now)
{
  NS_LOG_FUNCTION (this << newAve);
   double m_part = 0.4 * (m_maxTh - m_minTh);
   double m_target = m_minTh + m_part;
   qTimeFSCRED aql;
   aql.time = Simulator :: Now();
   aql.qlen = m_target;
   aql.minTh = m_minTh;
   aql.maxTh = m_maxTh;
   m_fscstats.avgQLen.push_back(aql);
   aql.qlen = newAve;
   m_fscstats.curQLen.push_back(aql);

  if (m_minTh < newAve && newAve < m_maxTh)
    {
      m_status = Between;
      if(m_minTh <= newAve && newAve < m_target)
        	{
        		m_curMaxP = 4*m_curMaxP*m_curMaxP*m_curMaxP;
        		m_lastSet = now;
        	}
      else if(m_target <= newAve && newAve < m_maxTh)
        	{
        		m_curMaxP = 2*m_curMaxP;
        		m_lastSet = now;
        	}
    } 
  else if (newAve < m_minTh && m_status != Below)
    {
      m_status = Below;
      m_curMaxP = m_curMaxP / m_a; 
    }
  else if (newAve > m_maxTh && m_status != Above)
    {
      m_status = Above;
      if(m_maxTh < newAve && newAve < 2*m_maxTh)
      	{
      		m_curMaxP = 4*m_curMaxP*m_curMaxP*m_curMaxP;
      	}
      else
      		m_curMaxP = m_curMaxP * m_b;
      m_lastSet = now;
    }
}

// Update m_curMaxP to keep the average queue length within the target range.
void
FscRedQueueDisc::UpdateMaxP (double newAve, Time now)
{
  double m_part = 0.4 * (m_maxTh - m_minTh);
  // AIMD rule to keep target Q~1/2(m_minTh + m_maxTh)
  if (newAve < m_minTh + m_part && m_curMaxP > m_bottom)
    {
      // we should increase the average queue size, so decrease m_curMaxP
      m_curMaxP = m_curMaxP * m_beta;
      m_lastSet = now;
    }
  else if (newAve > m_maxTh - m_part && m_top > m_curMaxP)
    {
      // we should decrease the average queue size, so increase m_curMaxP
      double alpha = m_alpha;
      if (alpha > 0.25 * m_curMaxP)
        {
          alpha = 0.25 * m_curMaxP;
        }
      m_curMaxP = m_curMaxP + alpha;
      m_lastSet = now;
    }
}

// Compute the average queue size
double
FscRedQueueDisc::Estimator (uint32_t nQueued, uint32_t m, double qAvg, double qW)
{
  NS_LOG_FUNCTION (this << nQueued << m << qAvg << qW);

  double newAve = qAvg * pow(1.0-qW, m);
  newAve += qW * nQueued;

  Time now = Simulator::Now();
 
  if (m_isAdaptMaxP && now > m_lastSet + m_interval)
    {
      UpdateMaxP (newAve, now);
    }
  else if (m_isFSC)
    {
      UpdateMaxPFSC (newAve,now);
    }

  return newAve;
}

// Check if packet p needs to be dropped due to probability mark
uint32_t
FscRedQueueDisc::DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize)
{
  NS_LOG_FUNCTION (this << item << qSize);
  m_vProb1 = CalculatePNew (m_qAvg, m_maxTh, m_isGentle, m_vA, m_vB, m_vC, m_vD, m_curMaxP);
  m_vProb = ModifyP (m_vProb1, m_count, m_countBytes, m_meanPktSize, m_isWait, item->GetPacketSize ());

  // Drop probability is computed, pick random number and act
  if (m_cautious == 1)
    {
      /*
       * Don't drop/mark if the instantaneous queue is much below the average.
       * For experimental purposes only.
       * pkts: the number of packets arriving in 50 ms
       */
      double pkts = m_ptc * 0.05;
      double fraction = std::pow ((1 - m_qW), pkts);

      if ((double) qSize < fraction * m_qAvg)
        {
          // Queue could have been empty for 0.05 seconds
          return 0;
        }
    }

  double u = m_uv->GetValue ();

  if (m_cautious == 2)
    {
      /*
       * Decrease the drop probability if the instantaneous
       * queue is much below the average.
       * For experimental purposes only.
       * pkts: the number of packets arriving in 50 ms
       */
      double pkts = m_ptc * 0.05;
      double fraction = std::pow ((1 - m_qW), pkts);
      double ratio = qSize / (fraction * m_qAvg);

      if (ratio < 1.0)
        {
          u *= 1.0 / ratio;
        }
    }

  if (u <= m_vProb)
    {
      NS_LOG_LOGIC ("u <= m_vProb; u " << u << "; m_vProb " << m_vProb);

      // DROP or MARK
      m_count = 0;
      m_countBytes = 0;
      /// \todo Implement set bit to mark

      return 1; // drop
    }

  return 0; // no drop/mark
}

// Returns a probability using these function parameters for the DropEarly funtion
double
FscRedQueueDisc::CalculatePNew (double qAvg, double maxTh, bool isGentle, double vA,
                         double vB, double vC, double vD, double maxP)
{
  NS_LOG_FUNCTION (this << qAvg << maxTh << isGentle << vA << vB << vC << vD << maxP);
  double p;
/*
  if (isGentle && qAvg >= maxTh)
    {
      // p ranges from maxP to 1 as the average queue
      // Size ranges from maxTh to twice maxTh
      p = vC * qAvg + vD;
    }
  else if (!isGentle && qAvg >= maxTh)
    {
      / * 
       * OLD: p continues to range linearly above max_p as
       * the average queue size ranges above th_max.
       * NEW: p is set to 1.0
       * /
      p = 1.0;
    }
  else
    {
      / *
       * p ranges from 0 to max_p as the average queue size ranges from
       * th_min to th_max
       * /
      p = vA * qAvg + vB;
      p *= maxP;
    }
*/

	std::vector<FSCResult> qAvgFuzzy = memberShipOfAQL(qAvg);
	std::vector<FSCResult> maxPFuzzy = membershipOfMaxP(maxP);
	double probabilityDrop = inputDefuzzifier(qAvgFuzzy,maxPFuzzy);
	p = probabilityDrop;


  if (p > 1.0)
    {
      p = 1.0;
    }

  return p;
}

// Returns a probability using these function parameters for the DropEarly funtion
double 
FscRedQueueDisc::ModifyP (double p, uint32_t count, uint32_t countBytes,
                   uint32_t meanPktSize, bool isWait, uint32_t size)
{
  NS_LOG_FUNCTION (this << p << count << countBytes << meanPktSize << isWait << size);
  double count1 = (double) count;

  if (GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      count1 = (double) (countBytes / meanPktSize);
    }

  if (isWait)
    {
      if (count1 * p < 1.0)
        {
          p = 0.0;
        }
      else if (count1 * p < 2.0)
        {
          p /= (2.0 - count1 * p);
        }
      else
        {
          p = 1.0;
        }
    }
  else
    {
      if (count1 * p < 1.0)
        {
          p /= (1.0 - count1 * p);
        }
      else
        {
          p = 1.0;
        }
    }

  if ((GetMode () == Queue::QUEUE_MODE_BYTES) && (p < 1.0))
    {
      p = (p * size) / meanPktSize;
    }

  if (p > 1.0)
    {
      p = 1.0;
    }

  return p;
}

uint32_t
FscRedQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      return GetInternalQueue (0)->GetNBytes ();
    }
  else if (GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      return GetInternalQueue (0)->GetNPackets ();
    }
  else
    {
      NS_ABORT_MSG ("Unknown RED mode.");
    }
}

Ptr<QueueDiscItem>
FscRedQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      m_idle = 1;
      m_idleTime = Simulator::Now ();

      return 0;
    }
  else
    {
      m_idle = 0;
      Ptr<QueueDiscItem> item = StaticCast<QueueDiscItem> (GetInternalQueue (0)->Dequeue ());

      NS_LOG_LOGIC ("Popped " << item);

      NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
      NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

      return item;
    }
}

Ptr<const QueueDiscItem>
FscRedQueueDisc::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);
  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<const QueueDiscItem> item = StaticCast<const QueueDiscItem> (GetInternalQueue (0)->Peek ());

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return item;
}

bool
FscRedQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("FscRedQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("FscRedQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // create a DropTail queue
      Ptr<Queue> queue = CreateObjectWithAttributes<DropTailQueue> ("Mode", EnumValue (m_mode));
      if (m_mode == Queue::QUEUE_MODE_PACKETS)
        {
          queue->SetMaxPackets (m_queueLimit);
        }
      else
        {
          queue->SetMaxBytes (m_queueLimit);
        }
      AddInternalQueue (queue);
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("FscRedQueueDisc needs 1 internal queue");
      return false;
    }

  if (GetInternalQueue (0)->GetMode () != m_mode)
    {
      NS_LOG_ERROR ("The mode of the provided queue does not match the mode set on the FscRedQueueDisc");
      return false;
    }

  if ((m_mode ==  Queue::QUEUE_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () < m_queueLimit) ||
      (m_mode ==  Queue::QUEUE_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal queue is less than the queue disc limit");
      return false;
    }

  return true;
}

} // namespace ns3
