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
 */
#include "vlc-phy-state-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"

NS_LOG_COMPONENT_DEFINE ("VlcPhyStateHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VlcPhyStateHelper)
  ;

TypeId
VlcPhyStateHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcPhyStateHelper")
    .SetParent<Object> ()
    .AddConstructor<VlcPhyStateHelper> ()
    .AddTraceSource ("State",
                     "The state of the PHY layer",
                     MakeTraceSourceAccessor (&VlcPhyStateHelper::m_stateLogger))
    .AddTraceSource ("RxOk",
                     "A packet has been received successfully.",
                     MakeTraceSourceAccessor (&VlcPhyStateHelper::m_rxOkTrace))
    .AddTraceSource ("RxError",
                     "A packet has been received unsuccessfully.",
                     MakeTraceSourceAccessor (&VlcPhyStateHelper::m_rxErrorTrace))
    .AddTraceSource ("Tx", "Packet transmission is starting.",
                     MakeTraceSourceAccessor (&VlcPhyStateHelper::m_txTrace))
  ;
  return tid;
}

VlcPhyStateHelper::VlcPhyStateHelper ()
  : m_rxing (false),
    m_endTx (Seconds (0)),
    m_endRx (Seconds (0)),
    m_endCcaBusy (Seconds (0)),
    m_endSwitching (Seconds (0)),
    m_startTx (Seconds (0)),
    m_startRx (Seconds (0)),
    m_startCcaBusy (Seconds (0)),
    m_startSwitching (Seconds (0)),
    m_previousStateChangeTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
}

void
VlcPhyStateHelper::SetReceiveOkCallback (VlcPhy::RxOkCallback callback)
{
  m_rxOkCallback = callback;
}
void
VlcPhyStateHelper::SetReceiveErrorCallback (VlcPhy::RxErrorCallback callback)
{
  m_rxErrorCallback = callback;
}
void
VlcPhyStateHelper::RegisterListener (WifiPhyListener *listener)
{
  m_listeners.push_back (listener);
}

bool
VlcPhyStateHelper::IsStateIdle (void)
{
  return (GetState () == VlcPhy::IDLE);
}
bool
VlcPhyStateHelper::IsStateBusy (void)
{
  return (GetState () != VlcPhy::IDLE);
}
bool
VlcPhyStateHelper::IsStateCcaBusy (void)
{
  return (GetState () == VlcPhy::CCA_BUSY);
}
bool
VlcPhyStateHelper::IsStateRx (void)
{
  return (GetState () == VlcPhy::RX);
}
bool
VlcPhyStateHelper::IsStateTx (void)
{
  return (GetState () == VlcPhy::TX);
}
bool
VlcPhyStateHelper::IsStateSwitching (void)
{
  return (GetState () == VlcPhy::SWITCHING);
}



Time
VlcPhyStateHelper::GetStateDuration (void)
{
  return Simulator::Now () - m_previousStateChangeTime;
}

Time
VlcPhyStateHelper::GetDelayUntilIdle (void)
{
  Time retval;

  switch (GetState ())
    {
    case VlcPhy::RX:
      retval = m_endRx - Simulator::Now ();
      break;
    case VlcPhy::TX:
      retval = m_endTx - Simulator::Now ();
      break;
    case VlcPhy::CCA_BUSY:
      retval = m_endCcaBusy - Simulator::Now ();
      break;
    case VlcPhy::SWITCHING:
      retval = m_endSwitching - Simulator::Now ();
      break;
    case VlcPhy::IDLE:
      retval = Seconds (0);
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      retval = Seconds (0);
      break;
    }
  retval = Max (retval, Seconds (0));
  return retval;
}

Time
VlcPhyStateHelper::GetLastRxStartTime (void) const
{
  return m_startRx;
}

enum VlcPhy::State
VlcPhyStateHelper::GetState (void)
{
  if (m_endTx > Simulator::Now ())
    {
      return VlcPhy::TX;
    }
  else if (m_rxing)
    {
      return VlcPhy::RX;
    }
  else if (m_endSwitching > Simulator::Now ())
    {
      return VlcPhy::SWITCHING;
    }
  else if (m_endCcaBusy > Simulator::Now ())
    {
      return VlcPhy::CCA_BUSY;
    }
  else
    {
      return VlcPhy::IDLE;
    }
}


void
VlcPhyStateHelper::NotifyTxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (duration);
    }
}
void
VlcPhyStateHelper::NotifyRxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart (duration);
    }
}
void
VlcPhyStateHelper::NotifyRxEndOk (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndOk ();
    }
}
void
VlcPhyStateHelper::NotifyRxEndError (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndError ();
    }
}
void
VlcPhyStateHelper::NotifyMaybeCcaBusyStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyMaybeCcaBusyStart (duration);
    }
}
void
VlcPhyStateHelper::NotifySwitchingStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifySwitchingStart (duration);
    }
}

void
VlcPhyStateHelper::LogPreviousIdleAndCcaBusyStates (void)
{
  Time now = Simulator::Now ();
  Time idleStart = Max (m_endCcaBusy, m_endRx);
  idleStart = Max (idleStart, m_endTx);
  idleStart = Max (idleStart, m_endSwitching);
  NS_ASSERT (idleStart <= now);
  if (m_endCcaBusy > m_endRx
      && m_endCcaBusy > m_endSwitching
      && m_endCcaBusy > m_endTx)
    {
      Time ccaBusyStart = Max (m_endTx, m_endRx);
      ccaBusyStart = Max (ccaBusyStart, m_startCcaBusy);
      ccaBusyStart = Max (ccaBusyStart, m_endSwitching);
      m_stateLogger (ccaBusyStart, idleStart - ccaBusyStart, VlcPhy::CCA_BUSY);
    }
  m_stateLogger (idleStart, now - idleStart, VlcPhy::IDLE);
}

void
VlcPhyStateHelper::SwitchToTx (Time txDuration_vlc, Ptr<const Packet> packet_vlc, WifiMode txMode_vlc,
                                WifiPreamble preamble_vlc, uint8_t txPower_vlc)
{
  m_txTrace (packet_vlc, txMode_vlc, preamble_vlc, txPower_vlc);
  NotifyTxStart (txDuration_vlc);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case VlcPhy::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_rxing = false;
      m_stateLogger (m_startRx, now - m_startRx, VlcPhy::RX);
      m_endRx = now;
      break;
    case VlcPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, VlcPhy::CCA_BUSY);
      } break;
    case VlcPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case VlcPhy::SWITCHING:
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_stateLogger (now, txDuration_vlc, VlcPhy::TX);
  m_previousStateChangeTime = now;
  m_endTx = now + txDuration_vlc;
  m_startTx = now;
}
void
VlcPhyStateHelper::SwitchToRx (Time rxDuration)
{
  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
  NS_ASSERT (!m_rxing);
  NotifyRxStart (rxDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case VlcPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case VlcPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, VlcPhy::CCA_BUSY);
      } break;
    case VlcPhy::SWITCHING:
    case VlcPhy::RX:
    case VlcPhy::TX:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_previousStateChangeTime = now;
  m_rxing = true;
  m_startRx = now;
  m_endRx = now + rxDuration;
  NS_ASSERT (IsStateRx ());
}

void
VlcPhyStateHelper::SwitchToChannelSwitching (Time switchingDuration)
{
  NotifySwitchingStart (switchingDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case VlcPhy::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_rxing = false;
      m_stateLogger (m_startRx, now - m_startRx, VlcPhy::RX);
      m_endRx = now;
      break;
    case VlcPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, VlcPhy::CCA_BUSY);
      } break;
    case VlcPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case VlcPhy::TX:
    case VlcPhy::SWITCHING:
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }

  if (now < m_endCcaBusy)
    {
      m_endCcaBusy = now;
    }

  m_stateLogger (now, switchingDuration, VlcPhy::SWITCHING);
  m_previousStateChangeTime = now;
  m_startSwitching = now;
  m_endSwitching = now + switchingDuration;
  NS_ASSERT (IsStateSwitching ());
}

void
VlcPhyStateHelper::SwitchFromRxEndOk (Ptr<Packet> packet_vlc, double snr_vlc, WifiMode mode_vlc, enum WifiPreamble preamble_vlc)
{
  m_rxOkTrace (packet_vlc, snr_vlc, mode_vlc, preamble_vlc);
  NotifyRxEndOk ();
  DoSwitchFromRx ();
  if (!m_rxOkCallback.IsNull ())
    {
      m_rxOkCallback (packet_vlc, snr_vlc, mode_vlc, preamble_vlc);
    }

}
void
VlcPhyStateHelper::SwitchFromRxEndError (Ptr<const Packet> packet_vlc, double snr_vlc)
{
  m_rxErrorTrace (packet_vlc, snr_vlc);
  NotifyRxEndError ();
  DoSwitchFromRx ();
  if (!m_rxErrorCallback.IsNull ())
    {
      m_rxErrorCallback (packet_vlc, snr_vlc);
    }
}

void
VlcPhyStateHelper::DoSwitchFromRx (void)
{
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_rxing);

  Time now = Simulator::Now ();
  m_stateLogger (m_startRx, now - m_startRx, VlcPhy::RX);
  m_previousStateChangeTime = now;
  m_rxing = false;

  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
}
void
VlcPhyStateHelper::SwitchMaybeToCcaBusy (Time duration)
{
  NotifyMaybeCcaBusyStart (duration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case VlcPhy::SWITCHING:
      break;
    case VlcPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case VlcPhy::CCA_BUSY:
      break;
    case VlcPhy::RX:
      break;
    case VlcPhy::TX:
      break;
    }
  if (GetState () != VlcPhy::CCA_BUSY)
    {
      m_startCcaBusy = now;
    }
  m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
}

} // namespace ns3
