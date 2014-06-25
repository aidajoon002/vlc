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
#include "vlc-net-device.h"
#include "vlc-mac.h"
#include "vlc-phy.h"
#include "ns3/wifi-remote-station-manager.h"
#include "vlc-channel.h"
#include "ns3/llc-snap-header.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("vlcNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VlcNetDevice)
  ;

TypeId
VlcNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<VlcNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                   MakeUintegerAccessor (&VlcNetDevice::SetMtu,
                                         &VlcNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (1,MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::DoGetChannel),
                   MakePointerChecker<VlcChannel> ())
    .AddAttribute ("Phy", "The PHY layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::GetPhy,
                                        &VlcNetDevice::SetPhy),
                   MakePointerChecker<WifiPhy> ())
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::GetMac,
                                        &VlcNetDevice::SetMac),
                   MakePointerChecker<VlcMac> ())
    .AddAttribute ("RemoteStationManager", "The station manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::SetRemoteStationManager,
                                        &VlcNetDevice::GetRemoteStationManager),
                   MakePointerChecker<WifiRemoteStationManager> ())
  ;
  return tid;
}

VlcNetDevice::VlcNetDevice ()
  : m_configComplete (false)
{
  NS_LOG_FUNCTION_NOARGS ();
}
VlcNetDevice::~VlcNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
VlcNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  m_mac->Dispose ();
  m_phy->Dispose ();
  m_stationManager->Dispose ();
  m_mac = 0;
  m_phy = 0;
  m_stationManager = 0;
  // chain up.
  NetDevice::DoDispose ();
}

void
VlcNetDevice::DoInitialize (void)
{
  m_phy->Initialize ();
  m_mac->Initialize ();
  m_stationManager->Initialize ();
  NetDevice::DoInitialize ();
}

void
VlcNetDevice::CompleteConfig (void)
{
  if (m_mac == 0
      || m_phy == 0
      || m_stationManager == 0
      || m_node == 0
      || m_configComplete)
    {
      return;
    }
  m_mac->SetWifiRemoteStationManager (m_stationManager);
  m_mac->SetVlcPhy (m_phy);
  m_mac->SetForwardUpCallback (MakeCallback (&VlcNetDevice::ForwardUp, this));
  m_mac->SetLinkUpCallback (MakeCallback (&VlcNetDevice::LinkUp, this));
  m_mac->SetLinkDownCallback (MakeCallback (&VlcNetDevice::LinkDown, this));
  m_stationManager->SetupPhy (m_phy);
  m_configComplete = true;
}

void
VlcNetDevice::SetMac (Ptr<VlcMac> mac)
{
  m_mac = mac;
  CompleteConfig ();
}
void
VlcNetDevice::SetPhy (Ptr<VlcPhy> phy)
{
  m_phy = phy;
  CompleteConfig ();
}
void
VlcNetDevice::SetRemoteStationManager (Ptr<WifiRemoteStationManager> manager)
{
  m_stationManager = manager;
  CompleteConfig ();
}
Ptr<VlcMac>
VlcNetDevice::GetMac (void) const
{
  return m_mac;
}
Ptr<VlcPhy>
VlcNetDevice::GetPhy (void) const
{
  return m_phy;
}
Ptr<WifiRemoteStationManager>
VlcNetDevice::GetRemoteStationManager (void) const
{
  return m_stationManager;
}

void
VlcNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}
uint32_t
VlcNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}
Ptr<Channel>
VlcNetDevice::GetChannel (void) const
{
  return m_phy->GetChannel ();
}
Ptr<VlcChannel>
VlcNetDevice::DoGetChannel (void) const
{
  return m_phy->GetChannel ();
}
void
VlcNetDevice::SetAddress (Address address)
{
  m_mac->SetAddress (Mac48Address::ConvertFrom (address));
}
Address
VlcNetDevice::GetAddress (void) const
{
  return m_mac->GetAddress ();
}
bool
VlcNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}
uint16_t
VlcNetDevice::GetMtu (void) const
{
  return m_mtu;
}
bool
VlcNetDevice::IsLinkUp (void) const
{
  return m_phy != 0 && m_linkUp;
}
void
VlcNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  m_linkChanges.ConnectWithoutContext (callback);
}
bool
VlcNetDevice::IsBroadcast (void) const
{
  return true;
}
Address
VlcNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}
bool
VlcNetDevice::IsMulticast (void) const
{
  return true;
}
Address
VlcNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}
Address VlcNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}
bool
VlcNetDevice::IsPointToPoint (void) const
{
  return false;
}
bool
VlcNetDevice::IsBridge (void) const
{
  return false;
}
bool
VlcNetDevice::Send (Ptr<Packet> packet_vlc, const Address& dest, uint16_t protocolNumber)
{
  NS_ASSERT (Mac48Address::IsMatchingType (dest));

  Mac48Address realTo = Mac48Address::ConvertFrom (dest);

  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet_vlc->AddHeader (llc);

  m_mac->NotifyTx (packet_vlc);
  m_mac->Enqueue (packet_vlc, realTo);
  return true;
}
Ptr<Node>
VlcNetDevice::GetNode (void) const
{
  return m_node;
}
void
VlcNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
  CompleteConfig ();
}
bool
VlcNetDevice::NeedsArp (void) const
{
  return true;
}
void
VlcNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

void
VlcNetDevice::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  enum NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == m_mac->GetAddress ())
    {
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      m_mac->NotifyRx (packet);
      m_forwardUp (this, packet, llc.GetType (), from);
    }

  if (!m_promiscRx.IsNull ())
    {
      m_mac->NotifyPromiscRx (packet);
      m_promiscRx (this, packet, llc.GetType (), from, to, type);
    }
}

void
VlcNetDevice::LinkUp (void)
{
  m_linkUp = true;
  m_linkChanges ();
}
void
VlcNetDevice::LinkDown (void)
{
  m_linkUp = false;
  m_linkChanges ();
}

bool
VlcNetDevice::SendFrom (Ptr<Packet> packet_vlc, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_ASSERT (Mac48Address::IsMatchingType (dest));
  NS_ASSERT (Mac48Address::IsMatchingType (source));

  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
  Mac48Address realFrom = Mac48Address::ConvertFrom (source);

  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet_vlc->AddHeader (llc);

  m_mac->NotifyTx (packet_vlc);
  m_mac->Enqueue (packet_vlc, realTo, realFrom);

  return true;
}

void
VlcNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
  m_mac->SetPromisc();
}

bool
VlcNetDevice::SupportsSendFrom (void) const
{
  return m_mac->SupportsSendFrom ();
}

} // namespace ns3

