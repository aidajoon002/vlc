

#include "vlc-phy.h"
#include "ns3/wifi-mode.h"
#include "vlc-channel.h"
#include "ns3/wifi-preamble.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/trace-source-accessor.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("vlcPhy");

namespace ns3 {

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/

WifiPhyListener::~WifiPhyListener ()
{
}

/****************************************************************
 *       The actual WifiPhy class
 ****************************************************************/

NS_OBJECT_ENSURE_REGISTERED (VlcPhy)
  ;

TypeId
VlcPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::vlcPhy")
    .SetParent<Object> ()
    .AddTraceSource ("PhyTxBegin",
                     "Trace source indicating a packet has begun transmitting over the channel medium",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyTxBeginTrace))
    .AddTraceSource ("PhyTxEnd",
                     "Trace source indicating a packet has been completely transmitted over the channel. NOTE: the only official WifiPhy implementation available to this date (YansWifiPhy) never fires this trace source.",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyTxEndTrace))
    .AddTraceSource ("PhyTxDrop",
                     "Trace source indicating a packet has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyTxDropTrace))
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet has begun being received from the channel medium by the device",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyRxBeginTrace))
    .AddTraceSource ("PhyRxEnd",
                     "Trace source indicating a packet has been completely received from the channel medium by the device",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyRxEndTrace))
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyRxDropTrace))
    .AddTraceSource ("MonitorSnifferRx",
                     "Trace source simulating a wifi device in monitor mode sniffing all received frames",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyMonitorSniffRxTrace))
    .AddTraceSource ("MonitorSnifferTx",
                     "Trace source simulating the capability of a wifi device in monitor mode to sniff all frames being transmitted",
                     MakeTraceSourceAccessor (&VlcPhy::m_phyMonitorSniffTxTrace))
  ;
  return tid;
}

VlcPhy::VlcPhy ()
{
  NS_LOG_FUNCTION (this);
}

VlcPhy::~VlcPhy ()
{
  NS_LOG_FUNCTION (this);
}

//Added by Ghada to support 11n

//return the L-SIG
WifiMode
VlcPhy::GetMFPlcpHeaderMode (WifiMode payloadMode_vlc, WifiPreamble preamble_vlc)
{
    switch (payloadMode_vlc.GetBandwidth ())
       {
       case 20000000:
          return VlcPhy::GetOfdmRate6_5MbpsBW20MHz ();
        case 40000000:
           return VlcPhy::GetOfdmRate13_5MbpsBW40MHz ();
        default:
            return VlcPhy::GetOfdmRate6_5MbpsBW20MHz ();
      }
}
uint32_t
VlcPhy::GetPlcpHtTrainingSymbolDurationMicroSeconds (WifiMode payloadMode_vlc, WifiPreamble preamble_vlc, WifiTxVector txvector_vlc)
{
   switch (preamble_vlc)
     {
     case WIFI_PREAMBLE_HT_MF:
        return 4+ (4* txvector_vlc.GetNss());
     case WIFI_PREAMBLE_HT_GF:
         return (4*txvector_vlc.GetNss())+(4*txvector_vlc.GetNess());
      default:
         // no training for non HT
          return 0;
      }
}

//return L-SIG
uint32_t
VlcPhy::GetPlcpHtSigHeaderDurationMicroSeconds (WifiMode payloadMode_vlc, WifiPreamble preamble_vlc)
{
         switch (preamble_vlc)
            {
             case WIFI_PREAMBLE_HT_MF:
               // HT-SIG
               return 8;
             case WIFI_PREAMBLE_HT_GF:
               //HT-SIG
               return 8;
             default:
               // no HT-SIG for non HT
               return 0;
            }

}
//end added by Ghada

WifiMode
VlcPhy::GetPlcpHeaderMode (WifiMode payloadMode_vlc, WifiPreamble preamble_vlc)
{
  switch (payloadMode_vlc.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (payloadMode_vlc.GetBandwidth ())
          {
          case 5000000:
            return VlcPhy::GetOfdmRate1_5MbpsBW5MHz ();
          case 10000000:
            return VlcPhy::GetOfdmRate3MbpsBW10MHz ();
          default:
            // IEEE Std 802.11-2007, 17.3.2
            // actually this is only the first part of the PlcpHeader,
            // because the last 16 bits of the PlcpHeader are using the
            // same mode of the payload
            return VlcPhy::GetOfdmRate6Mbps ();
          }
      }
    //Added by Ghada to support 11n
    case WIFI_MOD_CLASS_HT:
      {  //return the HT-SIG
         // IEEE Std 802.11n, 20.3.23
         switch (preamble_vlc)
           {
            case WIFI_PREAMBLE_HT_MF:
                switch (payloadMode_vlc.GetBandwidth ())
                  {
                   case 20000000:
                      return VlcPhy::GetOfdmRate13MbpsBW20MHz ();
                   case 40000000:
                      return VlcPhy::GetOfdmRate27MbpsBW40MHz ();
                   default:
                      return VlcPhy::GetOfdmRate13MbpsBW20MHz ();
                  }
            case WIFI_PREAMBLE_HT_GF:
                  switch (payloadMode_vlc.GetBandwidth ())
                  {
                   case 20000000:
                      return VlcPhy::GetOfdmRate13MbpsBW20MHz ();
                   case 40000000:
                      return VlcPhy::GetOfdmRate27MbpsBW40MHz ();
                   default:
                      return VlcPhy::GetOfdmRate13MbpsBW20MHz ();
                  }
             default:
                return VlcPhy::GetOfdmRate6Mbps ();
          }
      }
    case WIFI_MOD_CLASS_ERP_OFDM:
      return VlcPhy::GetErpOfdmRate6Mbps ();

    case WIFI_MOD_CLASS_DSSS:
      if (preamble_vlc == WIFI_PREAMBLE_LONG)
        {
          // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
          return VlcPhy::GetDsssRate1Mbps ();
        }
      else  //  WIFI_PREAMBLE_SHORT
        {
          // IEEE Std 802.11-2007, section 18.2.2.2
          return VlcPhy::GetDsssRate2Mbps ();
        }

    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return WifiMode ();
    }
}


uint32_t
VlcPhy::GetPlcpHeaderDurationMicroSeconds (WifiMode payloadMode_vlc, WifiPreamble preamble_vlc)
{
  switch (payloadMode_vlc.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (payloadMode_vlc.GetBandwidth ())
          {
          case 20000000:
          default:
            // IEEE Std 802.11-2007, section 17.3.3 and figure 17-4
            // also section 17.3.2.3, table 17-4
            // We return the duration of the SIGNAL field only, since the
            // SERVICE field (which strictly speaking belongs to the PLCP
            // header, see section 17.3.2 and figure 17-1) is sent using the
            // payload mode.
            return 4;
          case 10000000:
            // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
            return 8;
          case 5000000:
            // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
            return 16;
          }
      }
     //Added by Ghada to support 11n
    case WIFI_MOD_CLASS_HT:
      { //IEEE 802.11n Figure 20.1
         switch (preamble_vlc)
            {
             case WIFI_PREAMBLE_HT_MF:
               // L-SIG
               return 4;
             case WIFI_PREAMBLE_HT_GF:
               //L-SIG
               return 0;
             default:
               // L-SIG
               return 4;
            }
      }
    case WIFI_MOD_CLASS_ERP_OFDM:
      return 4;

    case WIFI_MOD_CLASS_DSSS:
      if (preamble_vlc == WIFI_PREAMBLE_SHORT)
        {
          // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
          return 24;
        }
      else // WIFI_PREAMBLE_LONG
        {
          // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
          return 48;
        }

    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}

uint32_t
VlcPhy::GetPlcpPreambleDurationMicroSeconds (WifiMode payloadMode_vlc, WifiPreamble preamble_vlc)
{
  switch (payloadMode_vlc.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (payloadMode_vlc.GetBandwidth ())
          {
          case 20000000:
          default:
            // IEEE Std 802.11-2007, section 17.3.3,  figure 17-4
            // also section 17.3.2.3, table 17-4
            return 16;
          case 10000000:
            // IEEE Std 802.11-2007, section 17.3.3, table 17-4
            // also section 17.3.2.3, table 17-4
            return 32;
          case 5000000:
            // IEEE Std 802.11-2007, section 17.3.3
            // also section 17.3.2.3, table 17-4
            return 64;
          }
      }
    case WIFI_MOD_CLASS_HT:
      { //IEEE 802.11n Figure 20.1 the training symbols before L_SIG or HT_SIG
           return 16;
      }
    case WIFI_MOD_CLASS_ERP_OFDM:
      return 16;

    case WIFI_MOD_CLASS_DSSS:
      if (preamble_vlc == WIFI_PREAMBLE_SHORT)
        {
          // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
          return 72;
        }
      else // WIFI_PREAMBLE_LONG
        {
          // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
          return 144;
        }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}

double
VlcPhy::GetPayloadDurationMicroSeconds (uint32_t size_vlc, WifiTxVector txvector_vlc)
{
  WifiMode payloadMode_vlc=txvector_vlc.GetMode();

  NS_LOG_FUNCTION (size_vlc << payloadMode_vlc);

  switch (payloadMode_vlc.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
        // corresponds to T_{SYM} in the table
        uint32_t symbolDurationUs;

        switch (payloadMode_vlc.GetBandwidth ())
          {
          case 20000000:
          default:
            symbolDurationUs = 4;
            break;
          case 10000000:
            symbolDurationUs = 8;
            break;
          case 5000000:
            symbolDurationUs = 16;
            break;
          }

        // IEEE Std 802.11-2007, section 17.3.2.2, table 17-3
        // corresponds to N_{DBPS} in the table
        double numDataBitsPerSymbol = payloadMode_vlc.GetDataRate () * symbolDurationUs / 1e6;

        // IEEE Std 802.11-2007, section 17.3.5.3, equation (17-11)
        uint32_t numSymbols = lrint (ceil ((16 + size_vlc * 8.0 + 6.0) / numDataBitsPerSymbol));

        // Add signal extension for ERP PHY
        if (payloadMode_vlc.GetModulationClass () == WIFI_MOD_CLASS_ERP_OFDM)
          {
            return numSymbols * symbolDurationUs + 6;
          }
        else
          {
            return numSymbols * symbolDurationUs;
          }
      }
    case WIFI_MOD_CLASS_HT:
      {
         double symbolDurationUs;
         double m_Stbc;
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and reciever support GI
         if (payloadMode_vlc.GetUniqueName() == "OfdmRate135MbpsBW40MHzShGi" || payloadMode_vlc.GetUniqueName() == "OfdmRate65MbpsBW20MHzShGi" )
           {
             symbolDurationUs=3.6;
           }
         else
           {
             switch (payloadMode_vlc.GetDataRate ()/ (txvector_vlc.GetNss()))
                  { //shortGi
                  case 7200000:
                  case 14400000:
                  case 21700000:
                  case 28900000:
                  case 43300000:
                  case 57800000:
                  case 72200000:
                  case 15000000:
                  case 30000000:
                  case 45000000:
                  case 60000000:
                  case 90000000:
                  case 120000000:
                  case 150000000:
                    symbolDurationUs=3.6;
                    break;
                 default:
                    symbolDurationUs=4;
              }
           }
         if  (txvector_vlc.IsStbc())
            m_Stbc=2;
         else
           m_Stbc=1;
         double numDataBitsPerSymbol = payloadMode_vlc.GetDataRate () *txvector_vlc.GetNss()  * symbolDurationUs / 1e6;
         //check tables 20-35 and 20-36 in the standard to get cases when nes =2
         double Nes=1;
        // IEEE Std 802.11n, section 20.3.11, equation (20-32)
        uint32_t numSymbols = lrint (m_Stbc*ceil ((16 + size_vlc * 8.0 + 6.0*Nes) / (m_Stbc* numDataBitsPerSymbol)));

        return numSymbols * symbolDurationUs;

      }
    case WIFI_MOD_CLASS_DSSS:
      // IEEE Std 802.11-2007, section 18.2.3.5
      NS_LOG_LOGIC (" size=" << size_vlc
                             << " mode=" << payloadMode_vlc
                             << " rate=" << payloadMode_vlc.GetDataRate () );
      return lrint (ceil ((size_vlc * 8.0) / (payloadMode_vlc.GetDataRate () / 1.0e6)));

    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}
Time
VlcPhy::CalculateTxDuration (uint32_t size_vlc, WifiTxVector txvector_vlc, WifiPreamble preamble_vlc)
{
  WifiMode payloadMode_vlc=txvector_vlc.GetMode();
  double duration = GetPlcpPreambleDurationMicroSeconds (payloadMode_vlc, preamble_vlc)
    + GetPlcpHeaderDurationMicroSeconds (payloadMode_vlc, preamble_vlc)
    + GetPlcpHtSigHeaderDurationMicroSeconds (payloadMode_vlc, preamble_vlc)
    + GetPlcpHtTrainingSymbolDurationMicroSeconds (payloadMode_vlc, preamble_vlc,txvector_vlc)
    + GetPayloadDurationMicroSeconds (size_vlc, txvector_vlc);
  return MicroSeconds (duration);
}



void
VlcPhy::NotifyTxBegin (Ptr<const Packet> packet_vlc)
{
  m_phyTxBeginTrace (packet_vlc);
}

void
VlcPhy::NotifyTxEnd (Ptr<const Packet> packet_vlc)
{
  m_phyTxEndTrace (packet_vlc);
}

void
VlcPhy::NotifyTxDrop (Ptr<const Packet> packet_vlc)
{
  m_phyTxDropTrace (packet_vlc);
}

void
VlcPhy::NotifyRxBegin (Ptr<const Packet> packet_vlc)
{
  m_phyRxBeginTrace (packet_vlc);
}

void
VlcPhy::NotifyRxEnd (Ptr<const Packet> packet_vlc)
{
  m_phyRxEndTrace (packet_vlc);
}

void
VlcPhy::NotifyRxDrop (Ptr<const Packet> packet_vlc)
{
  m_phyRxDropTrace (packet_vlc);
}

void
VlcPhy::NotifyMonitorSniffRx (Ptr<const Packet> packet_vlc, uint16_t channelFreqMhz_vlc, uint16_t channelNumber_vlc, uint32_t rate_vlc, bool isShortPreamble, double signalDbm, double noiseDbm)
{
  m_phyMonitorSniffRxTrace (packet_vlc, channelFreqMhz_vlc, channelNumber_vlc, rate_vlc, isShortPreamble, signalDbm, noiseDbm);
}

void
VlcPhy::NotifyMonitorSniffTx (Ptr<const Packet> packet_vlc, uint16_t channelFreqMhz_vlc, uint16_t channelNumber_vlc, uint32_t rate_vlc, bool isShortPreamble, uint8_t txPower_vlc)
{
  m_phyMonitorSniffTxTrace (packet_vlc, channelFreqMhz_vlc, channelNumber_vlc, rate_vlc, isShortPreamble, txPower_vlc);
}


// Clause 15 rates (DSSS)

WifiMode
VlcPhy::GetDsssRate1Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("DsssRate1Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     22000000, 1000000,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetDsssRate2Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("DsssRate2Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     22000000, 2000000,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     4);
  return mode_vlc;
}


// Clause 18 rates (HR/DSSS)

WifiMode
VlcPhy::GetDsssRate5_5Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("DsssRate5_5Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     22000000, 5500000,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetDsssRate11Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("DsssRate11Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     22000000, 11000000,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     4);
  return mode_vlc;
}


// Clause 19.5 rates (ERP-OFDM)

WifiMode
VlcPhy::GetErpOfdmRate6Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate6Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     20000000, 6000000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate9Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate9Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     20000000, 9000000,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate12Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate12Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     20000000, 12000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate18Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate18Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     20000000, 18000000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate24Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate24Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     20000000, 24000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate36Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate36Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     20000000, 36000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate48Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate48Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     20000000, 48000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetErpOfdmRate54Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate54Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     20000000, 54000000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}


// Clause 17 rates (OFDM)

WifiMode
VlcPhy::GetOfdmRate6Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate6Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     20000000, 6000000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate9Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate9Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     20000000, 9000000,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate12Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate12Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     20000000, 12000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate18Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate18Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     20000000, 18000000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate24Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate24Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     20000000, 24000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate36Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate36Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     20000000, 36000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate48Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate48Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     20000000, 48000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate54Mbps ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate54Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     20000000, 54000000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}

// 10 MHz channel rates

WifiMode
VlcPhy::GetOfdmRate3MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     10000000, 3000000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate4_5MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     10000000, 4500000,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate6MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     10000000, 6000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate9MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     10000000, 9000000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate12MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     10000000, 12000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate18MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate18MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     10000000, 18000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate24MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate24MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     10000000, 24000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate27MbpsBW10MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate27MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     10000000, 27000000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}

// 5 MHz channel rates

WifiMode
VlcPhy::GetOfdmRate1_5MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     5000000, 1500000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate2_25MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_25MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     5000000, 2250000,
                                    WIFI_CODE_RATE_3_4,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate3MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     5000000, 3000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate4_5MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     5000000, 4500000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate6MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     5000000, 6000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate9MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     5000000, 9000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate12MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     5000000, 12000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate13_5MbpsBW5MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     5000000, 13500000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}

// Clause 20

WifiMode
VlcPhy::GetOfdmRate6_5MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
  WifiModeFactory::CreateWifiMode ("OfdmRate6_5MbpsBW20MHz",
                                    WIFI_MOD_CLASS_HT,
                                    true,
                                    20000000, 6500000,
                                    WIFI_CODE_RATE_1_2,
                                    2);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate7_2MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
  WifiModeFactory::CreateWifiMode ("OfdmRate7_2MbpsBW20MHz",
                                    WIFI_MOD_CLASS_HT,
                                    false,
                                    20000000, 7200000,
                                    WIFI_CODE_RATE_1_2,
                                    2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate13MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate13MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 13000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate14_4MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate14_4MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 14400000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate19_5MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate19_5MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 19500000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate21_7MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate21_7MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 21700000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}


WifiMode
VlcPhy::GetOfdmRate26MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate26MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 26000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate28_9MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate28_9MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 28900000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate39MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate39MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 39000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate43_3MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate43_3MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 43300000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate52MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate52MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 52000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate57_8MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate57_8MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 57800000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}
//WifiMode
//VlcPhy::GetOfdmRate57_8MbpsBW20MHz ()
//{
//  static WifiMode mode_vlc =
//    WifiModeFactory::CreateWifiMode ("OfdmRate57_8MbpsBW20MHz",
//                                     WIFI_MOD_CLASS_HT,
//                                     false,
//                                     20000000, 57800000,
//                                     WIFI_CODE_RATE_2_3,
//                                     64);
//  return mode_vlc;
//}


WifiMode
VlcPhy::GetOfdmRate58_5MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate58_5MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 58500000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate65MbpsBW20MHzShGi ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate65MbpsBW20MHzShGi",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 65000000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate65MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate65MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     true,
                                     20000000, 65000000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate72_2MbpsBW20MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate72_2MbpsBW20MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     20000000, 72200000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate13_5MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 13500000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate15MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate15MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 15000000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate27MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate27MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 27000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate30MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate30MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 30000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate40_5MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate40_5MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 40500000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate45MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate45MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 45000000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate54MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate54MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 54000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate60MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate60MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 60000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate81MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate81MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 81000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate90MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate90MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 90000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate108MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate108MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 108000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate120MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate120MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 120000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate121_5MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate121_5MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 121500000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate135MbpsBW40MHzShGi ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate135MbpsBW40MHzShGi",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 135000000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode_vlc;
}
WifiMode
VlcPhy::GetOfdmRate135MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate135MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 135000000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
  return mode_vlc;
}

WifiMode
VlcPhy::GetOfdmRate150MbpsBW40MHz ()
{
  static WifiMode mode_vlc =
    WifiModeFactory::CreateWifiMode ("OfdmRate150MbpsBW40MHz",
                                     WIFI_MOD_CLASS_HT,
                                     false,
                                     40000000, 150000000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
  return mode_vlc;
}

std::ostream& operator<< (std::ostream& os, enum VlcPhy::State state)
{
  switch (state)
    {
    case VlcPhy::IDLE:
      return (os << "IDLE");
    case VlcPhy::CCA_BUSY:
      return (os << "CCA_BUSY");
    case VlcPhy::TX:
      return (os << "TX");
    case VlcPhy::RX:
      return (os << "RX");
    case VlcPhy::SWITCHING:
      return (os << "SWITCHING");
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state");
      return (os << "INVALID");
    }
}



} // namespace ns3

namespace {

static class Constructor
{
public:
  Constructor ()
  {
    ns3::VlcPhy::GetDsssRate1Mbps ();
    ns3::VlcPhy::GetDsssRate2Mbps ();
    ns3::VlcPhy::GetDsssRate5_5Mbps ();
    ns3::VlcPhy::GetDsssRate11Mbps ();
    ns3::VlcPhy::GetErpOfdmRate6Mbps ();
    ns3::VlcPhy::GetErpOfdmRate9Mbps ();
    ns3::VlcPhy::GetErpOfdmRate12Mbps ();
    ns3::VlcPhy::GetErpOfdmRate18Mbps ();
    ns3::VlcPhy::GetErpOfdmRate24Mbps ();
    ns3::VlcPhy::GetErpOfdmRate36Mbps ();
    ns3::VlcPhy::GetErpOfdmRate48Mbps ();
    ns3::VlcPhy::GetErpOfdmRate54Mbps ();
    ns3::VlcPhy::GetOfdmRate6Mbps ();
    ns3::VlcPhy::GetOfdmRate9Mbps ();
    ns3::VlcPhy::GetOfdmRate12Mbps ();
    ns3::VlcPhy::GetOfdmRate18Mbps ();
    ns3::VlcPhy::GetOfdmRate24Mbps ();
    ns3::VlcPhy::GetOfdmRate36Mbps ();
    ns3::VlcPhy::GetOfdmRate48Mbps ();
    ns3::VlcPhy::GetOfdmRate54Mbps ();
    ns3::VlcPhy::GetOfdmRate3MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate4_5MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate6MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate9MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate12MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate18MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate24MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate27MbpsBW10MHz ();
    ns3::VlcPhy::GetOfdmRate1_5MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate2_25MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate3MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate4_5MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate6MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate9MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate12MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate13_5MbpsBW5MHz ();
    ns3::VlcPhy::GetOfdmRate6_5MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate13MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate19_5MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate26MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate39MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate52MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate58_5MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate65MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate13_5MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate27MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate40_5MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate54MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate81MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate108MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate121_5MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate135MbpsBW40MHz ();
    ns3::VlcPhy::GetOfdmRate7_2MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate14_4MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate21_7MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate28_9MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate43_3MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate57_8MbpsBW20MHz ();
    ns3::VlcPhy::GetOfdmRate65MbpsBW20MHzShGi ();
    ns3::VlcPhy::GetOfdmRate72_2MbpsBW20MHz ();

  }
} g_constructor;
} 
