/*
 * rtp.h
 *
 * RTP protocol handler
 *
 * Open H323 Library
 *
 * Copyright (c) 1998-2001 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Open H323 Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions of this code were written with the assisance of funding from
 * Vovida Networks, Inc. http://www.vovida.com.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: rtp.h,v $
 * Revision 1.2009  2002/04/10 03:10:13  robertj
 * Added referential (container) copy functions to session manager class.
 *
 * Revision 2.7  2002/02/13 02:30:06  robertj
 * Added ability for media patch (and transcoders) to handle multiple RTP frames.
 *
 * Revision 2.6  2002/02/11 09:32:12  robertj
 * Updated to openH323 v1.8.0
 *
 * Revision 2.5  2002/01/22 05:58:55  robertj
 * Put MaxPayloadType back in for backward compatibility
 *
 * Revision 2.4  2002/01/22 05:03:06  robertj
 * Added enum for illegal payload type value.
 *
 * Revision 2.3  2001/11/14 06:20:40  robertj
 * Changed sending of control channel reports to be timer based.
 *
 * Revision 1.34  2002/02/09 02:33:37  robertj
 * Improved payload type docuemntation and added Cisco CN.
 *
 * Revision 1.33  2002/01/22 07:08:26  robertj
 * Added IllegalPayloadType enum as need marker for none set
 *   and MaxPayloadType is a legal value.
 *
 * Revision 1.32  2001/11/09 05:39:54  craigs
 * Added initial T.38 support thanks to Adam Lazur
 *
 * Revision 2.2  2001/10/05 00:22:13  robertj
 * Updated to PWLib 1.2.0 and OpenH323 1.7.0
 *
 * Revision 2.1  2001/08/01 05:08:43  robertj
 * Moved default session ID's to OpalMediaFormat class.
 *
 * Revision 2.0  2001/07/27 15:48:24  robertj
 * Conversion of OpenH323 to Open Phone Abstraction Library (OPAL)
 *
 * Revision 1.31  2001/09/11 00:21:21  robertj
 * Fixed missing stack sizes in endpoint for cleaner thread and jitter thread.
 *
 * Revision 1.30  2001/07/06 06:32:22  robertj
 * Added flag and checks for RTP data having specific SSRC.
 * Changed transmitter IP address check so is based on first received
 *    PDU instead of expecting it to come from the host we are sending to.
 *
 * Revision 1.29  2001/06/04 11:37:48  robertj
 * Added thread safe enumeration functions of RTP sessions.
 * Added member access functions to UDP based RTP sessions.
 *
 * Revision 1.28  2001/04/24 06:15:50  robertj
 * Added work around for strange Cisco bug which suddenly starts sending
 *   RTP packets beginning at a difference sequence number base.
 *
 * Revision 1.27  2001/04/02 23:58:23  robertj
 * Added jitter calculation to RTP session.
 * Added trace of statistics.
 *
 * Revision 1.26  2001/02/09 05:16:24  robertj
 * Added #pragma interface for GNU C++.
 *
 * Revision 1.25  2000/12/18 08:58:30  craigs
 * Added ability set ports
 *
 * Revision 1.24  2000/09/25 01:44:31  robertj
 * Fixed possible race condition on shutdown of RTP session with jitter buffer.
 *
 * Revision 1.23  2000/09/21 02:06:06  craigs
 * Added handling for endpoints that return conformant, but useless, RTP address
 * and port numbers
 *
 * Revision 1.22  2000/05/23 12:57:28  robertj
 * Added ability to change IP Type Of Service code from applications.
 *
 * Revision 1.21  2000/05/18 11:53:34  robertj
 * Changes to support doc++ documentation generation.
 *
 * Revision 1.20  2000/05/04 11:49:21  robertj
 * Added Packets Too Late statistics, requiring major rearrangement of jitter buffer code.
 *
 * Revision 1.19  2000/05/02 04:32:25  robertj
 * Fixed copyright notice comment.
 *
 * Revision 1.18  2000/05/01 01:01:24  robertj
 * Added flag for what to do with out of orer packets (use if jitter, don't if not).
 *
 * Revision 1.17  2000/04/30 03:55:18  robertj
 * Improved the RTCP messages, epecially reports
 *
 * Revision 1.16  2000/04/10 17:39:21  robertj
 * Fixed debug output of RTP payload types to allow for unknown numbers.
 *
 * Revision 1.15  2000/04/05 03:17:31  robertj
 * Added more RTP statistics gathering and H.245 round trip delay calculation.
 *
 * Revision 1.14  2000/03/23 02:55:18  robertj
 * Added sending of SDES control packets.
 *
 * Revision 1.13  2000/03/20 20:54:04  robertj
 * Fixed problem with being able to reopen for reading an RTP_Session (Cisco compatibilty)
 *
 * Revision 1.12  2000/02/29 13:00:13  robertj
 * Added extra statistic display for RTP packets out of order.
 *
 * Revision 1.11  1999/12/30 09:14:31  robertj
 * Changed payload type functions to use enum.
 *
 * Revision 1.10  1999/12/23 23:02:35  robertj
 * File reorganision for separating RTP from H.323 and creation of LID for VPB support.
 *
 * Revision 1.9  1999/11/20 05:35:57  robertj
 * Fixed possibly I/O block in RTP read loops.
 *
 * Revision 1.8  1999/11/19 09:17:15  robertj
 * Fixed problems with aycnhronous shut down of logical channels.
 *
 * Revision 1.7  1999/11/14 11:41:18  robertj
 * Added access functions to RTP statistics.
 *
 * Revision 1.6  1999/09/08 04:05:48  robertj
 * Added support for video capabilities & codec, still needs the actual codec itself!
 *
 * Revision 1.5  1999/08/31 12:34:18  robertj
 * Added gatekeeper support.
 *
 * Revision 1.4  1999/07/13 09:53:24  robertj
 * Fixed some problems with jitter buffer and added more debugging.
 *
 * Revision 1.3  1999/07/09 06:09:49  robertj
 * Major implementation. An ENORMOUS amount of stuff added everywhere.
 *
 * Revision 1.2  1999/06/22 13:49:40  robertj
 * Added GSM support and further RTP protocol enhancements.
 *
 * Revision 1.1  1999/06/14 06:12:25  robertj
 * Changes for using RTP sessions correctly in H323 Logical Channel context
 *
 */

#ifndef __RTP_RTP_H
#define __RTP_RTP_H

#ifdef __GNUC__
#pragma interface
#endif


#include <ptlib/sockets.h>


class RTP_JitterBuffer;


///////////////////////////////////////////////////////////////////////////////
// Real Time Protocol - IETF RFC1889 and RFC1890

/**An RTP data frame encapsulation.
  */
class RTP_DataFrame : public PBYTEArray
{
  PCLASSINFO(RTP_DataFrame, PBYTEArray);

  public:
    RTP_DataFrame(PINDEX payloadSize = 2048);

    enum {
      ProtocolVersion = 2,
      MinHeaderSize = 12
    };

    enum PayloadTypes {
      PCMU,         // G.711 u-Law
      FS1016,       // Federal Standard 1016 CELP
      G721,         // ADPCM - Subsumed by G.726
      G726 = G721,
      GSM,          // GSM 06.10
      G7231,        // G.723.1 at 6.3kbps or 5.3 kbps
      DVI4_8k,      // DVI4 at 8kHz sample rate
      DVI4_16k,     // DVI4 at 16kHz sample rate
      LPC,          // LPC-10 Linear Predictive CELP
      PCMA,         // G.711 A-Law
      G722,         // G.722
      L16_Stereo,   // 16 bit linear PCM
      L16_Mono,     // 16 bit linear PCM
      G723,         // G.723
      CN,           // Confort Noise
      MPA,          // MPEG1 or MPEG2 audio
      G728,         // G.728 16kbps CELP
      DVI4_11k,     // DVI4 at 11kHz sample rate
      DVI4_22k,     // DVI4 at 22kHz sample rate
      G729,         // G.729 8kbps
      Cisco_CN,     // Cisco systems comfort noise (unofficial)

      CelB = 25,    // Sun Systems Cell-B video
      JPEG,         // Motion JPEG
      H261 = 31,    // H.261
      MPV,          // MPEG1 or MPEG2 video
      MP2T,         // MPEG2 transport system
      H263,         // H.263

      LastKnownPayloadType,
      DynamicBase = 96,
      MaxPayloadType = 127,
      IllegalPayloadType
    };

    unsigned GetVersion() const { return (theArray[0]>>6)&3; }

    BOOL GetExtension() const   { return (theArray[0]&0x10) != 0; }
    void SetExtension(BOOL ext);

    BOOL GetMarker() const { return (theArray[1]&0x80) != 0; }
    void SetMarker(BOOL m);

    PayloadTypes GetPayloadType() const { return (PayloadTypes)(theArray[1]&0x7f); }
    void         SetPayloadType(PayloadTypes t);

    WORD GetSequenceNumber() const { return *(PUInt16b *)&theArray[2]; }
    void SetSequenceNumber(WORD n) { *(PUInt16b *)&theArray[2] = n; }

    DWORD GetTimestamp() const  { return *(PUInt32b *)&theArray[4]; }
    void  SetTimestamp(DWORD t) { *(PUInt32b *)&theArray[4] = t; }

    DWORD GetSyncSource() const  { return *(PUInt32b *)&theArray[8]; }
    void  SetSyncSource(DWORD s) { *(PUInt32b *)&theArray[8] = s; }

    PINDEX GetContribSrcCount() const { return theArray[0]&0xf; }
    DWORD  GetContribSource(PINDEX idx) const;
    void   SetContribSource(PINDEX idx, DWORD src);

    PINDEX GetHeaderSize()     const { return 12 + 4*GetContribSrcCount(); }

    PINDEX GetPayloadSize() const { return payloadSize; }
    BOOL   SetPayloadSize(PINDEX sz);
    BYTE * GetPayloadPtr()     const { return (BYTE *)(theArray+GetHeaderSize()); }

  protected:
    PINDEX payloadSize;

#if PTRACING
    friend ostream & operator<<(ostream & o, PayloadTypes t);
#endif
};

PLIST(RTP_DataFrameList, RTP_DataFrame);


/**An RTP control frame encapsulation.
  */
class RTP_ControlFrame : public PBYTEArray
{
  PCLASSINFO(RTP_ControlFrame, PBYTEArray);

  public:
    RTP_ControlFrame(PINDEX payloadSize = 2048);

    unsigned GetVersion() const { return (BYTE)theArray[0]>>6; }

    unsigned GetCount() const { return (BYTE)theArray[0]&0x1f; }
    void     SetCount(unsigned count);

    enum PayloadTypes {
      e_SenderReport = 200,
      e_ReceiverReport,
      e_SourceDescription,
      e_Goodbye,
      e_ApplDefined
    };

    unsigned GetPayloadType() const { return (BYTE)theArray[1]; }
    void     SetPayloadType(unsigned t);

    PINDEX GetPayloadSize() const { return 4*(*(PUInt16b *)&theArray[2]); }
    void   SetPayloadSize(PINDEX sz);

    BYTE * GetPayloadPtr() const { return (BYTE *)(theArray+4); }

#pragma pack(1)
    struct ReceiverReport {
      PUInt32b ssrc;      /* data source being reported */
      BYTE fraction;      /* fraction lost since last SR/RR */
      BYTE lost[3];	  /* cumulative number of packets lost (signed!) */
      PUInt32b last_seq;  /* extended last sequence number received */
      PUInt32b jitter;    /* interarrival jitter */
      PUInt32b lsr;       /* last SR packet from this source */
      PUInt32b dlsr;      /* delay since last SR packet */

      unsigned GetLostPackets() const { return (lost[0]<<16U)+(lost[1]<<8U)+lost[2]; }
      void SetLostPackets(unsigned lost);
    };

    struct SenderReport {
      PUInt32b ssrc;      /* source this RTCP packet refers to */
      PUInt32b ntp_sec;   /* NTP timestamp */
      PUInt32b ntp_frac;
      PUInt32b rtp_ts;    /* RTP timestamp */
      PUInt32b psent;     /* packets sent */
      PUInt32b osent;     /* octets sent */ 
    };

    enum DescriptionTypes {
      e_END,
      e_CNAME,
      e_NAME,
      e_EMAIL,
      e_PHONE,
      e_LOC,
      e_TOOL,
      e_NOTE,
      e_PRIV,
      NumDescriptionTypes
    };

    struct SourceDescription {
      PUInt32b src;       /* first SSRC/CSRC */
      struct Item {
        BYTE type;        /* type of SDES item (enum DescriptionTypes) */
        BYTE length;      /* length of SDES item (in octets) */
        char data[1];     /* text, not zero-terminated */

        const Item * GetNextItem() const { return (const Item *)((char *)this + length + 2); }
        Item * GetNextItem() { return (Item *)((char *)this + length + 2); }
      } item[1];          /* list of SDES items */
    };

    SourceDescription & AddSourceDescription(
      DWORD src   /// SSRC/CSRC identifier
    );

    SourceDescription::Item & AddSourceDescriptionItem(
      SourceDescription & sdes, /// SDES record to add item to
      unsigned type,            /// Description type
      const PString & data      /// Data for description
    );
#pragma pack()
};


class RTP_Session;

/**This class is the base for user data that may be attached to the RTP_session
   allowing callbacks for statistics and progress monitoring to be passed to an
   arbitrary object that an RTP consumer may require.
  */
class RTP_UserData : public PObject
{
  PCLASSINFO(RTP_UserData, PObject);

  public:
    /**Callback from the RTP session for transmit statistics monitoring.
       This is called every RTP_Session::senderReportTime milliseconds on the
       transmitter indicating that the statistics have been updated.

       The default behaviour does nothing.
      */
    virtual void OnTxStatistics(
      const RTP_Session & session   /// Session with statistics
    ) const;

    /**Callback from the RTP session for receive statistics monitoring.
       This is called every RTP_Session::receiverReportTime milliseconds on the
       receiver indicating that the statistics have been updated.

       The default behaviour does nothing.
      */
    virtual void OnRxStatistics(
      const RTP_Session & session   /// Session with statistics
    ) const;
};


/**This class is for encpsulating the IETF Real Time Protocol interface.
 */
class RTP_Session : public PObject
{
  PCLASSINFO(RTP_Session, PObject);

  public:
  /**@name Construction */
  //@{
    /**Create a new RTP session.
     */
    RTP_Session(
      unsigned id,                    /// Session ID for RTP channel
      RTP_UserData * userData = NULL  /// Optional data for session.
    );

    /**Delete a session.
       This deletes the userData field.
     */
    ~RTP_Session();
  //@}

  /**@name Operations */
  //@{
    /**Sets the size of the jitter buffer to be used by this RTP session.
       A session default to not having any jitter buffer enabled for reading
       and the ReadBufferedData() function simply calls ReadData(). Once a
       jitter buffer has been created it cannot be removed, though its size
       may be adjusted.
       
       If the jitterDelay paramter is zero, it destroys the jitter buffer
       attached to this RTP session.
      */
    void SetJitterBufferSize(
      unsigned jitterDelay,    /// Total jitter buffer delay in milliseconds
      PINDEX stackSize = 30000 /// Stack size for jitter thread
    );

    /**Read a data frame from the RTP channel.
       This function will conditionally read data from eth jitter buffer or
       directly if there is no jitter buffer enabled. An application should
       generally use this in preference to directly calling ReadData().
      */
    BOOL ReadBufferedData(
      DWORD timestamp,        /// Timestamp to read from buffer.
      RTP_DataFrame & frame   /// Frame read from the RTP session
    );

    /**Read a data frame from the RTP channel.
       Any control frames received are dispatched to callbacks and are not
       returned by this function. It will block until a data frame is
       available or an error occurs.
      */
    virtual BOOL ReadData(
      RTP_DataFrame & frame   /// Frame read from the RTP session
    ) = 0;

    /**Write a data frame from the RTP channel.
      */
    virtual BOOL WriteData(
      RTP_DataFrame & frame   /// Frame to write to the RTP session
    ) = 0;

    /**Write a control frame from the RTP channel.
      */
    virtual BOOL WriteControl(
      RTP_ControlFrame & frame    /// Frame to write to the RTP session
    ) = 0;

    /**Close down the RTP session.
      */
    virtual void Close(
      BOOL reading    /// Closing the read side of the session
    ) = 0;

    /**Get the local host name as used in SDES packes.
      */
    virtual PString GetLocalHostName() = 0;
  //@}

  /**@name Call back frunctions */
  //@{
    enum SendReceiveStatus {
      e_ProcessPacket,
      e_IgnorePacket,
      e_AbortTransport
    };
    virtual SendReceiveStatus OnSendData(RTP_DataFrame & frame);
    virtual SendReceiveStatus OnReceiveData(const RTP_DataFrame & frame);
    virtual SendReceiveStatus OnReceiveControl(const RTP_ControlFrame & frame);

    class ReceiverReport : public PObject  {
        PCLASSINFO(ReceiverReport, PObject);
      public:
        void PrintOn(ostream &) const;

        DWORD sourceIdentifier;
        DWORD fractionLost;         /* fraction lost since last SR/RR */
        DWORD totalLost;	    /* cumulative number of packets lost (signed!) */
        DWORD lastSequenceNumber;   /* extended last sequence number received */
        DWORD jitter;               /* interarrival jitter */
        PTimeInterval lastTimestamp;/* last SR packet from this source */
        PTimeInterval delay;        /* delay since last SR packet */
    };
    PARRAY(ReceiverReportArray, ReceiverReport);

    class SenderReport : public PObject  {
        PCLASSINFO(SenderReport, PObject);
      public:
        void PrintOn(ostream &) const;

        DWORD sourceIdentifier;
        PTime realTimestamp;
        DWORD rtpTimestamp;
        DWORD packetsSent;
        DWORD octetsSent;
    };
    virtual void OnRxSenderReport(const SenderReport & sender,
                                  const ReceiverReportArray & reports);
    virtual void OnRxReceiverReport(DWORD src,
                                    const ReceiverReportArray & reports);

    class SourceDescription : public PObject  {
        PCLASSINFO(SourceDescription, PObject);
      public:
        SourceDescription(DWORD src) { sourceIdentifier = src; }
        void PrintOn(ostream &) const;

        DWORD            sourceIdentifier;
        POrdinalToString items;
    };
    PARRAY(SourceDescriptionArray, SourceDescription);
    virtual void OnRxSourceDescription(const SourceDescriptionArray & descriptions);

    virtual void OnRxGoodbye(const PDWORDArray & sources,
                             const PString & reason);

    virtual void OnRxApplDefined(const PString & type, unsigned subtype, DWORD src,
                                 const BYTE * data, PINDEX size);
  //@}

  /**@name Member variable access */
  //@{
    /**Get the ID for the RTP session.
      */
    unsigned GetSessionID() const { return sessionID; }

    /**Get the user data for the session.
      */
    RTP_UserData * GetUserData() const { return userData; }

    /**Set the user data for the session.
      */
    void SetUserData(
      RTP_UserData * data   // New user data to be used
    );

    /**Get the source output identifier.
      */
    DWORD GetSyncSourceOut() const { return syncSourceOut; }

    /**Increment reference count for RTP session.
      */
    void IncrementReference() { referenceCount++; }

    /**Decrement reference count for RTP session.
      */
    BOOL DecrementReference() { return --referenceCount == 0; }

    /**Indicate if will ignore all but first received SSRC value.
      */
    BOOL WillIgnoreOtherSources() const { return ignoreOtherSources; }

    /**Indicate if will ignore all but first received SSRC value.
      */
    void SetIgnoreOtherSources(
      BOOL ignore   /// Flag for ignore other SSRC values
    ) { ignoreOtherSources = ignore; }

    /**Indicate if will ignore out of order packets.
      */
    BOOL WillIgnoreOutOfOrderPackets() const { return ignoreOutOfOrderPackets; }

    /**Indicate if will ignore out of order packets.
      */
    void SetIgnoreOutOfOrderPackets(
      BOOL ignore   /// Flag for ignore out of order packets
    ) { ignoreOutOfOrderPackets = ignore; }

    /**Get the time interval for transmitter reports in the session.
      */
    const PTimeInterval & GetSenderReportTime() { return senderReportTimer.GetResetTime(); }

    /**Set the time interval for transmitter reports in the session.
      */
    void SetSenderReportTime(
      const PTimeInterval & time   // Time between reports
    );

    /**Get the interval for receiver reports in the session.
      */
    const PTimeInterval & GetReceiverReportTime() { return receiverReportTimer.GetResetTime(); }

    /**Set the interval for receiver reports in the session.
      */
    void SetReceiverReportTime(
      const PTimeInterval & time  // Time between reports
    );

    /**Get total number of packets sent in session.
      */
    DWORD GetPacketsSent() const { return packetsSent; }

    /**Get total number of octets sent in session.
      */
    DWORD GetOctetsSent() const { return octetsSent; }

    /**Get total number of packets received in session.
      */
    DWORD GetPacketsReceived() const { return packetsReceived; }

    /**Get total number of octets received in session.
      */
    DWORD GetOctetsReceived() const { return octetsReceived; }

    /**Get total number received packets lost in session.
      */
    DWORD GetPacketsLost() const { return packetsLost; }

    /**Get total number of packets received out of order in session.
      */
    DWORD GetPacketsOutOfOrder() const { return packetsOutOfOrder; }

    /**Get total number received packets too late to go into jitter buffer.
      */
    DWORD GetPacketsTooLate() const;

    /**Get average time between sent packets.
       This is averaged over the last senderReportTime milliseconds and is in
       milliseconds.
      */
    DWORD GetAverageSendTime() const { return averageSendTime; }

    /**Get maximum time between sent packets.
       This is over the last senderReportTime milliseconds and is in
       milliseconds.
      */
    DWORD GetMaximumSendTime() const { return maximumSendTime; }

    /**Get minimum time between sent packets.
       This is over the last senderReportTime milliseconds and is in
       milliseconds.
      */
    DWORD GetMinimumSendTime() const { return minimumSendTime; }

    /**Get average time between received packets.
       This is averaged over the last receiverReportTime milliseconds and is in
       milliseconds.
      */
    DWORD GetAverageReceiveTime() const { return averageReceiveTime; }

    /**Get maximum time between received packets.
       This is over the last receiverReportTime milliseconds and is in
       milliseconds.
      */
    DWORD GetMaximumReceiveTime() const { return maximumReceiveTime; }

    /**Get minimum time between received packets.
       This is over the last receiverReportTime milliseconds and is in
       milliseconds.
      */
    DWORD GetMinimumReceiveTime() const { return minimumReceiveTime; }

    /**Get averaged jitter time for received packets.
       This is the calculated statistical variance of the interarrival
       time of received packets in milliseconds.
      */
    DWORD GetJitterTime() const { return jitterLevel>>7; }
  //@}

  protected:
    void SetReceiverReport(RTP_ControlFrame::ReceiverReport & receiver);

    unsigned           sessionID;
    unsigned           referenceCount;
    RTP_UserData     * userData;
    RTP_JitterBuffer * jitter;

    BOOL          ignoreOtherSources;
    BOOL          ignoreOutOfOrderPackets;
    DWORD         syncSourceOut;
    DWORD         syncSourceIn;
    WORD          lastSentSequenceNumber;
    WORD          expectedSequenceNumber;
    PTimeInterval lastSentPacketTime;
    PTimeInterval lastReceivedPacketTime;
    WORD          lastRRSequenceNumber;
    PINDEX        consecutiveOutOfOrderPackets;

    // Statistics
    DWORD packetsSent;
    DWORD octetsSent;
    DWORD packetsReceived;
    DWORD octetsReceived;
    DWORD packetsLost;
    DWORD packetsOutOfOrder;
    DWORD averageSendTime;
    DWORD maximumSendTime;
    DWORD minimumSendTime;
    DWORD averageReceiveTime;
    DWORD maximumReceiveTime;
    DWORD minimumReceiveTime;
    DWORD jitterLevel;

    PTimer   senderReportTimer;
    PTimer   receiverReportTimer;
    unsigned senderReportCount;
    unsigned receiverReportCount;
    DWORD    averageSendTimeAccum;
    DWORD    maximumSendTimeAccum;
    DWORD    minimumSendTimeAccum;
    DWORD    averageReceiveTimeAccum;
    DWORD    maximumReceiveTimeAccum;
    DWORD    minimumReceiveTimeAccum;
    DWORD    lastTransitTime;
};


/**This class is for encpsulating the IETF Real Time Protocol interface.
 */
class RTP_SessionManager : public PObject
{
  PCLASSINFO(RTP_SessionManager, PObject);

  public:
  /**@name Construction */
  //@{
    /**Construct new session manager database.
      */
    RTP_SessionManager();
    RTP_SessionManager(const RTP_SessionManager & sm);
    RTP_SessionManager & operator=(const RTP_SessionManager & sm);
  //@}


  /**@name Operations */
  //@{
    /**Use an RTP session for the specified ID.

       If this function returns a non-null value, then the ReleaseSession()
       function MUST be called or the session is never deleted for the
       lifetime of the session manager.

       If there is no session of the specified ID, then you MUST call the
       AddSession() function with a new RTP_Session. The mutex flag is left
       locked in this case. The AddSession() expects the mutex to be locked
       and unlocks it automatically.
      */
    RTP_Session * UseSession(
      unsigned sessionID    /// Session ID to use.
    );

    /**Add an RTP session for the specified ID.

       This function MUST be called only after the UseSession() function has
       returned NULL. The mutex flag is left locked in that case. This
       function expects the mutex to be locked and unlocks it automatically.
      */
    void AddSession(
      RTP_Session * session    /// Session to add.
    );

    /**Release the session. If the session ID is not being used any more any
       clients via the UseSession() function, then the session is deleted.
     */
    void ReleaseSession(
      unsigned sessionID    /// Session ID to release.
    );

    /**Get a session for the specified ID.
       Unlike UseSession, this does not increment the usage count on the
       session so may be used to just gain a pointer to an RTP session.
     */
    RTP_Session * GetSession(
      unsigned sessionID    /// Session ID to get.
    ) const;

    /**Begin an enumeration of the RTP sessions.
       This function acquires the mutex for the sessions database and returns
       the first element in it.

         eg:
         RTP_Session * session;
         for (session = rtpSessions.First(); session != NULL; session = rtpSessions.Next()) {
           if (session->Something()) {
             rtpSessions.Exit();
             break;
           }
         }

       Note that the Exit() function must be called if the enumeration is
       stopped prior to Next() returning NULL.
      */
    RTP_Session * First();

    /**Get the next session in the enumeration.
       The session database remains locked until this function returns NULL.

       Note that the Exit() function must be called if the enumeration is
       stopped prior to Next() returning NULL.
      */
    RTP_Session * Next();

    /**Exit the enumeration of RTP sessions.
       If the enumeration is desired to be exited before Next() returns NULL
       this this must be called to unlock the session database.

       Note that you should NOT call Exit() if Next() HAS returned NULL, or
       race conditions can result.
      */
    void Exit();
  //@}


  protected:
    PDICTIONARY(SessionDict, POrdinalKey, RTP_Session);
    SessionDict sessions;
    PMutex      mutex;
    PINDEX      enumerationIndex;
};



/**This class is for the IETF Real Time Protocol interface on UDP/IP.
 */
class RTP_UDP : public RTP_Session
{
  PCLASSINFO(RTP_UDP, RTP_Session);

  public:
  /**@name Construction */
  //@{
    /**Create a new RTP channel.
     */
    RTP_UDP(
      unsigned id  /// Session ID for RTP channel
    );

    /// Destroy the RTP
    ~RTP_UDP();
  //@}

  /**@name Overrides from class RTP_Session */
  //@{
    /**Read a data frame from the RTP channel.
       Any control frames received are dispatched to callbacks and are not
       returned by this function. It will block until a data frame is
       available or an error occurs.
      */
    virtual BOOL ReadData(RTP_DataFrame & frame);

    /**Write a data frame from the RTP channel.
      */
    virtual BOOL WriteData(RTP_DataFrame & frame);

    /**Write a control frame from the RTP channel.
      */
    virtual BOOL WriteControl(RTP_ControlFrame & frame);

    /**Close down the RTP session.
      */
    virtual void Close(
      BOOL reading    /// Closing the read side of the session
    );

    /**Get the session description name.
      */
    virtual PString GetLocalHostName();
  //@}

  /**@name New functions for class */
  //@{
    /**Open the UDP ports for the RTP session.
      */
    BOOL Open(
      PIPSocket::Address localAddress,  /// Local interface to bind to
      WORD portBase,                    /// Base of ports to search
      WORD portMax,                     /// end of ports to search (inclusive)
      BYTE ipTypeOfService              /// Type of Service byte
    );
  //@}

  /**@name Member variable access */
  //@{
    /**Get local address of session.
      */
    PIPSocket::Address GetLocalAddress() const { return localAddress; }

    /**Get remote address of session.
      */
    PIPSocket::Address GetRemoteAddress() const { return remoteAddress; }

    /**Get local data port of session.
      */
    WORD GetLocalDataPort() const { return localDataPort; }

    /**Get local control port of session.
      */
    WORD GetLocalControlPort() const { return localControlPort; }

    /**Get remote data port of session.
      */
    WORD GetRemoteDataPort() const { return remoteDataPort; }

    /**Get remote control port of session.
      */
    WORD GetRemoteControlPort() const { return remoteControlPort; }

    /**Get data UDP socket of session.
      */
    PUDPSocket & GetDataSocket() { return dataSocket; }

    /**Get control UDP socket of session.
      */
    PUDPSocket & GetControlSocket() { return controlSocket; }

    /**Set the remote address and port information for session.
      */
    BOOL SetRemoteSocketInfo(
      PIPSocket::Address address,   /// Address of remote
      WORD port,                    /// Port on remote
      BOOL isDataPort               /// Flag for data or control channel
    );
  //@}

  protected:
    SendReceiveStatus ReadDataPDU(RTP_DataFrame & frame);
    SendReceiveStatus ReadControlPDU();
    SendReceiveStatus ReadDataOrControlPDU(
      PUDPSocket & socket,
      PBYTEArray & frame,
      const char * PTRACE_name
    );

    PIPSocket::Address localAddress;
    WORD               localDataPort;
    WORD               localControlPort;

    PIPSocket::Address remoteAddress;
    WORD               remoteDataPort;
    WORD               remoteControlPort;

    PIPSocket::Address remoteTransmitAddress;

    BOOL shutdownRead;
    BOOL shutdownWrite;

    PUDPSocket dataSocket;
    PUDPSocket controlSocket;
};


#endif // __RTP_RTP_H


/////////////////////////////////////////////////////////////////////////////
