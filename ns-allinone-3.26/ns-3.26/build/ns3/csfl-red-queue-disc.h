#ifndef CSFLRED_QUEUE_DISC_H
#define CSFLRED_QUEUE_DISC_H

#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class TraceContainer;

/**
 * \ingroup traffic-control
 *
 * \brief A CSFLRED packet queue disc
 */

class qTimeCSFLRED
  {
	public:
	Time time;
	uint32_t qlen;
	uint32_t minTh;
	uint32_t maxTh;
	friend std::ostream& operator<<(std::ostream&, const qTimeCSFLRED&);
  };

std::ostream& operator<<(std::ostream& stream, const qTimeCSFLRED& obj)
{   
    stream << obj.time << ',' << obj.qlen << ',' << obj.minTh << ',' << obj.maxTh;
    return stream;
}


class CsflRedQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief CsflRedQueueDisc Constructor
   *
   * Create a CSFLRED queue disc
   */
  CsflRedQueueDisc ();

  /**
   * \brief Destructor
   *
   * Destructor
   */ 
  virtual ~CsflRedQueueDisc ();

  /** 
   * \brief Used in CSFL's Adaptive CSFLRED
   */
  typedef enum 
    {
      Above,        //!< When m_qAvg > m_maxTh
      Between,      //!< When m_maxTh < m_qAvg < m_minTh
      Below,        //!< When m_qAvg < m_minTh
    } Status_t;
  
  /**
   * \brief Stats
   */
  typedef struct
  {   
    uint32_t unforcedDrop;  //!< Early probability drops
    uint32_t forcedDrop;    //!< Forced drops, qavg > max threshold
    uint32_t qLimDrop;      //!< Drops due to queue limits
    std::vector<qTimeCSFLRED> avgQLen;
    std::vector<qTimeCSFLRED> curQLen;
  } Stats;

  /** 
   * \brief Drop types
   */
  enum
  {
    DTYPE_NONE,        //!< Ok, no drop
    DTYPE_FORCED,      //!< A "forced" drop
    DTYPE_UNFORCED,    //!< An "unforced" (random) drop
  };

  /**
   * \brief Set the operating mode of this queue.
   *  Set operating mode
   *
   * \param mode The operating mode of this queue.
   */
  void SetMode (Queue::QueueMode mode);

  /**
   * \brief Get the encapsulation mode of this queue.
   * Get the encapsulation mode of this queue
   *
   * \returns The encapsulation mode of this queue.
   */
  Queue::QueueMode GetMode (void);

  /**
   * \brief Get the current value of the queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t GetQueueSize (void);

   /**
    * \brief Set the alpha value to adapt m_curMaxP.
    *
    * \param alpha The value of alpha to adapt m_curMaxP.
    */
   void SetAredAlpha (double alpha);

   /**
    * \brief Get the alpha value to adapt m_curMaxP.
    *
    * \returns The alpha value to adapt m_curMaxP.
    */
   double GetAredAlpha (void);

   /**
    * \brief Set the beta value to adapt m_curMaxP.
    *
    * \param beta The value of beta to adapt m_curMaxP.
    */
   void SetAredBeta (double beta);

   /**
    * \brief Get the beta value to adapt m_curMaxP.
    *
    * \returns The beta value to adapt m_curMaxP.
    */
   double GetAredBeta (void);

   /**
    * \brief Set the alpha value to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    *
    * \param alpha The value of alpha to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    */
   void SetCSFLA (double a);

   /**
    * \brief Get the alpha value to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    *
    * \returns The alpha value to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    */
   double GetCSFLA (void);

   /**
    * \brief Set the beta value to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    *
    * \param beta The value of beta to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    */
   void SetCSFLB (double b);

   /**
    * \brief Get the beta value to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    *
    * \returns The beta value to adapt m_curMaxP in CSFL's Adaptive CSFLRED.
    */
   double GetCSFLB (void);

  /**
   * \brief Set the limit of the queue.
   *
   * \param lim The limit in bytes or packets.
   */
  void SetQueueLimit (uint32_t lim);

  /**
   * \brief Set the thresh limits of CSFLRED.
   *
   * \param minTh Minimum thresh in bytes or packets.
   * \param maxTh Maximum thresh in bytes or packets.
   */
  void SetTh (double minTh, double maxTh);

  /**
   * \brief Get the CSFLRED statistics after running.
   *
   * \returns The drop statistics.
   */
  Stats GetStats ();

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   *
   * Note: if the link bandwidth changes in the course of the
   * simulation, the bandwidth-dependent CSFLRED parameters do not change.
   * This should be fixed, but it would require some extra parameters,
   * and didn't seem worth the trouble...
   */
  virtual void InitializeParams (void);
  /**
   * \brief Compute the average queue size
   * \param nQueued number of queued packets
   * \param m simulated number of packets arrival during idle period
   * \param qAvg average queue size
   * \param qW queue weight given to cur q size sample
   * \returns new average queue size
   */
  double Estimator (uint32_t nQueued, uint32_t m, double qAvg, double qW);
   /**
    * \brief Update m_curMaxP
    * \param newAve new average queue length
    * \param now Current Time
    */
  void UpdateMaxP (double newAve, Time now);
   /**
    * \brief Update m_curMaxP in CSFLRED
    * \param newAve new average queue length
    */
  void UpdateMaxPCSFL (double newAve, Time now);
  /**
   * \brief Check if a packet needs to be dropped due to probability mark
   * \param item queue item
   * \param qSize queue size
   * \returns 0 for no drop/mark, 1 for drop
   */
  uint32_t DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize);
  /**
   * \brief Returns a probability using these function parameters for the DropEarly function
   * \param qAvg Average queue length
   * \param maxTh Max avg length threshold
   * \param gentle "gentle" algorithm
   * \param vA vA
   * \param vB vB
   * \param vC vC
   * \param vD vD
   * \param maxP max_p
   * \returns Prob. of packet drop before "count"
   */
  double CalculatePNew (double qAvg, double , bool gentle, double vA,
                        double vB, double vC, double vD, double maxP);
  /**
   * \brief Returns a probability using these function parameters for the DropEarly function
   * \param p Prob. of packet drop before "count"
   * \param count number of packets since last random number generation
   * \param countBytes number of bytes since last drop
   * \param meanPktSize Avg pkt size
   * \param wait True for waiting between dropped packets
   * \param size packet size
   * \returns Prob. of packet drop
   */
  double ModifyP (double p, uint32_t count, uint32_t countBytes,
                  uint32_t meanPktSize, bool wait, uint32_t size);

  Stats m_csflstats; //!< CSFLRED statistics

  // ** Variables supplied by user
  Queue::QueueMode m_mode;  //!< Mode (Bytes or packets)
  uint32_t m_meanPktSize;   //!< Avg pkt size
  uint32_t m_idlePktSize;   //!< Avg pkt size used during idle times
  bool m_isWait;            //!< True for waiting between dropped packets
  bool m_isGentle;          //!< True to increases dropping prob. slowly when ave queue exceeds maxthresh
  bool m_isARED;            //!< True to enable Adaptive RED
  bool m_isAdaptMaxP;       //!< True to adapt m_curMaxP        
  double m_minTh;           //!< Min avg length threshold (bytes)
  double m_maxTh;           //!< Max avg length threshold (bytes), should be >= 2*minTh
  uint32_t m_queueLimit;    //!< Queue limit in bytes / packets
  double m_qW;              //!< Queue weight given to cur queue size sample
  double m_lInterm;         //!< The max probability of dropping a packet
  Time m_targetDelay;       //!< Target average queuing delay in ARED
  Time m_interval;          //!< Time interval to update m_curMaxP
  double m_top;             //!< Upper bound for m_curMaxP in ARED
  double m_bottom;          //!< Lower bound for m_curMaxP in ARED
  double m_alpha;           //!< Increment parameter for m_curMaxP in ARED
  double m_beta;            //!< Decrement parameter for m_curMaxP in ARED
  Time m_rtt;               //!< Rtt to be considered while automatically setting m_bottom in ARED
  bool m_isCSFL;    //!< True to enable CSFLRED
  double m_b;               //!< Increment parameter for m_curMaxP in CSFL's Adaptive CSFLRED
  double m_a;               //!< Decrement parameter for m_curMaxP in CSFL's Adaptive CSFLRED
  bool m_isNs1Compat;       //!< Ns-1 compatibility
  DataRate m_linkBandwidth; //!< Link bandwidth
  Time m_linkDelay;         //!< Link delay

  // ** Variables maintained by CSFLRED
  double m_vProb1;          //!< Prob. of packet drop before "count"
  double m_vA;              //!< 1.0 / (m_maxTh - m_minTh)
  double m_vB;              //!< -m_minTh / (m_maxTh - m_minTh)
  double m_vC;              //!< (1.0 - m_curMaxP) / m_maxTh - used in "gentle" mode
  double m_vD;              //!< 2.0 * m_curMaxP - 1.0 - used in "gentle" mode
  double m_curMaxP;         //!< Current max_p
  Time m_lastSet;           //!< Last time m_curMaxP was updated
  double m_vProb;           //!< Prob. of packet drop
  uint32_t m_countBytes;    //!< Number of bytes since last drop
  uint32_t m_old;           //!< 0 when average queue first exceeds threshold
  uint32_t m_idle;          //!< 0/1 idle status
  double m_ptc;             //!< packet time constant in packets/second
  double m_qAvg;            //!< Average queue length
  uint32_t m_count;         //!< Number of packets since last random number generation
  Status_t m_status;        //!< For use in CSFL's Adaptive CSFLRED  
  /**
   * 0 for default CSFLRED
   * 1 experimental (see red-queue.cc)
   * 2 experimental (see red-queue.cc)
   * 3 use Idle packet size in the ptc
   */
  uint32_t m_cautious;
  Time m_idleTime;          //!< Start of current idle period

  Ptr<WeibullRandomVariable> m_uv;  //!< rng stream
};

}; // namespace ns3

#endif // CSFLRED_QUEUE_DISC_H
