


#ifndef YANS_VLC_PHY_H
#define YANS_VLC_PHY_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "vlc-phy.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/interference-helper.h"


namespace ns3 {

#define HT_PHY 127

class YansVlcChannel;
class VlcPhyStateHelper;


/**
 * \brief 802.11 PHY layer model
 * \ingroup wifi
 *
 * This PHY implements a model of 802.11a. The model
 * implemented here is based on the model described
 * in "Yet Another Network Simulator",
 * (http://cutebugs.net/files/wns2-yans.pdf).
 *
 *
 * This PHY model depends on a channel loss and delay
 * model as provided by the ns3::PropagationLossModel
 * and ns3::PropagationDelayModel classes, both of which are
 * members of the ns3::YansVlcChannel class.
 */
class YansVlcPhy : public VlcPhy
{
public:
  static TypeId GetTypeId (void);

  YansVlcPhy ();
  virtual ~YansVlcPhy ();

  /**
   * Set the YansVlcChannel this YansVlcPhy is to be connected to.
   *
   * \param channel the YansVlcChannel this YansVlcPhy is to be connected to
   */
  void SetChannel (Ptr<YansVlcChannel> channel_vlc);

  /**
   * Set the current channel number.
   *
   * \param id the channel number
   */
  void SetChannelNumber (uint16_t id_vlc);
  /**
   * Return the current channel number.
   *
   * \return the current channel number
   */
  uint16_t GetChannelNumber () const;
  /**
   * Return current center channel frequency in MHz.
   *
   * \return the current center channel frequency in MHz
   */  
  double GetChannelFrequencyMhz () const;

  /**
   * Starting receiving the packet (i.e. the first bit of the preamble has arrived).
   *
   * \param packet the arriving packet
   * \param rxPowerDbm the receive power in dBm
   * \param txVector the TXVECTOR of the arriving packet
   * \param preamble the preamble of the arriving packet
   */
  void StartReceivePacket (Ptr<Packet> packet_vlc,
                           double rxPowerDbm_vlc,
                           WifiTxVector txVector_vlc,
                           WifiPreamble preamble_vlc);

  /**
   * Sets the RX loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver.
   *
   * \param noiseFigureDb noise figure in dB
   */
  void SetRxNoiseFigure (double noiseFigureDb_vlc);
  /**
   * Sets the minimum available transmission power level (dBm).
   *
   * \param start the minimum transmission power level (dBm)
   */
  void SetTxPowerStart (double start_vlc);
  /**
   * Sets the maximum available transmission power level (dBm).
   *
   * \param end the maximum transmission power level (dBm)
   */
  void SetTxPowerEnd (double end_vlc);
  /**
   * Sets the number of transmission power levels available between the
   * minimum level and the maximum level.  Transmission power levels are
   * equally separated (in dBm) with the minimum and the maximum included.
   *
   * \param n the number of available levels
   */
  void SetNTxPower (uint32_t n_vlc);
  /**
   * Sets the transmission gain (dB).
   *
   * \param gain the transmission gain in dB
   */
  void SetTxGain (double gain_vlc);
  /**
   * Sets the reception gain (dB).
   *
   * \param gain the reception gain in dB
   */
  void SetRxGain (double gain_vlc);
  /**
   * Sets the energy detection threshold (dBm).https://www.yahoo.com/
   * The energy of a received signal should be higher than
   * this threshold (dbm) to allow the PHY layer to detect the signal.
   *
   * \param threshold the energy detction threshold in dBm
   */
  void SetEdThreshold (double threshold_vlc);
  /**
   * Sets the CCA threshold (dBm).  The energy of a received signal
   * should be higher than this threshold to allow the PHY
   * layer to declare CCA BUSY state.
   *
   * \param threshold the CCA threshold in dBm
   */
  void SetCcaMode1Threshold (double threshold_vlc);
  /**
   * Sets the error rate model.
   *
   * \param rate the error rate model
   */
  void SetErrorRateModel (Ptr<ErrorRateModel> rate_vlc);
  /**
   * Sets the device this PHY is associated with.
   *
  ://www.yahoo.com/https://www.yahoo.com/https://www.yahoo.com/ommandLine cmd;
  cmd.AddValue ("nWifi1", "Number of wifi1 STA devices", nWifi1);
  cmd.AddValue ("nWifi2", "Number of wifi2 STA devices", nWifi2);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
   * \param device the device this PHY is associated with
   */
  void SetDevice (Ptr<Object> device_vlc);
  /**
   * Sets the mobility model.
   *
   * \param mobility the mobility model this PHY is associated with
   */
  void SetMobility (Ptr<Object> mobility_vlc);
  /**
   * Return the RX noise figure (dBm).
   *
   * \return the RX noise figure in dBm
   */
  double GetRxNoiseFigure (void) const;
  /**
   * Return the transmission gain (dB).
   *
   * \return the transmission gain in dB
   */
  double GetTxGain (void) const;
  /**
   * Return the reception gain (dB).
   *
   * \return the reception gain in dB
   */
  double GetRxGain (void) const;
  /**
   * Return the energy detection threshold (dBm).
   *
   * \return the energy detection threshold in dBm
   */
  double GetEdThreshold (void) const;
  /**
   * Return the CCA threshold (dBm).
   *
   * \return the CCA threshold in dBm
   */
  double GetCcaMode1Threshold (void) const;
  /**
   * Return the error rate model this PHY is using.
   *
   * \return the error rate model this PHY is using
   */
  Ptr<ErrorRateModel> GetErrorRateModel (void) const;
  /**
   * Return the device this PHY is associated with
   *
   * \return the device this PHY is associated with
   */
  Ptr<Object> GetDevice (void) const;
  /**
   * Return the mobility model this PHY is associated with.
   *
   * \return the mobility model this PHY is associated with
   */
  Ptr<Object> GetMobility (void);

  /**
   * Return the minimum available transmission power level (dBm).
   * \return the minimum available transmission power level (dBm)
   */
  virtual double GetTxPowerStart (void) const; 
  /**
   * Return the maximum available transmission power level (dBm).
   * \return the maximum available transmission power level (dBm)
   */
  virtual double GetTxPowerEnd (void) const;
  /**
   * Return the number of available transmission power levels.
   *
   * \return the number of available transmission power levels
   */
  virtual uint32_t GetNTxPower (void) const;
  virtual void SetReceiveOkCallback (VlcPhy::RxOkCallback callback);
  virtual void SetReceiveErrorCallback (VlcPhy::RxErrorCallback callback);
  virtual void SendPacket (Ptr<const Packet> packet_vlc, WifiMode mode_vlc, enum WifiPreamble preamble_vlc, WifiTxVector txvector_vlc);
  virtual void RegisterListener (WifiPhyListener *listener);
  virtual bool IsStateCcaBusy (void);
  virtual bool IsStateIdle (void);
  virtual bool IsStateBusy (void);
  virtual bool IsStateRx (void);
  virtual bool IsStateTx (void);
  virtual bool IsStateSwitching (void);
  virtual Time GetStateDuration (void);
  virtual Time GetDelayUntilIdle (void);
  virtual Time GetLastRxStartTime (void) const;
  virtual uint32_t GetNModes (void) const;
  virtual WifiMode GetMode (uint32_t mode_vlc) const;
  virtual double CalculateSnr (WifiMode txMode, double ber_vlc) const;
  virtual Ptr<VlcChannel> GetChannel (void) const;
  
  virtual void ConfigureStandard (enum WifiPhyStandard standard_vlc);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream_vlc);

  /**
   * \param freq the operating frequency on this node (2.4 GHz or 5GHz).
   */
  virtual void SetFrequency (uint32_t freq_vlc);
  /**
   * \return the operating frequency on this node
   */
  virtual uint32_t GetFrequency (void) const;
  /**
   * \param tx the number of transmitters on this node.
   */
  virtual void SetNumberOfTransmitAntennas (uint32_t tx_vlc);
  virtual uint32_t GetNumberOfTransmitAntennas (void) const;
  /**
   * \param rx the number of receivers on this node.
   */
  virtual void SetNumberOfReceiveAntennas (uint32_t rx_vlc) ;
  /**
   * \return the number of receivers on this node.
   */
  virtual uint32_t GetNumberOfReceiveAntennas (void) const;
  /**
   * Enable or disable short/long guard interval.
   *
   * \param guardIntervalhttps://www.yahoo.com/ Enable or disable guard interval
   */
  virtual void SetGuardInterval (bool guardInterval_vlc);
  /**
   * Return whether guard interval is being used.
   *
   * \return true if guard interval is being used, false otherwise
   */
  virtual bool GetGuardInterval (void) const;
  /**
   * Enable or disable LDPC.
   * \param ldpc Enable or disable LDPC
   */
  virtual void SetLdpc (bool ldpc_vlc);
  /**
   * Return if LDPC is supported.
   *
   * \return true if LDPC is supported, false otherwise
   */
  virtual bool GetLdpc (void) const;
  /**
   * Enable or disable STBC.
   *
   * \param stbc Enable or disable STBC
   */
  virtual void SetStbc (bool stbc_vlc);
  /**
   * Return whether STBC is supported. 
   *
   * \return true if STBC is supported, false otherwise
   */
  virtual bool GetStbc (void) const;
  /**
   * Enable or disable Greenfield support.
   *
   * \param greenfield Enable or disable Greenfield
   */
  virtual void SetGreenfield (bool greenfield_vlc);
  /**
   * Return whether Greenfield is supported.
   *
   * \return true if Greenfield is supported, false otherwise
   */
  virtual bool GetGreenfield (void) const;
  /**
   * Return whether channel bonding is supported.
   * 
   * \return true if channel bonding is supported, false otherwise
   */
  virtual bool GetChannelBonding (void) const ;
  /**
   * Enable or disable channel bonding support.
   * 
   * \param channelbonding Enable or disable channel bonding
   */
  virtual void SetChannelBonding (bool channelbonding_vlc) ;

  virtual uint32_t GetNBssMembershipSelectors (void) const;
  virtual uint32_t GetBssMembershipSelector (uint32_t selector_vlc) const;
  virtual WifiModeList GetMembershipSelectorModes(uint32_t selector_vlc);
  /**
   * \return the number of MCS supported by this phy
   */
  virtual uint8_t GetNMcs (void) const;
  virtual uint8_t GetMcs (uint8_t mcs_vlc) const;

  virtual uint32_t WifiModeToMcs (WifiMode mode_vlc);
  virtual WifiMode McsToWifiMode (uint8_t mcs_vlc);

private:
  //YansVlcPhy (const YansVlcPhy &o);
  virtual void DoDispose (void);
  /**
   * Configure YansVlcPhy with appropriate channel frequency and
   * supported rates for 802.11a standard.
   */
  void Configure80211a (void);
  /**
   * Configure YansVlcPhy with appropriate channel frequency and
   * supported rates for 802.11b standard.
   */
  void Configure80211b (void);
  /**
   * Configure YansVlcPhy with appropriate channel frequency and
   * supported rates for 802.11g standard.
   */
  void Configure80211g (void);
  /**
   * Configure YansVlcPhy with appropriate channel frequency and
   * supported rates for 802.11a standard with 10MHz channel spacing.
   */
  void Configure80211_10Mhz (void);
  /**
   * Configure YansVlcPhy with appropriate channel frequency and
   * supported rates for 802.11a standard with 5MHz channel spacing.
   */
  void Configure80211_5Mhz ();
  void ConfigureHolland (void);
  /**
   * Configure YansVlcPhy with appropriate channel frequency and
   * supported rates for 802.11n standard.
   */
  void Configure80211n (void);
  /**
   * Return the energy detection threshold.
   *
   * \return the energy detection threshold.
   */
  double GetEdThresholdW (void) const;
  /**
   * Convert from dBm to Watts.
   *
   * \param dbm the power in dBm
   * \return the equivalent Watts for the given dBm
   */
  double DbmToW (double dbm_vlc) const;
  /**
   * Convert from dB to ratio.
   *
   * \param db
   * \return ratio
   */
  double DbToRatio (double db_vlc) const;
  /**
   * Convert from Watts to dBm.
   *
   * \param w the power in Watts
   * \return the equivalent dBm for the given Watts
   */
  double WToDbm (double w_vlc) const;
  /**
   * Convert from ratio to dB.
   *
   * \param ratio
   * \return dB
   */
  double RatioToDb (double ratio_vlc) const;
  /**
   * Get the power of the given power level in dBm.
   * In YansWifiPhy implementation, the power levels are equally spaced (in dBm).
   *
   * \param power the power level
   * \return the transmission power in dBm at the given power level
   */
  double GetPowerDbm (uint8_t power_vlc) const;
  /**
   * The last bit of the packet has arrived.
   *
   * \param packet the packet that the last bit has arrived
   * \param event the corresponding event of the first time the packet arrives
   */
  void EndReceive (Ptr<Packet> packet_vlc, Ptr<InterferenceHelper::Event> event_vlc);

private:
  double   m_edThresholdW;        //!< Energy detection threshold in watts
  double   m_ccaMode1ThresholdW;  //!< Clear channel assessment (CCA) threshold in watts
  double   m_txGainDb;            //!< Transmission gain (dB)
  double   m_rxGainDb;            //!< Reception gain (dB)
  double   m_txPowerBaseDbm;      //!< Minimum transmission power (dBm)
  double   m_txPowerEndDbm;       //!< Maximum transmission power (dBm)
  uint32_t m_nTxPower;            //!< Number of available transmission power levels

  Ptr<YansVlcChannel> m_channel;        //!< YansVlcChannel that this YansVlcPhy is connected to
  uint16_t             m_channelNumber;  //!< Operating channel number
  Ptr<Object>          m_device;         //!< Pointer to the device
  Ptr<Object>          m_mobility;       //!< Pointer to the mobility model

  uint32_t m_numberOfTransmitters;  //!< Number of transmitters
  uint32_t m_numberOfReceivers;     //!< Number of receivers
  bool     m_ldpc;                  //!< Flag if LDPC is used
  bool     m_stbc;                  //!< Flag if STBC is used      
  bool     m_greenfield;            //!< Flag if GreenField format is supported
  bool     m_guardInterval;         //!< Flag if short guard interval is used
  bool     m_channelBonding;        //!< Flag if channel conding is used


  /**
   * This vector holds the set of transmission modes that this
   * VlcPhy(-derived class) can support. In conversation we call this
   * the DeviceRateSet (not a term you'll find in the standard), and
   * it is a superset of standard-defined parameters such as the
   * OperationalRateSet, and the BSSBasicRateSet (which, themselves,
   * have a superset/subset relationship).
   *
   * Mandatory rates relevant to this VlcPhy can be found by
   * iterating over this vector looking for WifiMode objects for which
   * WifiMode::IsMandatory() is true.
   *
   * A quick note is appropriate here (well, here is as good a place
   * as any I can find)...
   *
   * In the standard there is no text that explicitly precludes
   * production of a device that does not support some rates that are
   * mandatory (according to the standard) for PHYs that the device
   * happens to fully or partially support.
   *
   * This approach is taken by some devices which choose to only support,
   * for example, 6 and 9 Mbps ERP-OFDM rates for cost and power
   * consumption reasons (i.e., these devices don't need to be designed
   * for and waste current on the increased linearity requirement of
   * higher-order constellations when 6 and 9 Mbps more than meet their
   * data requirements). The wording of the standard allows such devices
   * to have an OperationalRateSet which includes 6 and 9 Mbps ERP-OFDM
   * rates, despite 12 and 24 Mbps being "mandatory" rates for the
   * ERP-OFDM PHY.
   *
   * Now this doesn't actually have any impact on code, yet. It is,
   * however, something that we need to keep in mind for the
   * future. Basically, the key point is that we can't be making
   * assumptions like "the Operational Rate Set will contain all the
   * mandatory rates".
   */
  WifiModeList m_deviceRateSet;
  
  std::vector<uint32_t> m_bssMembershipSelectorSet;
  std::vector<uint8_t> m_deviceMcsSet;
  EventId m_endRxEvent;

  Ptr<UniformRandomVariable> m_random;  //!< Provides uniform random variables.
  double m_channelStartingFrequency;    //!< Standard-dependent center frequency of 0-th channel in MHz
  Ptr<VlcPhyStateHelper> m_state;      //!< Pointer to VlcPhyStateHelper
  InterferenceHelper m_interference;    //!< Pointer to InterferenceHelper
  Time m_channelSwitchDelay;            //!< Time required to switch between channel

};

} // namespace ns3


#endif /* YANS_VLC_PHY_H */
