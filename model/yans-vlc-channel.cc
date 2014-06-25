/* -*- Mopyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "yans-vlc-channel.h"
#include "yans-vlc-phy.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

NS_LOG_COMPONENT_DEFINE ("YansVlcChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (YansVlcChannel)
  ;

TypeId
YansVlcChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YansVlcChannel")
    .SetParent<VlcChannel> ()
    .AddConstructor<YansVlcChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansVlcChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansVlcChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
    ;
  return tid;
}

YansVlcChannel::YansVlcChannel ()
{
}
YansVlcChannel::~YansVlcChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_phyList.clear ();
}

void
YansVlcChannel::SetPropagationLossModel (Ptr<PropagationLossModel> loss_vlc)
{
  m_loss = loss_vlc;
}
void
YansVlcChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay_vlc)
{
  m_delay = delay_vlc;
}

void
YansVlcChannel::Send (Ptr<YansVlcPhy> sender_vlc, Ptr<const Packet> packet_vlc, double txPowerDbm_vlc,
                       WifiTxVector txVector_vlc, WifiPreamble preamble_vlc) const
{
  Ptr<MobilityModel> sender_vlcMobility = sender_vlc->GetMobility ()->GetObject<MobilityModel> ();
  NS_ASSERT (sender_vlcMobility != 0);
  uint32_t j_vlc = 0;
  for (PhyList::const_iterator i_vlc = m_phyList.begin (); i_vlc != m_phyList.end (); i_vlc++, j_vlc++)
    {
      if (sender_vlc != (*i_vlc))
        {
          // For now don't account for inter channel interference
          if ((*i_vlc)->GetChannelNumber () != sender_vlc->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiver_vlcMobility = (*i_vlc)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (sender_vlcMobility, receiver_vlcMobility);
          double rxPowerDbm_vlc = m_loss->CalcRxPower (txPowerDbm_vlc, sender_vlcMobility, receiver_vlcMobility);
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm_vlc << "dbm, rxPower=" << rxPowerDbm_vlc << "dbm, " <<
                        "distance=" << sender_vlcMobility->GetDistanceFrom (receiver_vlcMobility) << "m, delay=" << delay);
          Ptr<Packet> copy = packet_vlc->Copy ();
          Ptr<Object> dstNetDevice = m_phyList[j_vlc]->GetDevice ();
          uint32_t dstNode;
 if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
            }
          Simulator::ScheduleWithContext (dstNode,
                                          delay, &YansVlcChannel::Receive, this,
                                          j_vlc, copy, rxPowerDbm_vlc, txVector_vlc, preamble_vlc);
        }
    }
}

void
YansVlcChannel::Receive (uint32_t i_vlc, Ptr<Packet> packet_vlc, double rxPowerDbm_vlc,
                          WifiTxVector txVector_vlc, WifiPreamble preamble_vlc) const
{
  m_phyList[i_vlc]->StartReceivePacket (packet_vlc, rxPowerDbm_vlc, txVector_vlc, preamble_vlc);
}

uint32_t
YansVlcChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}
Ptr<NetDevice>
YansVlcChannel::GetDevice (uint32_t i_vlc) const
{
  return m_phyList[i_vlc]->GetDevice ()->GetObject<NetDevice> ();
}

void
YansVlcChannel::Add (Ptr<YansVlcPhy> phy)
{
  m_phyList.push_back (phy);
}

int64_t
YansVlcChannel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}
}
 // namespace ns3

