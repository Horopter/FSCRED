#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "fuzzy-red-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include <vector> 

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FuzzyRedQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (FuzzyRedQueueDisc);

FuzzyRedQueueDisc::Stats m_fstats;


struct FuzzyResult
{
	std::string name;
	double value;
};

bool DoubleEquals(double a, double b)
{
	if((0 <= a-b && a-b < 0.001)||(0 <= b-a && b-a < 0.001))
		return true;
	return false;
}

bool isBetween(double alpha, double min, double max)
{
	if(min <= alpha && alpha <= max)
		return true;
	return false;
}

double GetMembershipForTrapezoid(double param, double low1, double peak1, double peak2, double low2)
{
	if(param < low1)
		return 0;
	else if (low1 <= param && param < peak1)
	{
		if(DoubleEquals(peak1,low1))
			return 1;
		return (param - low1)/(peak1-low1);
	}
	else if(peak1 <= param && param <= peak2)
		return 1;
	else if(peak2 <= param && param <= low2)
	{
		if(DoubleEquals(peak2,low2))
			return 1;
		return (param - peak2)/(low2 - peak2);
	}
	return 1;
}

double GetCentroidForTrapezoid(double slope, double low1, double peak1, double peak2, double low2)
{
	return (slope*(peak1+peak2) + (low1+low2)*(2 - slope))/4;
}

std::vector<FuzzyResult> MembershipAQL(double aql)
{
	double conservative_low1 = 0;
	double conservative_peak1 = 0.05;
	double conservative_peak2 = 0.20;
	double conservative_low2 = 0.25;
	double middle_low1 = 0.20;
	double middle_peak1 = 0.33;
	double middle_peak2 = 0.50;
	double middle_low2 = 0.75;
	double aggressive_low1 = 0.70;
	double aggressive_peak1 = 0.75;
	double aggressive_peak2 = 1;
	double aggressive_low2 = 1;
	double conservative_score = GetMembershipForTrapezoid(aql,conservative_low1,conservative_peak1,conservative_peak2,conservative_low2);
	double middle_score = GetMembershipForTrapezoid(aql,middle_low1,middle_peak1,middle_peak2,middle_low2);
	double aggressive_score = GetMembershipForTrapezoid(aql,aggressive_low1,aggressive_peak1,aggressive_peak2,aggressive_low2);
	std::vector<FuzzyResult> result;
	FuzzyResult fr;
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

std::vector<FuzzyResult> MembershipMaxP(double maxp)
{
	double few_low1 = 0;
	double few_peak1 = 0.0;
	double few_peak2 = 0.004;
	double few_low2 = 0.009;
	double medium_low1 = 0.004;
	double medium_peak1 = 0.02;
	double medium_peak2 = 0.08;
	double medium_low2 = 0.1;
	double alot_low1 = 0.08;
	double alot_peak1 = 0.1;
	double alot_peak2 = 1;
	double alot_low2 = 1;
	double few_score = GetMembershipForTrapezoid(maxp,few_low1,few_peak1,few_peak2,few_low2);
	double medium_score = GetMembershipForTrapezoid(maxp,medium_low1,medium_peak1,medium_peak2,medium_low2);
	double alot_score = GetMembershipForTrapezoid(maxp,alot_low1,alot_peak1,alot_peak2,alot_low2);
	std::vector<FuzzyResult> result;
	FuzzyResult fr;
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

double MembershipDropP(std::vector<FuzzyResult> fr)
{
	double zero_low1 = 0;
	double zero_peak1 = 0.0;
	double zero_peak2 = 0.0;
	double zero_low2 = 0.001;
	double low_low1 = 0.0;
	double low_peak1 = 0.002;
	double low_peak2 = 0.004;
	double low_low2 = 0.008;
	double moderate_low1 = 0.0075;
	double moderate_peak1 = 0.05;
	double moderate_peak2 = 0.08;
	double moderate_low2 = 0.1;
	double high_low1 = 0.1;
	double high_peak1 = 0.2;
	double high_peak2 = 1;
	double high_low2 = 1;
	FuzzyResult zero,low,moderate,high;
	for(std::vector<FuzzyResult>::iterator it = fr.begin() ; it != fr.end(); ++it)
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
	double zero_centroid = GetCentroidForTrapezoid(zero.value,zero_low1,zero_peak1,zero_peak2,zero_low2);
	double low_centroid = GetCentroidForTrapezoid(low.value,low_low1,low_peak1,low_peak2,low_low2);
	double moderate_centroid = GetCentroidForTrapezoid(moderate.value,moderate_low1,moderate_peak1,moderate_peak2,moderate_low2);
	double high_centroid = GetCentroidForTrapezoid(high.value,high_low1,high_peak1,high_peak2,high_low2);
	
	int count = 0;
	if(zero_centroid > 0) count++;
	if(low_centroid > 0) count++;
	if(moderate_centroid > 0) count++;
	if(high_centroid > 0) count++;
	
	return (zero_centroid+low_centroid+moderate_centroid+high_centroid)/count;
}

FuzzyResult FuzzyInferenceRuleBase(FuzzyResult aql, FuzzyResult maxP)
{
	FuzzyResult fr;
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

std::vector<FuzzyResult> AvgClassifier(std::vector<FuzzyResult> fr)
{
	double zero=0, low=0, mod=0, high=0;
	double zc =0, lc =0, mc =0, hc =0;
	for(std::vector<FuzzyResult>::iterator it = fr.begin() ; it != fr.end(); ++it)
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
	
	FuzzyResult zf,lf,mf,hf;
	zf.name = "zero"; zf.value = zero;
	lf.name = "low"; lf.value = low;
	mf.name = "moderate"; mf.value = mod;
	hf.name = "high"; hf.value = high;
	
	std::vector<FuzzyResult> f;
	f.push_back(zf);
	f.push_back(lf);
	f.push_back(mf);
	f.push_back(hf);

	return f;
}

double TwoInputDefuzzifier(std::vector<FuzzyResult> aql, std::vector<FuzzyResult> maxP)
{
	std::vector<FuzzyResult> result;
	for (std::vector<FuzzyResult>::iterator it1 = aql.begin() ; it1 != aql.end(); ++it1)
		for (std::vector<FuzzyResult>::iterator it2 = maxP.begin() ; it2 != maxP.end(); ++it2)
		{
			FuzzyResult f = FuzzyInferenceRuleBase(*it1, *it2);
			if(f.value>0)
				result.push_back(f);
		}
	result = AvgClassifier(result);
	return MembershipDropP(result);	
}

TypeId FuzzyRedQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FuzzyRedQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName("TrafficControl")
    .AddConstructor<FuzzyRedQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (Queue::QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&FuzzyRedQueueDisc::SetMode),
                   MakeEnumChecker (Queue::QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    Queue::QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (500),
                   MakeUintegerAccessor (&FuzzyRedQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("IdlePktSize",
                   "Average packet size used during idle times. Used when m_cautions = 3",
                   UintegerValue (0),
                   MakeUintegerAccessor (&FuzzyRedQueueDisc::m_idlePktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Wait",
                   "True for waiting between dropped packets",
                   BooleanValue (true),
                   MakeBooleanAccessor (&FuzzyRedQueueDisc::m_isWait),
                   MakeBooleanChecker ())
    .AddAttribute ("Gentle",
                   "True to increases dropping probability slowly when average queue exceeds maxthresh",
                   BooleanValue (true),
                   MakeBooleanAccessor (&FuzzyRedQueueDisc::m_isGentle),
                   MakeBooleanChecker ())
    .AddAttribute ("FRED",
                   "True to enable fred",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FuzzyRedQueueDisc::m_isFRED),
                   MakeBooleanChecker ())
    .AddAttribute ("AdaptMaxP",
                   "True to adapt m_curMaxP",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FuzzyRedQueueDisc::m_isAdaptMaxP),
                   MakeBooleanChecker ())
    .AddAttribute ("MinTh",
                   "Minimum average length threshold in packets/bytes",
                   DoubleValue (5),
                   MakeDoubleAccessor (&FuzzyRedQueueDisc::m_minTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxTh",
                   "Maximum average length threshold in packets/bytes",
                   DoubleValue (15),
                   MakeDoubleAccessor (&FuzzyRedQueueDisc::m_maxTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (40),
                   MakeUintegerAccessor (&FuzzyRedQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QW",
                   "Queue weight related to the exponential weighted moving average (EWMA)",
                   DoubleValue (0.002),
                   MakeDoubleAccessor (&FuzzyRedQueueDisc::m_qW),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("LInterm",
                   "The maximum probability of dropping a packet",
                   DoubleValue (50),
                   MakeDoubleAccessor (&FuzzyRedQueueDisc::m_lInterm),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("TargetDelay",
                   "Target average queuing delay in ARED",
                   TimeValue (Seconds (0.005)),
                   MakeTimeAccessor (&FuzzyRedQueueDisc::m_targetDelay),
                   MakeTimeChecker ())
    .AddAttribute ("Interval",
                   "Time interval to update m_curMaxP",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&FuzzyRedQueueDisc::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("LastSet",
                   "Store the last time m_curMaxP was updated",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&FuzzyRedQueueDisc::m_lastSet),
                   MakeTimeChecker ())
    .AddAttribute ("Ns1Compat",
                   "NS-1 compatibility",
                   BooleanValue (false),
                   MakeBooleanAccessor (&FuzzyRedQueueDisc::m_isNs1Compat),
                   MakeBooleanChecker ())
    .AddAttribute ("LinkBandwidth", 
                   "The RED link bandwidth",
                   DataRateValue (DataRate ("1.5Mbps")),
                   MakeDataRateAccessor (&FuzzyRedQueueDisc::m_linkBandwidth),
                   MakeDataRateChecker ())
    .AddAttribute ("LinkDelay", 
                   "The RED link delay",
                   TimeValue (MilliSeconds (20)),
                   MakeTimeAccessor (&FuzzyRedQueueDisc::m_linkDelay),
                   MakeTimeChecker ())
  ;

  return tid;
}

FuzzyRedQueueDisc::FuzzyRedQueueDisc () :
  QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<WeibullRandomVariable> ();
}

FuzzyRedQueueDisc::~FuzzyRedQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
FuzzyRedQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  QueueDisc::DoDispose ();
}

void
FuzzyRedQueueDisc::SetMode (Queue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

Queue::QueueMode
FuzzyRedQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}


void
FuzzyRedQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

void
FuzzyRedQueueDisc::SetTh (double minTh, double maxTh)
{
  NS_LOG_FUNCTION (this << minTh << maxTh);
  NS_ASSERT (minTh <= maxTh);
  m_minTh = minTh;
  m_maxTh = maxTh;
  std::cout << "Minimum Threshold : "<< minTh << "Minimum Threshold : " << maxTh;
}

FuzzyRedQueueDisc::Stats
FuzzyRedQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_fstats;
}

int64_t 
FuzzyRedQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
FuzzyRedQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
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
      NS_LOG_DEBUG ("fred Queue Disc is idle.");
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

  qTimeFRED aql;
  aql.time = Simulator :: Now();
  aql.qlen = m_qAvg;
  aql.minTh = m_minTh;
  aql.maxTh = m_maxTh;
  m_fstats.avgQLen.push_back(aql);

  m_qAvg = Estimator (nQueued, m + 1, m_qAvg, m_qW);

  aql.qlen = m_qAvg;
  m_fstats.curQLen.push_back(aql);


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
      m_fstats.qLimDrop++;
    }

  if (dropType == DTYPE_UNFORCED)
    {
      NS_LOG_DEBUG ("\t Dropping due to Prob Mark " << m_qAvg);
      m_fstats.unforcedDrop++;
      Drop (item);
      return false;
    }
  else if (dropType == DTYPE_FORCED)
    {
      NS_LOG_DEBUG ("\t Dropping due to Hard Mark " << m_qAvg);
      m_fstats.forcedDrop++;
      Drop (item);
      if (m_isNs1Compat)
        {
          m_count = 0;
          m_countBytes = 0;
        }
      return false;
    }

  bool retval = GetInternalQueue (0)->Enqueue (item);

  // If Queue::Enqueue fails, QueueDisc::Drop is called by the internal queue
  // because QueueDisc::AddInternalQueue sets the drop callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return retval;
}

/*
 * Note: if the link bandwidth changes in the course of the
 * simulation, the bandwidth-dependent RED parameters do not change.
 * This should be fixed, but it would require some extra parameters,
 * and didn't seem worth the trouble...
 */
void
FuzzyRedQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Initializing RED params.");

  m_cautious = 0;
  m_ptc = m_linkBandwidth.GetBitRate () / (8.0 * m_meanPktSize);

  std::cout << "InitParams :: RedMinth : " << m_minTh << " RedMaxTh : " << m_maxTh << " m_isFRED :  "<< m_isFRED <<std::endl;

  if (m_isFRED && m_minTh == 5 && m_maxTh == 15)
    {
      // Set m_minTh, m_maxTh and m_qW to zero for automatic setting
      m_minTh = 0;
      m_maxTh = 0;
      m_qW = 0;

      // Turn on m_isAdaptMaxP to adapt m_curMaxP
      m_isAdaptMaxP = true;
    }

  if(m_isFRED)
	m_isAdaptMaxP = true;

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
  m_fstats.forcedDrop = 0;
  m_fstats.unforcedDrop = 0;
  m_fstats.qLimDrop = 0;
  m_fstats.avgQLen.clear();
  m_fstats.curQLen.clear();

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

  NS_LOG_DEBUG ("\tm_delay " << m_linkDelay.GetSeconds () << "; m_isWait " 
                             << m_isWait << "; m_qW " << m_qW << "; m_ptc " << m_ptc
                             << "; m_minTh " << m_minTh << "; m_maxTh " << m_maxTh
                             << "; m_isGentle " << m_isGentle << "; th_diff " << th_diff
                             << "; lInterm " << m_lInterm << "; va " << m_vA <<  "; cur_max_p "
                             << m_curMaxP << "; v_b " << m_vB <<  "; m_vC "
                             << m_vC << "; m_vD " <<  m_vD);
}

// Update m_curMaxP to keep the average queue length within the target range.
void
FuzzyRedQueueDisc::UpdateMaxP (double newAve, Time now)
{

   double m_part = 0.4 * (m_maxTh - m_minTh);
   double m_target = m_minTh + m_part;

   qTimeFRED aql;
   aql.time = Simulator :: Now();
   aql.qlen = m_target;
   aql.minTh = m_minTh;
   aql.maxTh = m_maxTh;
   m_fstats.avgQLen.push_back(aql);
   aql.qlen = newAve;
   m_fstats.curQLen.push_back(aql);

   if(newAve < m_minTh)
	{
		m_curMaxP = 0;
		m_lastSet = now;
	}
   else if(m_minTh <= newAve && newAve < m_target)
	{
		m_curMaxP = 4*m_curMaxP*m_curMaxP*m_curMaxP;
		m_lastSet = now;
	}
   else if(m_target <= newAve && newAve < m_maxTh)
	{
		m_curMaxP = 2*m_curMaxP;
		m_lastSet = now;
	}
   else if(m_maxTh<= newAve && newAve < 2*m_maxTh)
	{
		m_curMaxP = 4*m_curMaxP*m_curMaxP*m_curMaxP;
		m_lastSet = now;
	}
   else if(newAve > 2*m_maxTh)
	{
		m_curMaxP = 1;
		m_lastSet = now;
	} 
}

// Compute the average queue size
double
FuzzyRedQueueDisc::Estimator (uint32_t nQueued, uint32_t m, double qAvg, double qW)
{
  NS_LOG_FUNCTION (this << nQueued << m << qAvg << qW);

  double newAve = qAvg * pow(1.0-qW, m);
  newAve += qW * nQueued;

  Time now = Simulator::Now();
  if (m_isAdaptMaxP && now > m_lastSet + m_interval)
    {
      UpdateMaxP(newAve, now);
    }

  return newAve;
}

// Check if packet p needs to be dropped due to probability mark
uint32_t
FuzzyRedQueueDisc::DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize)
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
FuzzyRedQueueDisc::CalculatePNew (double qAvg, double maxTh, bool isGentle, double vA,
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
      // 
       // OLD: p continues to range linearly above max_p as
       // the average queue size ranges above th_max.
       // NEW: p is set to 1.0
       //
      p = 1.0;
    }
  else
    {
      //
       // p ranges from 0 to max_p as the average queue size ranges from
       // th_min to th_max
       //
      p = vA * qAvg + vB;
      p *= maxP;
    }
*/
	std::vector<FuzzyResult> qAvgFuzzy = MembershipAQL(qAvg);
	std::vector<FuzzyResult> maxPFuzzy = MembershipMaxP(maxP);
	double probabilityDrop = TwoInputDefuzzifier(qAvgFuzzy,maxPFuzzy);
	p = probabilityDrop;
	//p = 0;
	
  if (p > 1.0)
    {
      p = 1.0;
    }

  return p;
}

// Returns a probability using these function parameters for the DropEarly funtion
double 
FuzzyRedQueueDisc::ModifyP (double p, uint32_t count, uint32_t countBytes,
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
FuzzyRedQueueDisc::GetQueueSize (void)
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
FuzzyRedQueueDisc::DoDequeue (void)
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
FuzzyRedQueueDisc::DoPeek (void) const
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
FuzzyRedQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("FuzzyRedQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("FuzzyRedQueueDisc cannot have packet filters");
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
      NS_LOG_ERROR ("FuzzyRedQueueDisc needs 1 internal queue");
      return false;
    }

  if (GetInternalQueue (0)->GetMode () != m_mode)
    {
      NS_LOG_ERROR ("The mode of the provided queue does not match the mode set on the FuzzyRedQueueDisc");
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
