/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Ghada Badawy <gbadawy@gmail.com>
 */

#include "yans-vlc-phy.h"
#include "yans-vlc-channel.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "vlc-phy-state-helper.h"
#include "ns3/error-rate-model.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("YansvlcPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (YansVlcPhy)
  ;

TypeId
YansVlcPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YansvlcPhy")
    .SetParent<VlcPhy> ()
    .AddConstructor<YansVlcPhy> ()
    .AddAttribute ("EnergyDetectionThreshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to detect the signal.",
                   DoubleValue (-96.0),
                   MakeDoubleAccessor (&YansVlcPhy::SetEdThreshold,
                                       &YansVlcPhy::GetEdThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CcaMode1Threshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state",
                   DoubleValue (-99.0),
                   MakeDoubleAccessor (&YansVlcPhy::SetCcaMode1Threshold,
                                       &YansVlcPhy::GetCcaMode1Threshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxGain",
                   "Transmission gain (dB).",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&YansVlcPhy::SetTxGain,
                                       &YansVlcPhy::GetTxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxGain",
                   "Reception gain (dB).",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&YansVlcPhy::SetRxGain,
                                       &YansVlcPhy::GetRxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerLevels",
                   "Number of transmission power levels available between "
                   "TxPowerStart and TxPowerEnd included.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&YansVlcPhy::m_nTxPower),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TxPowerEnd",
                   "Maximum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&YansVlcPhy::SetTxPowerEnd,
                                       &YansVlcPhy::GetTxPowerEnd),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerStart",
                   "Minimum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&YansVlcPhy::SetTxPowerStart,
                                       &YansVlcPhy::GetTxPowerStart),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxNoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0 (usually 290 K)\"."
                   " For",
                   DoubleValue (7),
                   MakeDoubleAccessor (&YansVlcPhy::SetRxNoiseFigure,
                                       &YansVlcPhy::GetRxNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("State", "The state of the PHY layer",
                   PointerValue (),
                   MakePointerAccessor (&YansVlcPhy::m_state),
                   MakePointerChecker<VlcPhyStateHelper> ())
    .AddAttribute ("ChannelSwitchDelay",
                   "Delay between two short frames transmitted on different frequencies.",
                   TimeValue (MicroSeconds (250)),
                   MakeTimeAccessor (&YansVlcPhy::m_channelSwitchDelay),
                   MakeTimeChecker ())
    .AddAttribute ("ChannelNumber",
                   "Channel center frequency = Channel starting frequency + 5 MHz * nch",
                   UintegerValue (1),
                   MakeUintegerAccessor (&YansVlcPhy::SetChannelNumber,
                                         &YansVlcPhy::GetChannelNumber),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Frequency", "The operating frequency.",
                   UintegerValue (2407),
                   MakeUintegerAccessor (&YansVlcPhy::GetFrequency,
                                        &YansVlcPhy::SetFrequency),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Transmitters", "The number of transmitters.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&YansVlcPhy::GetNumberOfTransmitAntennas,
                                        &YansVlcPhy::SetNumberOfTransmitAntennas),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Recievers", "The number of recievers.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&YansVlcPhy::GetNumberOfReceiveAntennas,
                                        &YansVlcPhy::SetNumberOfReceiveAntennas),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ShortGuardEnabled", "Whether or not short guard interval is enabled.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&YansVlcPhy::GetGuardInterval,
                                        &YansVlcPhy::SetGuardInterval),
                   MakeBooleanChecker ())
    .AddAttribute ("LdpcEnabled", "Whether or not LDPC is enabled.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&YansVlcPhy::GetLdpc,
                                        &YansVlcPhy::SetLdpc),
                   MakeBooleanChecker ())
    .AddAttribute ("STBCEnabled", "Whether or not STBC is enabled.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&YansVlcPhy::GetStbc,
                                        &YansVlcPhy::SetStbc),
                   MakeBooleanChecker ())
    .AddAttribute ("GreenfieldEnabled", "Whether or not STBC is enabled.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&YansVlcPhy::GetGreenfield,
                                        &YansVlcPhy::SetGreenfield),
                   MakeBooleanChecker ())
    .AddAttribute ("ChannelBonding", "Whether 20MHz or 40MHz.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&YansVlcPhy::GetChannelBonding,
                                        &YansVlcPhy::SetChannelBonding),
                   MakeBooleanChecker ())


  ;
  return tid;
}

YansVlcPhy::YansVlcPhy ()
  :  m_channelNumber (1),
    m_endRxEvent (),
    m_channelStartingFrequency (0)
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
  m_state = CreateObject<VlcPhyStateHelper> ();
}

YansVlcPhy::~YansVlcPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
YansVlcPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_deviceRateSet.clear ();
  m_deviceMcsSet.clear();
  m_device = 0;
  m_mobility = 0;
  m_state = 0;
}

void
YansVlcPhy::ConfigureStandard (enum WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      Configure80211a ();
      break;
    case WIFI_PHY_STANDARD_80211b:
      Configure80211b ();
      break;
    case WIFI_PHY_STANDARD_80211g:
      Configure80211g ();
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      Configure80211_10Mhz ();
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      Configure80211_5Mhz ();
      break;
    case WIFI_PHY_STANDARD_holland:
      ConfigureHolland ();
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      m_channelStartingFrequency=2407;
      Configure80211n ();
      break;
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      m_channelStartingFrequency=5e3;
      Configure80211n ();
      break;

    default:
      NS_ASSERT (false);
      break;
    }
}


void
YansVlcPhy::SetRxNoiseFigure (double noiseFigureDb_vlc)
{
  NS_LOG_FUNCTION (this << noiseFigureDb_vlc);
  m_interference.SetNoiseFigure (DbToRatio (noiseFigureDb_vlc));
}
void
YansVlcPhy::SetTxPowerStart (double start_vlc)
{
  NS_LOG_FUNCTION (this << start_vlc);
  m_txPowerBaseDbm = start_vlc;
}
void
YansVlcPhy::SetTxPowerEnd (double end_vlc)
{
  NS_LOG_FUNCTION (this << end_vlc);
  m_txPowerEndDbm = end_vlc;
}
void
YansVlcPhy::SetNTxPower (uint32_t n_vlc)
{
  NS_LOG_FUNCTION (this << n_vlc);
  m_nTxPower = n_vlc;
}
void
YansVlcPhy::SetTxGain (double gain_vlc)
{
  NS_LOG_FUNCTION (this << gain_vlc);
  m_txGainDb = gain_vlc;
}
void
YansVlcPhy::SetRxGain (double gain_vlc)
{
  NS_LOG_FUNCTION (this << gain_vlc);
  m_rxGainDb = gain_vlc;
}
void
YansVlcPhy::SetEdThreshold (double threshold_vlc)
{
  NS_LOG_FUNCTION (this << threshold_vlc);
  m_edThresholdW = DbmToW (threshold_vlc);
}
void
YansVlcPhy::SetCcaMode1Threshold (double threshold_vlc)
{
  NS_LOG_FUNCTION (this << threshold_vlc);
  m_ccaMode1ThresholdW = DbmToW (threshold_vlc);
}
void
YansVlcPhy::SetErrorRateModel (Ptr<ErrorRateModel> rate_vlc)
{
  m_interference.SetErrorRateModel (rate_vlc);
}
void
YansVlcPhy::SetDevice (Ptr<Object> device_vlc)
{
  m_device = device_vlc;
}
void
YansVlcPhy::SetMobility (Ptr<Object> mobility_vlc)
{
  m_mobility = mobility_vlc;
}

double
YansVlcPhy::GetRxNoiseFigure (void) const
{
  return RatioToDb (m_interference.GetNoiseFigure ());
}
double
YansVlcPhy::GetTxPowerStart (void) const
{
  return m_txPowerBaseDbm;
}
double
YansVlcPhy::GetTxPowerEnd (void) const
{
  return m_txPowerEndDbm;
}
double
YansVlcPhy::GetTxGain (void) const
{
  return m_txGainDb;
}
double
YansVlcPhy::GetRxGain (void) const
{
  return m_rxGainDb;
}

double
YansVlcPhy::GetEdThreshold (void) const
{
  return WToDbm (m_edThresholdW);
}

double
YansVlcPhy::GetCcaMode1Threshold (void) const
{
  return WToDbm (m_ccaMode1ThresholdW);
}

Ptr<ErrorRateModel>
YansVlcPhy::GetErrorRateModel (void) const
{
  return m_interference.GetErrorRateModel ();
}
Ptr<Object>
YansVlcPhy::GetDevice (void) const
{
  return m_device;
}
Ptr<Object>
YansVlcPhy::GetMobility (void)
{
  return m_mobility;
}

double
YansVlcPhy::CalculateSnr (WifiMode txMode, double ber_vlc) const
{
  return m_interference.GetErrorRateModel ()->CalculateSnr (txMode, ber_vlc);
}

Ptr<VlcChannel>
YansVlcPhy::GetChannel (void) const
{
  return m_channel;
}
void
YansVlcPhy::SetChannel (Ptr<YansVlcChannel> channel_vlc)
{
  m_channel = channel_vlc;
  m_channel->Add (this);
}

void
YansVlcPhy::SetChannelNumber (uint16_t nch_vlc)
{
  if (Simulator::Now () == Seconds (0))
    {
      // this is not channel switch, this is initialization
      NS_LOG_DEBUG ("start at channel " << nch_vlc);
      m_channelNumber = nch_vlc;
      return;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (m_state->GetState ())
    {
    case YansVlcPhy::RX:
      NS_LOG_DEBUG ("drop packet because of channel switching while reception");
      m_endRxEvent.Cancel ();
      goto switchChannel;
      break;
    case YansVlcPhy::TX:
      NS_LOG_DEBUG ("channel switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &YansVlcPhy::SetChannelNumber, this, nch_vlc);
      break;
    case YansVlcPhy::CCA_BUSY:
    case YansVlcPhy::IDLE:
      goto switchChannel;
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return;

switchChannel:

  NS_LOG_DEBUG ("switching channel " << m_channelNumber << " -> " << nch_vlc);
  m_state->SwitchToChannelSwitching (m_channelSwitchDelay);
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  m_channelNumber = nch_vlc;
}

uint16_t
YansVlcPhy::GetChannelNumber () const
{
  return m_channelNumber;
}

double
YansVlcPhy::GetChannelFrequencyMhz () const
{
  return m_channelStartingFrequency + 5 * GetChannelNumber ();
}

void
YansVlcPhy::SetReceiveOkCallback (RxOkCallback callback)
{
  m_state->SetReceiveOkCallback (callback);
}
void
YansVlcPhy::SetReceiveErrorCallback (RxErrorCallback callback)
{
  m_state->SetReceiveErrorCallback (callback);
}
void
YansVlcPhy::StartReceivePacket (Ptr<Packet> packet_vlc,
                                 double rxPowerDbm_vlc,
                                 WifiTxVector txVector_vlc,
                                 enum WifiPreamble preamble_vlc)
{
  NS_LOG_FUNCTION (this << packet_vlc << rxPowerDbm_vlc << txVector_vlc.GetMode()<< preamble_vlc);
  rxPowerDbm_vlc += m_rxGainDb;
  double rxPowerW = DbmToW (rxPowerDbm_vlc);
  Time rxDuration = CalculateTxDuration (packet_vlc->GetSize (), txVector_vlc, preamble_vlc);
WifiMode txMode=txVector_vlc.GetMode();
  Time endRx = Simulator::Now () + rxDuration;

  Ptr<InterferenceHelper::Event> event_vlc;
  event_vlc = m_interference.Add (packet_vlc->GetSize (),
                              txMode,
                              preamble_vlc,
                              rxDuration,
                              rxPowerW,
		          txVector_vlc);  // we need it to calculate duration of HT training symbols

  switch (m_state->GetState ())
    {
    case YansVlcPhy::SWITCHING:
      NS_LOG_DEBUG ("drop packet because of channel switching");
      NotifyRxDrop (packet_vlc);
      /*
       * Packets received on the upcoming channel are added to the event list
       * during the switching state. This way the medium can be correctly sensed
       * when the device listens to the channel for the first time after the
       * switching e.g. after channel switching, the channel may be sensed as
       * busy due to other devices' tramissions started before the end of
       * the switching.
       */
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          // that packet will be noise _after_ the completion of the
          // channel switching.
          goto maybeCcaBusy;
        }
      break;
    case YansVlcPhy::RX:
      NS_LOG_DEBUG ("drop packet because already in Rx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet_vlc);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          // that packet will be noise _after_ the reception of the
          // currently-received packet.
          goto maybeCcaBusy;
        }
      break;
    case YansVlcPhy::TX:
      NS_LOG_DEBUG ("drop packet because already in Tx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet_vlc);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          // that packet will be noise _after_ the transmission of the
          // currently-transmitted packet.
          goto maybeCcaBusy;
        }
      break;
    case YansVlcPhy::CCA_BUSY:
    case YansVlcPhy::IDLE:
      if (rxPowerW > m_edThresholdW)
        {
          NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
          // sync to signal
          m_state->SwitchToRx (rxDuration);
          NS_ASSERT (m_endRxEvent.IsExpired ());
          NotifyRxBegin (packet_vlc);
          m_interference.NotifyRxStart ();
          m_endRxEvent = Simulator::Schedule (rxDuration, &YansVlcPhy::EndReceive, this,
                                              packet_vlc,
                                              event_vlc);
        }
      else
        {
          NS_LOG_DEBUG ("drop packet because signal power too Small (" <<
                        rxPowerW << "<" << m_edThresholdW << ")");
          NotifyRxDrop (packet_vlc);
          goto maybeCcaBusy;
        }
      break;
    }

  return;

maybeCcaBusy:
  // We are here because we have received the first bit of a packet and we are
  // not going to be able to synchronize on it
  // In this model, CCA becomes busy when the aggregation of all signals as
  // tracked by the InterferenceHelper class is higher than the CcaBusyThreshold

  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaMode1ThresholdW);
  if (!delayUntilCcaEnd.IsZero ())
    {
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }
}

void
YansVlcPhy::SendPacket (Ptr<const Packet> packet_vlc, WifiMode txMode, WifiPreamble preamble_vlc, WifiTxVector txVector_vlc)
{
  NS_LOG_FUNCTION (this << packet_vlc << txMode << preamble_vlc << (uint32_t)txVector_vlc.GetTxPowerLevel());
  /* Transmission can happen if:
   *  - we are syncing on a packet. It is the responsability of the
   *    MAC layer to avoid doing this but the PHY does nothing to
   *    prevent it.
   *  - we are idle
   */
  NS_ASSERT (!m_state->IsStateTx () && !m_state->IsStateSwitching ());

  Time txDuration = CalculateTxDuration (packet_vlc->GetSize (), txVector_vlc, preamble_vlc);
  if (m_state->IsStateRx ())
    {
      m_endRxEvent.Cancel ();
      m_interference.NotifyRxEnd ();
    }
  NotifyTxBegin (packet_vlc);
  uint32_t dataRate500KbpsUnits = txVector_vlc.GetMode().GetDataRate () * txVector_vlc.GetNss() / 500000;
  bool isShortPreamble = (WIFI_PREAMBLE_SHORT == preamble_vlc);
  NotifyMonitorSniffTx (packet_vlc, (uint16_t)GetChannelFrequencyMhz (), GetChannelNumber (), dataRate500KbpsUnits, isShortPreamble, txVector_vlc.GetTxPowerLevel());
  m_state->SwitchToTx (txDuration, packet_vlc, txVector_vlc.GetMode(), preamble_vlc,  txVector_vlc.GetTxPowerLevel());
  m_channel->Send (this, packet_vlc, GetPowerDbm ( txVector_vlc.GetTxPowerLevel()) + m_txGainDb, txVector_vlc, preamble_vlc);
}

uint32_t
YansVlcPhy::GetNModes (void) const
{
  return m_deviceRateSet.size ();
}
WifiMode
YansVlcPhy::GetMode (uint32_t mode_vlc) const
{
  return m_deviceRateSet[mode_vlc];
}
uint32_t
YansVlcPhy::GetNTxPower (void) const
{
  return m_nTxPower;
}

void
YansVlcPhy::Configure80211a (void)
{
  NS_LOG_FUNCTION (this);
  m_channelStartingFrequency = 5e3; // 5.000 GHz

  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate9Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate24Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate48Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate54Mbps ());
}


void
YansVlcPhy::Configure80211b (void)
{
  NS_LOG_FUNCTION (this);
  m_channelStartingFrequency = 2407; // 2.407 GHz

  m_deviceRateSet.push_back (VlcPhy::GetDsssRate1Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate2Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate5_5Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate11Mbps ());
}

void
YansVlcPhy::Configure80211g (void)
{
  NS_LOG_FUNCTION (this);
  m_channelStartingFrequency = 2407; // 2.407 GHz

  m_deviceRateSet.push_back (VlcPhy::GetDsssRate1Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate2Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate5_5Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate6Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate9Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate11Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate12Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate18Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate24Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate36Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate48Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate54Mbps ());
}

void
YansVlcPhy::Configure80211_10Mhz (void)
{
  NS_LOG_FUNCTION (this);
  m_channelStartingFrequency = 5e3; // 5.000 GHz, suppose 802.11a

  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate3MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate4_5MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate6MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate9MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate12MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate18MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate24MbpsBW10MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate27MbpsBW10MHz ());
}

void
YansVlcPhy::Configure80211_5Mhz (void)
{
  NS_LOG_FUNCTION (this);
  m_channelStartingFrequency = 5e3; // 5.000 GHz, suppose 802.11a

  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate1_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate2_25MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate3MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate4_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate6MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate9MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate12MbpsBW5MHz ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate13_5MbpsBW5MHz ());
}

void
YansVlcPhy::ConfigureHolland (void)
{
  NS_LOG_FUNCTION (this);
  m_channelStartingFrequency = 5e3; // 5.000 GHz
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetOfdmRate54Mbps ());
}

void
YansVlcPhy::RegisterListener (WifiPhyListener *listener)
{
  m_state->RegisterListener (listener);
}

bool
YansVlcPhy::IsStateCcaBusy (void)
{
  return m_state->IsStateCcaBusy ();
}

bool
YansVlcPhy::IsStateIdle (void)
{
  return m_state->IsStateIdle ();
}
bool
YansVlcPhy::IsStateBusy (void)
{
  return m_state->IsStateBusy ();
}
bool
YansVlcPhy::IsStateRx (void)
{
  return m_state->IsStateRx ();
}
bool
YansVlcPhy::IsStateTx (void)
{
  return m_state->IsStateTx ();
}
bool
YansVlcPhy::IsStateSwitching (void)
{
  return m_state->IsStateSwitching ();
}

Time
YansVlcPhy::GetStateDuration (void)
{
  return m_state->GetStateDuration ();
}
Time
YansVlcPhy::GetDelayUntilIdle (void)
{
  return m_state->GetDelayUntilIdle ();
}

Time
YansVlcPhy::GetLastRxStartTime (void) const
{
  return m_state->GetLastRxStartTime ();
}

double
YansVlcPhy::DbToRatio (double dB_vlc) const
{
  double ratio_vlc = std::pow (10.0, dB_vlc / 10.0);
  return ratio_vlc;
}

double
YansVlcPhy::DbmToW (double dBm_vlc) const
{
  double mW = std::pow (10.0, dBm_vlc / 10.0);
  return mW / 1000.0;
}

double
YansVlcPhy::WToDbm (double w_vlc) const
{
  return 10.0 * std::log10 (w_vlc * 1000.0);
}

double
YansVlcPhy::RatioToDb (double ratio_vlc) const
{
  return 10.0 * std::log10 (ratio_vlc);
}

double
YansVlcPhy::GetEdThresholdW (void) const
{
  return m_edThresholdW;
}

double
YansVlcPhy::GetPowerDbm (uint8_t power_vlc) const
{
  NS_ASSERT (m_txPowerBaseDbm <= m_txPowerEndDbm);
  NS_ASSERT (m_nTxPower > 0);
  double dbm_vlc;
  if (m_nTxPower > 1)
    {
      dbm_vlc = m_txPowerBaseDbm + power_vlc * (m_txPowerEndDbm - m_txPowerBaseDbm) / (m_nTxPower - 1);
    }
  else
    {
      NS_ASSERT_MSG (m_txPowerBaseDbm == m_txPowerEndDbm, "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
      dbm_vlc = m_txPowerBaseDbm;
    }
  return dbm_vlc;
}

void
YansVlcPhy::EndReceive (Ptr<Packet> packet_vlc, Ptr<InterferenceHelper::Event> event_vlc)
{
  NS_LOG_FUNCTION (this << packet_vlc << event_vlc);
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (event_vlc->GetEndTime () == Simulator::Now ());

  struct InterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculateSnrPer (event_vlc);
  m_interference.NotifyRxEnd ();

  NS_LOG_DEBUG ("mode=" << (event_vlc->GetPayloadMode ().GetDataRate ()) <<
                ", snr=" << snrPer.snr << ", per=" << snrPer.per << ", size=" << packet_vlc->GetSize ());
  if (m_random->GetValue () > snrPer.per)
    {
      NotifyRxEnd (packet_vlc);
      uint32_t dataRate500KbpsUnits = event_vlc->GetPayloadMode ().GetDataRate () * event_vlc->GetTxVector().GetNss()/ 500000;
      bool isShortPreamble = (WIFI_PREAMBLE_SHORT == event_vlc->GetPreambleType ());
      double signalDbm = RatioToDb (event_vlc->GetRxPowerW ()) + 30;
      double noiseDbm = RatioToDb (event_vlc->GetRxPowerW () / snrPer.snr) - GetRxNoiseFigure () + 30;
      NotifyMonitorSniffRx (packet_vlc, (uint16_t)GetChannelFrequencyMhz (), GetChannelNumber (), dataRate500KbpsUnits, isShortPreamble, signalDbm, noiseDbm);
      m_state->SwitchFromRxEndOk (packet_vlc, snrPer.snr, event_vlc->GetPayloadMode (), event_vlc->GetPreambleType ());
    }
  else
    {
      /* failure. */
      NotifyRxDrop (packet_vlc);
      m_state->SwitchFromRxEndError (packet_vlc, snrPer.snr);
    }
}

int64_t
YansVlcPhy::AssignStreams (int64_t stream_vlc)
{
  NS_LOG_FUNCTION (this << stream_vlc);
  m_random->SetStream (stream_vlc);
  return 1;
}

void
YansVlcPhy::SetFrequency (uint32_t freq_vlc)
{
  m_channelStartingFrequency = freq_vlc;
}

void
YansVlcPhy::SetNumberOfTransmitAntennas (uint32_t tx_vlc)
{
  m_numberOfTransmitters = tx_vlc;
}
void
YansVlcPhy::SetNumberOfReceiveAntennas (uint32_t rx_vlc)
{
  m_numberOfReceivers = rx_vlc;
}

void
YansVlcPhy::SetLdpc (bool Ldpc_vlc)
{
  m_ldpc = Ldpc_vlc;
}

void
YansVlcPhy::SetStbc (bool stbc_vlc)
{
  m_stbc = stbc_vlc;
}

void
YansVlcPhy::SetGreenfield (bool greenfield_vlc)
{
  m_greenfield = greenfield_vlc;
}
bool
YansVlcPhy::GetGuardInterval (void) const
{
  return m_guardInterval;
}
void
YansVlcPhy::SetGuardInterval (bool guardInterval_vlc)
{
  m_guardInterval = guardInterval_vlc;
}

uint32_t
YansVlcPhy::GetFrequency (void) const
{
  return m_channelStartingFrequency;
}

uint32_t
YansVlcPhy::GetNumberOfTransmitAntennas (void) const
{
  return m_numberOfTransmitters;
}
uint32_t
YansVlcPhy::GetNumberOfReceiveAntennas (void) const
{
  return m_numberOfReceivers;
}

bool
YansVlcPhy::GetLdpc (void) const
{
  return m_ldpc;
}
bool
YansVlcPhy::GetStbc (void) const
{
  return m_stbc;
}

bool
YansVlcPhy::GetGreenfield (void) const
{
  return m_greenfield;
}

bool
YansVlcPhy::GetChannelBonding(void) const
{
  return m_channelBonding;
}

void
YansVlcPhy::SetChannelBonding(bool channelbonding_vlc) 
{
  m_channelBonding= channelbonding_vlc;
}

void
YansVlcPhy::Configure80211n (void)
{
  NS_LOG_FUNCTION (this);
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate1Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate2Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate5_5Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate6Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetDsssRate11Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate12Mbps ());
  m_deviceRateSet.push_back (VlcPhy::GetErpOfdmRate24Mbps ());
  m_bssMembershipSelectorSet.push_back(HT_PHY);
  for (uint8_t i=0; i <8; i++)
    {
      m_deviceMcsSet.push_back(i);
    }

}
uint32_t
YansVlcPhy::GetNBssMembershipSelectors (void) const
{
  return  m_bssMembershipSelectorSet.size ();
}
uint32_t
YansVlcPhy::GetBssMembershipSelector (uint32_t selector_vlc) const
{
  return  m_bssMembershipSelectorSet[selector_vlc];
}
WifiModeList
YansVlcPhy::GetMembershipSelectorModes(uint32_t selector_vlc)
{
  uint32_t id=GetBssMembershipSelector(selector_vlc);
  WifiModeList supportedmodes;
  if (id == HT_PHY)
  {
    //mandatory MCS 0 to 7
     supportedmodes.push_back (VlcPhy::GetOfdmRate6_5MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate13MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate19_5MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate26MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate39MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate52MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate58_5MbpsBW20MHz ());
     supportedmodes.push_back (VlcPhy::GetOfdmRate65MbpsBW20MHz ());
  }
  return supportedmodes;
}
uint8_t
YansVlcPhy::GetNMcs (void) const
{
  return  m_deviceMcsSet.size ();
}
uint8_t
YansVlcPhy::GetMcs (uint8_t mcs_vlc) const
{
  return  m_deviceMcsSet[mcs_vlc];
}
uint32_t 
YansVlcPhy::WifiModeToMcs (WifiMode mode_vlc)
{
    uint32_t mcs_vlc = 0;
   if (mode_vlc.GetUniqueName() == "OfdmRate135MbpsBW40MHzShGi" || mode_vlc.GetUniqueName() == "OfdmRate65MbpsBW20MHzShGi" )
     {
             mcs_vlc=6;
     }
  else
    {
     switch (mode_vlc.GetDataRate())
       {
         case 6500000:
         case 7200000:
         case 13500000:
         case 15000000:
           mcs_vlc=0;
           break;
         case 13000000:
         case 14400000:
         case 27000000:
         case 30000000:
           mcs_vlc=1;
           break;
         case 19500000:
         case 21700000:
         case 40500000:
         case 45000000:
           mcs_vlc=2;
           break;
         case 26000000:
         case 28900000:
         case 54000000:
         case 60000000:
           mcs_vlc=3;
           break;
         case 39000000:
         case 43300000:
         case 81000000:
         case 90000000:        
           mcs_vlc=4;
           break;
         case 52000000:
         case 57800000:
         case 108000000:
         case 120000000:
           mcs_vlc=5;
           break; 
         case 58500000:
         case 121500000:
           mcs_vlc=6;
           break;
         case 65000000:
         case 72200000:
         case 135000000:
         case 150000000:
           mcs_vlc=7;
           break;     
       }
    }
  return mcs_vlc;
}
WifiMode
YansVlcPhy::McsToWifiMode (uint8_t mcs_vlc)
{
   WifiMode mode_vlc;
   switch (mcs_vlc)
     { 
       case 7:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc =  VlcPhy::GetOfdmRate65MbpsBW20MHz ();
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate72_2MbpsBW20MHz ();
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate135MbpsBW40MHz ();
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate150MbpsBW40MHz ();
            }
          break;
       case 6:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc = VlcPhy::GetOfdmRate58_5MbpsBW20MHz ();
 
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc =  VlcPhy::GetOfdmRate65MbpsBW20MHzShGi ();
       
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate121_5MbpsBW40MHz ();
     
            }
          else
            {
              mode_vlc= VlcPhy::GetOfdmRate135MbpsBW40MHzShGi ();
          
            }
          break;
       case 5:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc = VlcPhy::GetOfdmRate52MbpsBW20MHz ();
  
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate57_8MbpsBW20MHz ();
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate108MbpsBW40MHz ();
     
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate120MbpsBW40MHz ();
       
            }
          break;
       case 4:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc = VlcPhy::GetOfdmRate39MbpsBW20MHz ();
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate43_3MbpsBW20MHz ();
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate81MbpsBW40MHz ();
  
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate90MbpsBW40MHz ();
         
            }
          break;
       case 3:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc =  VlcPhy::GetOfdmRate26MbpsBW20MHz ();
  
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate28_9MbpsBW20MHz ();
      
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate54MbpsBW40MHz ();
     
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate60MbpsBW40MHz ();
            }
          break;
       case 2:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc = VlcPhy::GetOfdmRate19_5MbpsBW20MHz ();
 
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate21_7MbpsBW20MHz ();
     
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc =  VlcPhy::GetOfdmRate40_5MbpsBW40MHz ();
  
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate45MbpsBW40MHz ();
           
            }
          break;
       case 1:
          if (!GetGuardInterval() && !GetChannelBonding())
           {
            mode_vlc = VlcPhy::GetOfdmRate13MbpsBW20MHz ();
  
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc =  VlcPhy::GetOfdmRate14_4MbpsBW20MHz ();
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate27MbpsBW40MHz ();
     
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate30MbpsBW40MHz ();
            }
          break;
       case 0:
       default:
         if (!GetGuardInterval() && !GetChannelBonding())
           {
              mode_vlc = VlcPhy::GetOfdmRate6_5MbpsBW20MHz ();
              
            }
         else if(GetGuardInterval() && !GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate7_2MbpsBW20MHz ();
            }
          else if (!GetGuardInterval() && GetChannelBonding())
            {
              mode_vlc = VlcPhy::GetOfdmRate13_5MbpsBW40MHz ();
 
            }
          else
            {
              mode_vlc = VlcPhy::GetOfdmRate15MbpsBW40MHz ();
            }
         break;
        }
 return mode_vlc;
}
} // namespace ns3
