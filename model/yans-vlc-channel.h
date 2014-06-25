/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef YANS_VLC_CHANNEL_H
#define YANS_VLC_CHANNEL_H


#include <vector>
#include <stdint.h>
#include "ns3/packet.h"
#include "vlc-channel.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-tx-vector.h"

namespace ns3 {

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class YansVlcPhy;

/**
 * \brief A Yans vlc channel
 * \ingroup vlc
 *
 * This wifi channel implements the propagation model described in
 * "Yet Another Network Simulator", (http://cutebugs.net/files/wns2-yans.pdf).
 *
 * This class is expected to be used in tandem with the ns3::YansvlcPhy
 * class and contains a ns3::PropagationLossModel and a ns3::PropagationDelayModel.
 * By default, no propagation models are set so, it is the caller's responsability
 * to set them before using the channel.
 */
class YansVlcChannel : public VlcChannel
{
public:
  static TypeId GetTypeId (void);

  YansVlcChannel ();
  virtual ~YansVlcChannel ();

  // inherited from Channel.
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

  /**
   * Adds the given YansvlcPhy to the PHY list
   *
   * \param phy the YansvlcPhy to be added to the PHY list
   */
  void Add (Ptr<YansVlcPhy> phy_vlc);
 /**
   * \param loss the new propagation loss model.
   */
  void SetPropagationLossModel (Ptr<PropagationLossModel> loss_vlc);
  /**
   * \param delay the new propagation delay model.
   */
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay_vlc);

  /**
   * \param sender the device from which the packet is originating.
   * \param packet the packet to send
   * \param txPowerDbm the tx power associated to the packet
   * \param txVector the TXVECTOR associated to the packet
   * \param preamble the preamble associated to the packet
   *
   * This method should not be invoked by normal users. It is
   * currently invoked only from VlcPhy::Send. YansWifiChannel
   * delivers packets only between PHYs with the same m_channelNumber,
   * e.g. PHYs that are operating on the same channel.
   */
  void Send (Ptr<YansVlcPhy> sender_vlc, Ptr<const Packet> packet_vlc, double txPowerDbm_vlc,
             WifiTxVector txVector_vlc, WifiPreamble preamble_vlc) const;

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream_vlc);

private:
  //YansVlcChannel& operator = (const YansVlcChannel &);
  //YansVlcChannel (const YansVlcChannel &);

  /**
   * A vector of pointers to YansWifiPhy.
   */
  typedef std::vector<Ptr<YansVlcPhy> > PhyList;
 /**
   * This method is scheduled by Send for each associated YansVlcPhy.
   * The method then calls the corresponding YansWifiPhy that the first
   * bit of the packet has arrived.
   *
   * \param i index of the corresponding YansWifiPhy in the PHY list
   * \param packet the packet being sent
   * \param rxPowerDbm the received power of the packet
   * \param txVector the TXVECTOR of the packet
   * \param preamble the type of preamble being used to send the packet
   */
  void Receive (uint32_t i_vlc, Ptr<Packet> packet_vlc, double rxPowerDbm_vlc,
                WifiTxVector txVector, WifiPreamble preamble_vlc) const;


  PhyList m_phyList; //!< List of VlcsWifiPhys connected to this YansVlcChannel
  Ptr<PropagationLossModel> m_loss; //!< Propagation loss model
  Ptr<PropagationDelayModel> m_delay; //!< Propagation delay model
};

} // namespace ns3


#endif /* YANS_VLC_CHANNEL_H */






    
    


 




