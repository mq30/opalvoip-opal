/*
 * transport.cxx
 *
 * Opal transports handler
 *
 * Open Phone Abstraction Library (OPAL)
 * Formally known as the Open H323 project.
 *
 * Copyright (c) 2001 Equivalence Pty. Ltd.
 * Portions Copyright (C) 2006 by Post Increment
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
 * The Original Code is Open Phone Abstraction Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): Post Increment
 *     Portions of this code were written with the assistance of funding from
 *     US Joint Forces Command Joint Concept Development & Experimentation (J9)
 *     http://www.jfcom.mil/about/abt_j9.htm
 *
 * $Log: transports.cxx,v $
 * Revision 2.93  2007/10/12 04:00:44  rjongbloed
 * Fixed being able to SIP call to localhost.
 *
 * Revision 2.92  2007/10/12 00:20:17  rjongbloed
 * Added more logging
 *
 * Revision 2.91  2007/10/07 07:36:13  rjongbloed
 * Changed bundled sockets so does not return error if interface goes away it just
 *   blocks reads till the interface comes back, or is explicitly closed.
 * Also return error codes, rather than just a BOOL.
 *
 * Revision 2.90  2007/10/04 05:41:20  rjongbloed
 * Assure OpalTransportUDP is pre-opened in non-listener constructor
 *   to emulate the previous semantics. Fixes issue with gk server.
 *
 * Revision 2.89  2007/09/22 04:32:30  rjongbloed
 * Fixed lock up on exit whena  gatekeeper is used.
 * Also fixed fatal "read error" (ECONNRESET) when send packet to a machine which
 *   is not listening on the specified port. No error is lgged but does not stop listener.
 *
 * Revision 2.88  2007/09/07 05:44:15  rjongbloed
 * Fixed initialisation of SSL context in both OpalListenerTCPS contructors.
 *
 * Revision 2.87  2007/08/26 20:17:44  hfriederich
 * Allow to filter interfaces based on destination address
 *
 * Revision 2.86  2007/08/08 12:43:25  rjongbloed
 * Incorrect test for STUN local address.
 *
 * Revision 2.85  2007/08/03 18:43:15  dsandras
 * Fixed remaining STUN issues.
 *
 * Revision 2.84  2007/08/03 11:00:51  dsandras
 * Removed comment. Ooops.
 *
 * Revision 2.83  2007/08/03 10:54:37  dsandras
 * Fixed warning on GNU/Linux.
 *
 * Revision 2.82  2007/07/24 13:46:45  rjongbloed
 * Fixed correct selection of interface after forked INVITE reply arrives on bundled socket.
 *
 * Revision 2.81  2007/07/23 06:32:52  csoutheren
 * Temporary hack to avoid problem with new bundle code.
 * This change results in PDUs being sent on multiple interfaces and will be removed
 * once further debugging has been performed
 *
 * Revision 2.80  2007/07/22 04:03:46  rjongbloed
 * Fixed issues with STUN usage in socket bundling, now OpalTransport indicates
 *   if it wants local or NAT address/port for inclusion to outgoing PDUs.
 *
 * Revision 2.79  2007/07/03 08:54:07  rjongbloed
 * Fixed spurios error log when dropping interface.
 *
 * Revision 2.78  2007/06/25 05:44:45  rjongbloed
 * Fixed numerous issues with "bound" managed socket, ie associating
 *   listeners to a specific named interface.
 *
 * Revision 2.77  2007/06/10 08:55:12  rjongbloed
 * Major rework of how SIP utilises sockets, using new "socket bundling" subsystem.
 *
 * Revision 2.76  2007/04/04 02:12:01  rjongbloed
 * Reviewed and adjusted PTRACE log levels
 *   Now follows 1=error,2=warn,3=info,4+=debug
 *
 * Revision 2.75  2007/03/29 07:07:23  rjongbloed
 * Fixed deadlock in UDP multi-interface connect algorithm, and getting a SIP retry before completion.
 *
 * Revision 2.74  2007/02/19 08:35:02  csoutheren
 * Add better way to fliter interfaces
 *
 * Revision 2.73  2007/02/19 08:30:40  csoutheren
 * Only allow use of loopback interface when destination is also on the loopback interface
 *
 * Revision 2.72  2007/02/13 23:38:04  csoutheren
 * Allow use of localhost for incoming calls
 *
 * Revision 2.71  2006/12/13 04:59:48  csoutheren
 * Applied 1613084 - Memory leak in internal transports handling
 * Thanks to Drazen Dimoti
 *
 * Revision 2.70  2006/12/08 05:10:44  csoutheren
 * Applied 1608002 - Callback for OpalTransportUDP multiple interface handling
 * Thanks to Hannes Friederich
 *
 * Revision 2.69  2006/09/28 07:42:18  csoutheren
 * Merge of useful SRTP implementation
 *
 * Revision 2.68  2006/08/29 01:37:11  csoutheren
 * Change secure URLs to use h323s and tcps to be inline with sips
 *
 * Revision 2.67  2006/08/28 08:07:29  csoutheren
 * Remove incorrect patch
 *
 * Revision 2.66  2006/08/28 00:51:13  csoutheren
 * Applied 1545808 - OpalTransportUDP connectSockets protection
 * Thanks to Drazen Dimoti
 *
 * Revision 2.65  2006/08/25 06:15:59  csoutheren
 * Fix problem with establishing media channels with secure transport
 *
 * Revision 2.64  2006/08/22 09:02:41  csoutheren
 * Removed left over debug code
 *
 * Revision 2.63  2006/08/21 05:29:25  csoutheren
 * Messy but relatively simple change to add support for secure (SSL/TLS) TCP transport
 * and secure H.323 signalling via the sh323 URL scheme
 *
 * Revision 2.62  2006/07/14 04:22:43  csoutheren
 * Applied 1517397 - More Phobos stability fix
 * Thanks to Dinis Rosario
 *
 * Revision 2.61  2006/06/30 07:38:59  csoutheren
 * Applied 1490817 - Fix lastReceivedAddress for OpalTransportUDP
 * Thanks to Dave Moss
 *
 * Revision 2.60  2006/06/30 05:30:41  csoutheren
 * Applied 1509269 - Fix OpalTransportUDP::Read-channelPointer
 * Thanks to Borko Jandras
 *
 * Revision 2.59  2006/06/20 05:22:30  csoutheren
 * Remove bogus warning at run-time about converting "0.0.0.0"
 *
 * Revision 2.58  2006/02/18 19:00:38  dsandras
 * Added PTRACE statements.
 *
 * Revision 2.57  2005/11/29 11:49:35  dsandras
 * socket is autodeleted, even in case of failure.
 *
 * Revision 2.56  2005/10/21 17:57:00  dsandras
 * Applied patch from Hannes Friederich <hannesf AATT ee.ethz.ch> to fix GK
 * registration issues when there are multiple interfaces. Thanks!
 *
 * Revision 2.55  2005/10/09 15:18:12  dsandras
 * Only add socket to the connectSockets when it is open.
 *
 * Revision 2.54  2005/10/09 15:12:38  dsandras
 * Moved some code around.
 *
 * Revision 2.53  2005/09/22 18:16:29  dsandras
 * Definitely fixed the previous problem.
 *
 * Revision 2.52  2005/09/22 17:07:34  dsandras
 * Fixed bug leading to a crash when registering to a gatekeeper.
 *
 * Revision 2.51  2005/09/19 20:49:59  dsandras
 * Following the API, a "reusable" address ends with '+', not something different than '+'.
 * When a socket is created, reuse the "reusable" flag from the original socket.
 *
 * Revision 2.50  2005/09/18 20:32:57  dsandras
 * New fix for the same problem that works.
 *
 * Revision 2.49  2005/09/18 18:41:01  dsandras
 * Reverted previous broken patch.
 *
 * Revision 2.48  2005/09/17 17:36:21  dsandras
 * Close the old channel before creating the new socket.
 *
 * Revision 2.47  2005/07/16 17:16:17  dsandras
 * Moved code upward so that the local source port range is always taken into account when creating a transport.
 *
 * Revision 2.46  2005/06/08 17:35:10  dsandras
 * Fixed sockets leak thanks to Ted Szoczei. Thanks!
 *
 * Revision 2.45  2005/06/02 18:39:03  dsandras
 * Committed fix for Gatekeeper registration thanks to Hannes Friederich <hannesf   __@__ ee.ethz.ch>.
 *
 * Revision 2.44  2005/05/23 20:14:48  dsandras
 * Added STUN socket to the list of connected sockets.
 *
 * Revision 2.43  2005/04/30 20:59:55  dsandras
 * Consider we are already connected only if the connectSockets array is not empty.
 *
 * Revision 2.42  2005/04/20 06:18:35  csoutheren
 * Patch 1182998. Fix for using GK through NAT, and fixed Connect to be idempotent
 * Thanks to Hannes Friederich
 *
 * Revision 2.41  2005/04/20 06:15:25  csoutheren
 * Patch 1181901. Fix race condition in OpalTransportUDP
 * Thanks to Ted Szoczei
 *
 * Revision 2.40  2005/01/16 23:08:33  csoutheren
 * Fixed problem with IPv6 INADDR_ANY
 * Fixed problem when transport thread self terminates
 *
 * Revision 2.39  2004/12/12 13:37:02  dsandras
 * Made the transport type comparison insensitive. Required for interoperation with some IP Phones.
 *
 * Revision 2.38  2004/08/14 07:56:43  rjongbloed
 * Major revision to utilise the PSafeCollection classes for the connections and calls.
 *
 * Revision 2.37  2004/05/09 13:12:38  rjongbloed
 * Fixed issues with non fast start and non-tunnelled connections
 *
 * Revision 2.36  2004/04/27 07:23:40  rjongbloed
 * Fixed uninitialised variable getting ip without port
 *
 * Revision 2.35  2004/04/27 04:40:17  rjongbloed
 * Changed UDP listener IsOpen to indicae open only if all sockets on each
 *   interface are open.
 *
 * Revision 2.34  2004/04/07 08:21:10  rjongbloed
 * Changes for new RTTI system.
 *
 * Revision 2.33  2004/03/29 11:04:19  rjongbloed
 * Fixed shut down of OpalTransportUDP when still in "connect" phase.
 *
 * Revision 2.32  2004/03/22 11:39:44  rjongbloed
 * Fixed problems with correctly terminating the OpalTransportUDP that is generated from
 *   an OpalListenerUDP, this should not close the socket or stop the thread.
 *
 * Revision 2.31  2004/03/16 12:01:37  rjongbloed
 * Temporary fix for closing UDP transport
 *
 * Revision 2.30  2004/03/13 06:30:03  rjongbloed
 * Changed parameter in UDP write function to void * from PObject *.
 *
 * Revision 2.29  2004/02/24 11:37:02  rjongbloed
 * More work on NAT support, manual external address translation and STUN
 *
 * Revision 2.28  2004/02/19 10:47:06  rjongbloed
 * Merged OpenH323 version 1.13.1 changes.
 *
 * Revision 2.27  2003/01/07 06:01:07  robertj
 * Fixed MSVC warnings.
 *
 * Revision 1.135  2003/12/03 06:57:11  csoutheren
 * Protected against dwarf Q.931 PDUs
 *
 * Revision 1.134  2003/04/10 09:45:34  robertj
 * Added associated transport to new GetInterfaceAddresses() function so
 *   interfaces can be ordered according to active transport links. Improves
 *   interoperability.
 * Replaced old listener GetTransportPDU() with GetInterfaceAddresses()
 *   and H323SetTransportAddresses() functions.
 *
 * Revision 1.133  2003/04/10 00:58:54  craigs
 * Added functions to access to lists of interfaces
 *
 * Revision 1.132  2003/03/26 06:14:31  robertj
 * More IPv6 support (INADDR_ANY handling), thanks S�bastien Josset
 *
 * Revision 1.131  2003/03/21 05:24:54  robertj
 * Added setting of remote port in UDP transport constructor.
 *
 * Revision 1.130  2003/03/20 01:51:12  robertj
 * More abstraction of H.225 RAS and H.501 protocols transaction handling.
 *
 * Revision 1.129  2003/03/11 23:15:23  robertj
 * Fixed possible double delete of socket (crash) on garbage input.
 *
 * Revision 1.128  2003/02/06 04:31:02  robertj
 * Added more support for adding things to H323TransportAddressArrays
 *
 * Revision 1.127  2003/02/05 01:57:18  robertj
 * Fixed STUN usage on gatekeeper discovery.
 *
 * Revision 1.126  2003/02/04 07:06:42  robertj
 * Added STUN support.
 *
 * Revision 1.125  2003/01/23 02:36:32  robertj
 * Increased (and made configurable) timeout for H.245 channel TCP connection.
 *
 * Revision 1.124  2002/12/23 22:46:06  robertj
 * Changed gatekeeper discovery so an GRJ does not indicate "discovered".
 *
 * Revision 1.123  2002/11/21 06:40:00  robertj
 * Changed promiscuous mode to be three way. Fixes race condition in gkserver
 *   which can cause crashes or more PDUs to be sent to the wrong place.
 *
 * Revision 1.122  2002/11/12 03:14:18  robertj
 * Fixed gatekeeper discovery so does IP address translation correctly for
 *   hosts inside the firewall.
 *
 * Revision 1.121  2002/11/10 08:10:44  robertj
 * Moved constants for "well known" ports to better place (OPAL change).
 *
 * Revision 1.120  2002/11/05 00:31:48  robertj
 * Prevented a failure to start separate H.245 channel stopping the call until
 *   after a CONNECT is received and have no audio. At that point no H.245
 *   is a useless call and we disconnect.
 *
 * Revision 1.119  2002/11/01 03:38:18  robertj
 * More IPv6 fixes, thanks S�bastien Josset.
 *
 * Revision 1.118  2002/10/29 08:30:32  robertj
 * Fixed problem with simultaneous startH245 condition possibly shutting down
 *   the call under some circumstances.
 *
 * Revision 1.117  2002/10/16 06:28:20  robertj
 * More IPv6 support changes, especially in unambiguising v6 addresses colons
 *   from the port fields colon, thanks Sebastien Josset
 *
 * Revision 1.116  2002/10/08 23:34:30  robertj
 * Fixed ip v6 usage on H.245 pdu setting.
 *
 * Revision 1.115  2002/10/08 13:08:21  robertj
 * Changed for IPv6 support, thanks S�bastien Josset.
 *
 * Revision 1.114  2002/08/05 10:03:48  robertj
 * Cosmetic changes to normalise the usage of pragma interface/implementation.
 *
 * Revision 1.113  2002/08/05 05:17:41  robertj
 * Fairly major modifications to support different authentication credentials
 *   in ARQ to the logged in ones on RRQ. For both client and server.
 * Various other H.235 authentication bugs and anomalies fixed on the way.
 *
 * Revision 1.112  2002/07/22 09:40:19  robertj
 * Added ability to automatically convert string arrays, lists sorted lists
 *   directly to H323TransportAddressArray.
 *
 * Revision 1.111  2002/07/18 08:25:47  robertj
 * Fixed problem in decoding host when '+' was used without port in address.
 *
 * Revision 1.110  2002/07/10 01:23:33  robertj
 * Added extra debugging output
 *
 * Revision 1.109  2002/07/02 10:02:32  robertj
 * Added H323TransportAddress::GetIpAddress() so don't have to provide port
 *   when you don't need it as in GetIpAndPort(),.
 *
 * Revision 1.108  2002/06/28 03:34:29  robertj
 * Fixed issues with address translation on gatekeeper RAS channel.
 *
 * Revision 1.107  2002/06/24 07:35:23  robertj
 * Fixed ability to do gk discovery on localhost, thanks Artis Kugevics
 *
 * Revision 1.106  2002/06/12 03:52:27  robertj
 * Added function to compare two transport addresses in a more intelligent
 *   way that strict string comparison. Takes into account wildcarding.
 *
 * Revision 1.105  2002/05/28 06:38:08  robertj
 * Split UDP (for RAS) from RTP port bases.
 * Added current port variable so cycles around the port range specified which
 *   fixes some wierd problems on some platforms, thanks Federico Pinna
 *
 * Revision 1.104  2002/05/22 07:39:59  robertj
 * Fixed double increment of port number when making outgoing TCP connection.
 *
 * Revision 1.103  2002/04/18 00:18:58  robertj
 * Increased timeout for thread termination assert, on heavily loaded machines it can
 *   take more than one second to complete.
 *
 * Revision 1.102  2002/04/17 05:36:38  robertj
 * Fixed problems with using pre-bound inferface/port in gk discovery.
 *
 * Revision 1.101  2002/04/12 04:51:28  robertj
 * Fixed small possibility crashes if open and close transport at same time.
 *
 * Revision 1.100  2002/03/08 01:22:30  robertj
 * Fixed possible use of IsSuspended() on terminated thread causing assert.
 *
 * Revision 1.99  2002/03/05 04:49:41  robertj
 * Fixed leak of thread (and file handles) if get incoming connection aborted
 *   very early (before receiving a setup PDU), thanks Hans Bjurstr�m
 *
 * Revision 1.98  2002/02/28 04:35:43  robertj
 * Added trace output of the socket handle number when have new connection.
 *
 * Revision 1.97  2002/02/28 00:57:03  craigs
 * Changed SetWriteTimeout to SetReadTimeout in connect, as Craig got it wrong!
 *
 * Revision 1.96  2002/02/25 10:55:33  robertj
 * Added ability to speficy dynamically allocated port in transport address.
 *
 * Revision 1.95  2002/02/14 03:36:14  craigs
 * Added default 10sec timeout on connect to IP addresses
 * This prevents indefinite hangs when connecting to IP addresses
 * that don't exist
 *
 * Revision 1.94  2002/02/05 23:29:09  robertj
 * Changed default for H.323 listener to reuse addresses.
 *
 * Revision 1.93  2002/02/01 01:48:18  robertj
 * Fixed ability to shut down a Listener, if it had never been started.
 *
 * Revision 1.92  2002/01/02 06:06:43  craigs
 * Made T.38 UDP socket obey rules
 *
 * Revision 1.91  2001/12/22 01:48:40  robertj
 * Added ability to use local and remote port from transport channel as well
 *   as explicit port in H.245 address PDU setting routine.
 * Added PrintOn() to listener and transport for tracing purposes.
 *
 * Revision 1.90  2001/12/15 07:12:22  robertj
 * Added optimisation so if discovering a static gk on same machine as ep is
 *   running on then uses that specific interface preventing multiple GRQs.
 *
 * Revision 1.89  2001/10/11 07:16:49  robertj
 * Removed port check for gk's that change sockets in mid-stream.
 *
 * Revision 1.88  2001/10/09 12:41:20  robertj
 * Set promiscuous flag back to PFalse after gatkeeper discovery.
 *
 * Revision 1.87  2001/09/10 03:06:29  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.86  2001/08/10 11:03:52  robertj
 * Major changes to H.235 support in RAS to support server.
 *
 * Revision 1.85  2001/08/07 02:57:52  robertj
 * Improved tracing on closing transport.
 *
 * Revision 1.84  2001/08/06 03:08:57  robertj
 * Fission of h323.h to h323ep.h & h323con.h, h323.h now just includes files.
 *
 * Revision 1.83  2001/07/17 04:44:32  robertj
 * Partial implementation of T.120 and T.38 logical channels.
 *
 * Revision 1.82  2001/07/06 02:31:15  robertj
 * Made sure a release complete is sent if no connection is created.
 *
 * Revision 1.81  2001/07/04 09:02:07  robertj
 * Added more tracing
 *
 * Revision 1.80  2001/06/25 05:50:22  robertj
 * Improved error logging on TCP listener.
 *
 * Revision 1.79  2001/06/25 02:28:34  robertj
 * Allowed TCP listener socket to be opened in non-exclusive mode
 *   (ie SO_REUSEADDR) to avoid daemon restart problems.
 * Added trailing '+' on H323TransportAddress string to invoke above.
 *
 * Revision 1.78  2001/06/22 02:47:12  robertj
 * Took one too many lines out in previous fix!
 *
 * Revision 1.77  2001/06/22 02:40:27  robertj
 * Fixed discovery so uses new promiscuous mode.
 * Also used the RAS GRQ address of server isntead of UDP return address
 *   for address of gatekeeper for future packets.
 *
 * Revision 1.76  2001/06/22 01:54:47  robertj
 * Removed initialisation of localAddress to hosts IP address, does not
 *   work on multi-homed hosts.
 *
 * Revision 1.75  2001/06/22 00:14:46  robertj
 * Added ConnectTo() function to conencto specific address.
 * Added promiscuous mode for UDP channel.
 *
 * Revision 1.74  2001/06/14 23:18:06  robertj
 * Change to allow for CreateConnection() to return NULL to abort call.
 *
 * Revision 1.73  2001/06/14 04:23:32  robertj
 * Changed incoming call to pass setup pdu to endpoint so it can create
 *   different connection subclasses depending on the pdu eg its alias
 *
 * Revision 1.72  2001/06/06 00:29:54  robertj
 * Added trace for when doing TCP connect.
 *
 * Revision 1.71  2001/06/02 01:35:32  robertj
 * Added thread names.
 *
 * Revision 1.70  2001/05/31 07:16:52  craigs
 * Fixed remote address initialisation for incoming H245 channels
 *
 * Revision 1.69  2001/05/17 06:37:04  robertj
 * Added multicast gatekeeper discovery support.
 *
 * Revision 1.68  2001/04/13 07:44:51  robertj
 * Moved starting connection trace message to be on both Connect() and Accept()
 *
 * Revision 1.67  2001/04/10 01:21:02  robertj
 * Added some more error messages into trace log.
 *
 * Revision 1.66  2001/04/09 08:44:19  robertj
 * Added ability to get transport address for a listener.
 * Added '*' to indicate INADDR_ANY ip address.
 *
 * Revision 1.65  2001/03/06 05:03:00  robertj
 * Changed H.245 channel start failure so does not abort call if there were
 *   some fast started media streams opened. Just lose user indications.
 *
 * Revision 1.64  2001/03/05 04:28:50  robertj
 * Added net mask to interface info returned by GetInterfaceTable()
 *
 * Revision 1.63  2001/02/09 05:13:56  craigs
 * Added pragma implementation to (hopefully) reduce the executable image size
 * under Linux
 *
 * Revision 1.62  2001/02/08 22:29:39  robertj
 * Fixed failure to reset fill character in trace log when output interface list.
 *
 * Revision 1.61  2001/01/29 06:43:32  robertj
 * Added printing of entry of interface table.
 *
 * Revision 2.26  2003/01/07 04:39:53  robertj
 * Updated to OpenH323 v1.11.2
 *
 * Revision 2.25  2002/11/10 11:33:20  robertj
 * Updated to OpenH323 v1.10.3
 *
 * Revision 2.24  2002/10/09 04:26:57  robertj
 * Fixed ability to call CloseWait() multiple times, thanks Ted Szoczei
 *
 * Revision 2.23  2002/09/26 01:21:16  robertj
 * Fixed error in trace output when get illegal transport address.
 *
 * Revision 2.22  2002/09/12 06:57:56  robertj
 * Removed protocol prefix strings as static members as has problems with
 *   use in DLL environment.
 * Added support for the + character in OpalTransportAddress decoding
 *  to indicate exclusive listener.
 *
 * Revision 2.21  2002/09/06 02:41:18  robertj
 * Added ability to specify stream or datagram (TCP or UDP) transport is to
 * be created from a transport address regardless of the addresses mode.
 *
 * Revision 2.20  2002/07/04 07:41:47  robertj
 * Fixed memory/thread leak of transports.
 *
 * Revision 2.19  2002/07/01 08:55:07  robertj
 * Changed TCp/UDP port allocation to use new thread safe functions.
 *
 * Revision 2.18  2002/07/01 08:43:44  robertj
 * Fixed assert on every SIP packet.
 *
 * Revision 2.17  2002/07/01 04:56:33  robertj
 * Updated to OpenH323 v1.9.1
 *
 * Revision 2.16  2002/06/16 23:07:19  robertj
 * Fixed several memory leaks, thanks Ted Szoczei
 * Fixed error opening UDP listener for broadcast packets under Win32.
 *   Is not needed as it is under windows, thanks Ted Szoczei
 *
 * Revision 2.15  2002/04/16 07:52:51  robertj
 * Change to allow SetRemoteAddress before UDP is connected.
 *
 * Revision 2.14  2002/04/10 03:12:35  robertj
 * Fixed SetLocalAddress to return PFalse if did not set the address to a
 *   different address to the current one. Altered UDP version to cope.
 *
 * Revision 2.13  2002/04/09 04:44:36  robertj
 * Fixed bug where crahses if close channel while in UDP connect mode.
 *
 * Revision 2.12  2002/04/09 00:22:16  robertj
 * Added ability to set the local address on a transport, under some circumstances.
 *
 * Revision 2.11  2002/03/27 05:37:39  robertj
 * Fixed removal of writeChannel after wrinting to UDP transport in connect mode.
 *
 * Revision 2.10  2002/03/15 00:20:54  robertj
 * Fixed bug when closing UDP transport when in "connect" mode.
 *
 * Revision 2.9  2002/02/06 06:07:10  robertj
 * Fixed shutting down UDP listener thread
 *
 * Revision 2.8  2002/01/14 00:19:33  craigs
 * Fixed problems with remote address used instead of local address
 *
 * Revision 2.7  2001/12/07 08:55:32  robertj
 * Used UDP port base when creating UDP transport.
 *
 * Revision 2.6  2001/11/14 06:28:20  robertj
 * Added missing break when ending UDP connect phase.
 *
 * Revision 2.5  2001/11/13 04:29:48  robertj
 * Changed OpalTransportAddress CreateTransport and CreateListsner functions
 *   to have extra parameter to control local binding of sockets.
 *
 * Revision 2.4  2001/11/12 05:31:36  robertj
 * Changed CreateTransport() on an OpalTransportAddress to bind to local address.
 * Added OpalTransportAddress::GetIpAddress when don't need port number.
 *
 * Revision 2.3  2001/11/09 05:49:47  robertj
 * Abstracted UDP connection algorithm
 *
 * Revision 2.2  2001/11/06 05:40:13  robertj
 * Added OpalListenerUDP class to do listener semantics on a UDP socket.
 *
 * Revision 2.1  2001/10/03 05:53:25  robertj
 * Update to new PTLib channel error system.
 *
 * Revision 2.0  2001/07/27 15:48:25  robertj
 * Conversion of OpenH323 to Open Phone Abstraction Library (OPAL)
 *
 */

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "transports.h"
#endif

#include <opal/transports.h>

#include <opal/manager.h>
#include <opal/endpoint.h>
#include <opal/call.h>
#include <opal/buildopts.h>

#include <ptclib/pstun.h>


static const char IpPrefix[]  = "ip$";   // For backward compatibility with OpenH323
static const char TcpPrefix[] = "tcp$";
static const char UdpPrefix[] = "udp$";

static PFactory<OpalInternalTransport>::Worker<OpalInternalTCPTransport> opalInternalTCPTransportFactory(TcpPrefix, true);
static PFactory<OpalInternalTransport>::Worker<OpalInternalTCPTransport>  opalInternalIPTransportFactory(IpPrefix, true);
static PFactory<OpalInternalTransport>::Worker<OpalInternalUDPTransport> opalInternalUDPTransportFactory(UdpPrefix, true);

#if P_SSL
#include <ptclib/pssl.h>
static const char TcpsPrefix[] = "tcps$";
static PFactory<OpalInternalTransport>::Worker<OpalInternalTCPSTransport> opalInternalTCPSTransportFactory(TcpsPrefix, true);
#endif

/////////////////////////////////////////////////////////////////

#define new PNEW

/////////////////////////////////////////////////////////////////

OpalTransportAddress::OpalTransportAddress()
{
  transport = NULL;
}


OpalTransportAddress::OpalTransportAddress(const char * cstr,
                                           WORD port,
                                           const char * proto)
  : PString(cstr)
{
  SetInternalTransport(port, proto);
}


OpalTransportAddress::OpalTransportAddress(const PString & str,
                                           WORD port,
                                           const char * proto)
  : PString(str)
{
  SetInternalTransport(port, proto);
}


OpalTransportAddress::OpalTransportAddress(const PIPSocket::Address & addr, WORD port, const char * proto)
  : PString(addr.AsString())
{
  SetInternalTransport(port, proto);
}


PString OpalTransportAddress::GetHostName() const
{
  if (transport == NULL)
    return *this;

  return transport->GetHostName(*this);
}
  

PBoolean OpalTransportAddress::IsEquivalent(const OpalTransportAddress & address) const
{
  if (*this == address)
    return PTrue;

  if (IsEmpty() || address.IsEmpty())
    return PFalse;

  PIPSocket::Address ip1, ip2;
  WORD port1 = 65535, port2 = 65535;
  return GetIpAndPort(ip1, port1) &&
         address.GetIpAndPort(ip2, port2) &&
         (ip1.IsAny() || ip2.IsAny() || (ip1 *= ip2)) &&
         (port1 == 65535 || port2 == 65535 || port1 == port2);
}


PBoolean OpalTransportAddress::IsCompatible(const OpalTransportAddress & address) const
{
  if (IsEmpty() || address.IsEmpty())
    return PTrue;

  PCaselessString myPrefix = Left(Find('$'));
  PCaselessString theirPrefix = address.Left(address.Find('$'));
  return myPrefix == theirPrefix ||
        (myPrefix    == IpPrefix && (theirPrefix == TcpPrefix || theirPrefix == UdpPrefix)) ||
        (theirPrefix == IpPrefix && (myPrefix    == TcpPrefix || myPrefix    == UdpPrefix));
}


PBoolean OpalTransportAddress::GetIpAddress(PIPSocket::Address & ip) const
{
  if (transport == NULL)
    return PFalse;

  WORD dummy = 65535;
  return transport->GetIpAndPort(*this, ip, dummy);
}


PBoolean OpalTransportAddress::GetIpAndPort(PIPSocket::Address & ip, WORD & port) const
{
  if (transport == NULL)
    return PFalse;

  return transport->GetIpAndPort(*this, ip, port);
}


OpalListener * OpalTransportAddress::CreateListener(OpalEndPoint & endpoint,
                                                    BindOptions option) const
{
  if (transport == NULL)
    return NULL;

  return transport->CreateListener(*this, endpoint, option);
}


OpalTransport * OpalTransportAddress::CreateTransport(OpalEndPoint & endpoint,
                                                      BindOptions option) const
{
  if (transport == NULL)
    return NULL;

  return transport->CreateTransport(*this, endpoint, option);
}


void OpalTransportAddress::SetInternalTransport(WORD port, const char * proto)
{
  transport = NULL;
  
  if (IsEmpty())
    return;

  PINDEX dollar = Find('$');
  if (dollar == P_MAX_INDEX) {
    PString prefix(proto == NULL ? TcpPrefix : proto);
    if (prefix.Find('$') == P_MAX_INDEX)
      prefix += '$';

    Splice(prefix, 0);
    dollar = prefix.GetLength()-1;
  }

  // use factory to create transport types
  transport = PFactory<OpalInternalTransport>::CreateInstance(Left(dollar+1));
  if (transport == NULL)
    return;

  if (port != 0 && Find(':', dollar) == P_MAX_INDEX)
    sprintf(":%u", port);
}


/////////////////////////////////////////////////////////////////

void OpalTransportAddressArray::AppendString(const char * str)
{
  AppendAddress(OpalTransportAddress(str));
}


void OpalTransportAddressArray::AppendString(const PString & str)
{
  AppendAddress(OpalTransportAddress(str));
}


void OpalTransportAddressArray::AppendAddress(const OpalTransportAddress & addr)
{
  if (!addr)
    Append(new OpalTransportAddress(addr));
}


void OpalTransportAddressArray::AppendStringCollection(const PCollection & coll)
{
  for (PINDEX i = 0; i < coll.GetSize(); i++) {
    PObject * obj = coll.GetAt(i);
    if (obj != NULL && PIsDescendant(obj, PString))
      AppendAddress(OpalTransportAddress(*(PString *)obj));
  }
}


/////////////////////////////////////////////////////////////////

PString OpalInternalTransport::GetHostName(const OpalTransportAddress & address) const
{
  // skip transport identifier
  PINDEX pos = address.Find('$');
  if (pos == P_MAX_INDEX)
    return address;

  return address.Mid(pos+1);
}


PBoolean OpalInternalTransport::GetIpAndPort(const OpalTransportAddress &,
                                         PIPSocket::Address &,
                                         WORD &) const
{
  return PFalse;
}


//////////////////////////////////////////////////////////////////////////

static PBoolean SplitAddress(const PString & addr, PString & host, PString & service)
{
  // skip transport identifier
  PINDEX dollar = addr.Find('$');
  if (dollar == P_MAX_INDEX)
    return PFalse;
  
  PINDEX lastChar = addr.GetLength()-1;
  if (addr[lastChar] == '+')
    lastChar--;

  PINDEX bracket = addr.FindLast(']');
  if (bracket == P_MAX_INDEX)
    bracket = 0;

  PINDEX colon = addr.Find(':', bracket);
  if (colon == P_MAX_INDEX)
    host = addr(dollar+1, lastChar);
  else {
    host = addr(dollar+1, colon-1);
    service = addr(colon+1, lastChar);
  }

  return PTrue;
}


PString OpalInternalIPTransport::GetHostName(const OpalTransportAddress & address) const
{
  PString host, service;
  if (!SplitAddress(address, host, service))
    return address;

  PIPSocket::Address ip;
  if (PIPSocket::GetHostAddress(host, ip))
    return ip.AsString();

  return host;
}


PBoolean OpalInternalIPTransport::GetIpAndPort(const OpalTransportAddress & address,
                                           PIPSocket::Address & ip,
                                           WORD & port) const
{
  PString host, service;
  if (!SplitAddress(address, host, service))
    return PFalse;

  if (host.IsEmpty()) {
    PTRACE(2, "Opal\tIllegal IP transport address: \"" << address << '"');
    return PFalse;
  }

  if (service == "*")
    port = 0;
  else {
    if (!service) {
      PString proto = address.Left(address.Find('$'));
      if (proto *= "ip")
        proto = "tcp";
      port = PIPSocket::GetPortByService(proto, service);
    }
    if (port == 0) {
      PTRACE(2, "Opal\tIllegal IP transport port/service: \"" << address << '"');
      return PFalse;
    }
  }

  if (host[0] == '*' || host == "0.0.0.0") {
    ip = PIPSocket::GetDefaultIpAny();
    return PTrue;
  }

  if (PIPSocket::GetHostAddress(host, ip))
    return PTrue;

  PTRACE(1, "Opal\tCould not find host : \"" << host << '"');
  return PFalse;
}


//////////////////////////////////////////////////////////////////////////

PBoolean OpalInternalIPTransport::GetAdjustedIpAndPort(const OpalTransportAddress & address,
                                 OpalEndPoint & endpoint,
                                 OpalTransportAddress::BindOptions option,
                                 PIPSocket::Address & ip,
                                 WORD & port,
                                 PBoolean & reuseAddr)
{
  reuseAddr = address[address.GetLength()-1] == '+';

  switch (option) {
    case OpalTransportAddress::NoBinding :
      ip = PIPSocket::GetDefaultIpAny();
      port = 0;
      return PTrue;

    case OpalTransportAddress::HostOnly :
      port = 0;
      return address.GetIpAddress(ip);

    default :
      port = endpoint.GetDefaultSignalPort();
      return address.GetIpAndPort(ip, port);
  }
}


//////////////////////////////////////////////////////////////////////////

OpalListener::OpalListener(OpalEndPoint & ep)
  : endpoint(ep)
{
  thread = NULL;
  threadMode = SpawnNewThreadMode;
}


void OpalListener::PrintOn(ostream & strm) const
{
  strm << GetLocalAddress();
}


void OpalListener::CloseWait()
{
  PTRACE(3, "Listen\tStopping listening thread on " << GetLocalAddress());
  Close();

  PAssert(PThread::Current() != thread, PLogicError);
  if (thread != NULL) {
    PAssert(thread->WaitForTermination(10000), "Listener thread did not terminate");
    delete thread;
    thread = NULL;
  }
}


void OpalListener::ListenForConnections(PThread & thread, INT)
{
  PTRACE(3, "Listen\tStarted listening thread on " << GetLocalAddress());
  PAssert(!acceptHandler.IsNULL(), PLogicError);

  while (IsOpen()) {
    OpalTransport * transport = Accept(PMaxTimeInterval);
    if (transport == NULL)
      acceptHandler(*this, 0);
    else {
      switch (threadMode) {
        case SpawnNewThreadMode :
          transport->AttachThread(PThread::Create(acceptHandler,
                                                  (INT)transport,
                                                  PThread::NoAutoDeleteThread,
                                                  PThread::NormalPriority,
                                                  "Opal Answer:%x"));
          break;

        case HandOffThreadMode :
          transport->AttachThread(&thread);
          // Then do next case

        case SingleThreadMode :
          acceptHandler(*this, (INT)transport);
      }
      // Note: acceptHandler is responsible for deletion of the transport
    }
  }
}


PBoolean OpalListener::StartThread(const PNotifier & theAcceptHandler, ThreadMode mode)
{
  acceptHandler = theAcceptHandler;
  threadMode = mode;

  thread = PThread::Create(PCREATE_NOTIFIER(ListenForConnections), 0,
                           PThread::NoAutoDeleteThread,
                           PThread::NormalPriority,
                           "Opal Listener:%x");

  return thread != NULL;
}


//////////////////////////////////////////////////////////////////////////

OpalTransportAddressArray OpalGetInterfaceAddresses(const OpalListenerList & listeners,
                                                    PBoolean excludeLocalHost,
                                                    OpalTransport * associatedTransport)
{
  OpalTransportAddressArray interfaceAddresses;

  PINDEX i;
  for (i = 0; i < listeners.GetSize(); i++) {
    OpalTransportAddressArray newAddrs = OpalGetInterfaceAddresses(listeners[i].GetTransportAddress(), excludeLocalHost, associatedTransport);
    PINDEX size  = interfaceAddresses.GetSize();
    PINDEX nsize = newAddrs.GetSize();
    interfaceAddresses.SetSize(size + nsize);
    PINDEX j;
    for (j = 0; j < nsize; j++)
      interfaceAddresses.SetAt(size + j, new OpalTransportAddress(newAddrs[j]));
  }

  return interfaceAddresses;
}


OpalTransportAddressArray OpalGetInterfaceAddresses(const OpalTransportAddress & addr,
                                                    PBoolean excludeLocalHost,
                                                    OpalTransport * associatedTransport)
{
  PIPSocket::Address ip;
  WORD port = 0;
  if (!addr.GetIpAndPort(ip, port) || !ip.IsAny())
    return addr;

  PIPSocket::InterfaceTable interfaces;
  if (!PIPSocket::GetInterfaceTable(interfaces))
    return addr;

  if (interfaces.GetSize() == 1)
    return OpalTransportAddress(interfaces[0].GetAddress(), port);

  PINDEX i;
  OpalTransportAddressArray interfaceAddresses;
  PIPSocket::Address firstAddress(0);

  if (associatedTransport != NULL) {
    if (associatedTransport->GetLocalAddress().GetIpAddress(firstAddress)) {
      for (i = 0; i < interfaces.GetSize(); i++) {
        PIPSocket::Address ip = interfaces[i].GetAddress();
        if (ip == firstAddress)
          interfaceAddresses.Append(new OpalTransportAddress(ip, port));
      }
    }
  }

  for (i = 0; i < interfaces.GetSize(); i++) {
    PIPSocket::Address ip = interfaces[i].GetAddress();
    if (ip != firstAddress && !(excludeLocalHost && ip.IsLoopback()))
      interfaceAddresses.Append(new OpalTransportAddress(ip, port));
  }

  return interfaceAddresses;
}


//////////////////////////////////////////////////////////////////////////

OpalListenerIP::OpalListenerIP(OpalEndPoint & ep,
                               PIPSocket::Address binding,
                               WORD port,
                               PBoolean exclusive)
  : OpalListener(ep),
    localAddress(binding)
{
  listenerPort = port;
  exclusiveListener = exclusive;
}


OpalListenerIP::OpalListenerIP(OpalEndPoint & endpoint,
                               const OpalTransportAddress & binding,
                               OpalTransportAddress::BindOptions option)
  : OpalListener(endpoint)
{
  OpalInternalIPTransport::GetAdjustedIpAndPort(binding, endpoint, option, localAddress, listenerPort, exclusiveListener);
}


OpalTransportAddress OpalListenerIP::GetLocalAddress(const OpalTransportAddress & preferredAddress) const
{
  PString addr;

  // If specifically bound to interface use that
  if (!localAddress.IsAny())
    addr = localAddress.AsString();
  else {
    // If bound to all, then use '*' unless a preferred address is specified
    addr = "*";

    PIPSocket::Address ip;
    if (preferredAddress.GetIpAddress(ip)) {
      // Verify preferred address is actually an interface in this machine!
      PIPSocket::InterfaceTable interfaces;
      if (PIPSocket::GetInterfaceTable(interfaces)) {
        for (PINDEX i = 0; i < interfaces.GetSize(); i++) {
          if (interfaces[i].GetAddress() == ip) {
            addr = ip.AsString();
            break;
          }
        }
      }
    }
  }

  addr.sprintf(":%u", listenerPort);

  return GetProtoPrefix() + addr;
}


//////////////////////////////////////////////////////////////////////////

OpalListenerTCP::OpalListenerTCP(OpalEndPoint & ep,
                                 PIPSocket::Address binding,
                                 WORD port,
                                 PBoolean exclusive)
  : OpalListenerIP(ep, binding, port, exclusive)
{
}


OpalListenerTCP::OpalListenerTCP(OpalEndPoint & endpoint,
                                 const OpalTransportAddress & binding,
                                 OpalTransportAddress::BindOptions option)
  : OpalListenerIP(endpoint, binding, option)
{
}


OpalListenerTCP::~OpalListenerTCP()
{
  CloseWait();
}


PBoolean OpalListenerTCP::Open(const PNotifier & theAcceptHandler, ThreadMode mode)
{
  if (listenerPort == 0) {
    OpalManager & manager = endpoint.GetManager();
    listenerPort = manager.GetNextTCPPort();
    WORD firstPort = listenerPort;
    while (!listener.Listen(localAddress, 1, listenerPort)) {
      listenerPort = manager.GetNextTCPPort();
      if (listenerPort == firstPort) {
        PTRACE(1, "Listen\tOpen on " << localAddress << " failed: " << listener.GetErrorText());
        break;
      }
    }
    listenerPort = listener.GetPort();
    return StartThread(theAcceptHandler, mode);
  }

  if (listener.Listen(localAddress, 1, listenerPort))
    return StartThread(theAcceptHandler, mode);

  if (exclusiveListener) {
    PTRACE(1, "Listen\tOpen on " << localAddress << ':' << listener.GetPort()
           << " failed: " << listener.GetErrorText());
    return PFalse;
  }

  if (listener.GetErrorNumber() != EADDRINUSE)
    return PFalse;

  PTRACE(1, "Listen\tSocket for " << localAddress << ':' << listener.GetPort()
         << " already in use, incoming connections may not all be serviced!");

  if (listener.Listen(localAddress, 100, 0, PSocket::CanReuseAddress))
    return StartThread(theAcceptHandler, mode);

  PTRACE(1, "Listen\tOpen (REUSEADDR) on " << localAddress << ':' << listener.GetPort()
         << " failed: " << listener.GetErrorText());
  return PFalse;
}


PBoolean OpalListenerTCP::IsOpen()
{
  return listener.IsOpen();
}


void OpalListenerTCP::Close()
{
  listener.Close();
}


OpalTransport * OpalListenerTCP::Accept(const PTimeInterval & timeout)
{
  if (!listener.IsOpen())
    return NULL;

  listener.SetReadTimeout(timeout); // Wait for remote connect

  PTRACE(4, "Listen\tWaiting on socket accept on " << GetLocalAddress());
  PTCPSocket * socket = new PTCPSocket;
  if (socket->Accept(listener)) {
    OpalTransportTCP * transport = new OpalTransportTCP(endpoint);
    if (transport->Open(socket))
      return transport;

    PTRACE(1, "Listen\tFailed to open transport, connection not started.");
    delete transport;
    return NULL;
  }
  else if (socket->GetErrorCode() != PChannel::Interrupted) {
    PTRACE(1, "Listen\tAccept error:" << socket->GetErrorText());
    listener.Close();
  }

  delete socket;
  return NULL;
}



OpalTransport * OpalListenerTCP::CreateTransport(const OpalTransportAddress & localAddress,
                                                 const OpalTransportAddress & remoteAddress) const
{
  OpalTransportAddress myLocalAddress = GetLocalAddress();
  if (myLocalAddress.IsCompatible(remoteAddress) && myLocalAddress.IsCompatible(remoteAddress))
    return localAddress.IsEmpty() ? new OpalTransportTCP(endpoint) : localAddress.CreateTransport(endpoint, OpalTransportAddress::NoBinding);
  return NULL;
}


const char * OpalListenerTCP::GetProtoPrefix() const
{
  return TcpPrefix;
}


//////////////////////////////////////////////////////////////////////////

OpalListenerUDP::OpalListenerUDP(OpalEndPoint & endpoint,
                                 PIPSocket::Address binding,
                                 WORD port,
                                 PBoolean exclusive)
  : OpalListenerIP(endpoint, binding, port, exclusive),
    listenerBundle(PMonitoredSockets::Create(binding.AsString(), !exclusive, endpoint.GetManager().GetSTUN()))
{
}


OpalListenerUDP::OpalListenerUDP(OpalEndPoint & endpoint,
                                 const OpalTransportAddress & binding,
                                 OpalTransportAddress::BindOptions option)
  : OpalListenerIP(endpoint, binding, option),
    listenerBundle(PMonitoredSockets::Create(binding.GetHostName(), !exclusiveListener, endpoint.GetManager().GetSTUN()))
{
}


OpalListenerUDP::~OpalListenerUDP()
{
  CloseWait();
}


PBoolean OpalListenerUDP::Open(const PNotifier & theAcceptHandler, ThreadMode /*mode*/)
{
  if (listenerBundle->Open(listenerPort))
    return StartThread(theAcceptHandler, SingleThreadMode);

  PTRACE(1, "Listen\tCould not start any UDP listeners");
  return PFalse;
}


PBoolean OpalListenerUDP::IsOpen()
{
  return listenerBundle != NULL && listenerBundle->IsOpen();
}


void OpalListenerUDP::Close()
{
  if (listenerBundle != NULL)
    listenerBundle->Close();
}


OpalTransport * OpalListenerUDP::Accept(const PTimeInterval & timeout)
{
  if (!IsOpen())
    return NULL;

  PBYTEArray pdu;
  PIPSocket::Address remoteAddr;
  WORD remotePort;
  PString iface;
  PINDEX readCount;
  switch (listenerBundle->ReadFromBundle(pdu.GetPointer(2000), 2000, remoteAddr, remotePort, iface, readCount, timeout)) {
    case PChannel::NoError :
      pdu.SetSize(readCount);
      return new OpalTransportUDP(endpoint, pdu, listenerBundle, iface, remoteAddr, remotePort);

    case PChannel::Interrupted :
      PTRACE(4, "Listen\tDropped interface " << iface);
      break;

    default :
      PTRACE(1, "Listen\tUDP read error.");
  }

  return NULL;
}


OpalTransport * OpalListenerUDP::CreateTransport(const OpalTransportAddress & localAddress,
                                                 const OpalTransportAddress & remoteAddress) const
{
  if (!GetLocalAddress().IsCompatible(remoteAddress))
    return NULL;

  PIPSocket::Address addr;
  if (remoteAddress.GetIpAddress(addr) && addr.IsLoopback())
    return new OpalTransportUDP(endpoint, addr);

  PString iface;
  if (localAddress.GetIpAddress(addr))
    iface = addr.AsString();
  return new OpalTransportUDP(endpoint, PBYTEArray(), listenerBundle, iface, PIPSocket::GetDefaultIpAny(), 0);
}


const char * OpalListenerUDP::GetProtoPrefix() const
{
  return UdpPrefix;
}


//////////////////////////////////////////////////////////////////////////

OpalTransport::OpalTransport(OpalEndPoint & end)
  : endpoint(end)
{
  thread = NULL;
}


OpalTransport::~OpalTransport()
{
  PAssert(thread == NULL, PLogicError);
}


void OpalTransport::PrintOn(ostream & strm) const
{
  strm << GetRemoteAddress() << "<if=" << GetLocalAddress() << '>';
}


PString OpalTransport::GetInterface() const
{
  return GetLocalAddress().GetHostName();
}


void OpalTransport::EndConnect(const PString &)
{
}


PBoolean OpalTransport::Close()
{
  PTRACE(4, "Opal\tTransport Close");

  /* Do not use PIndirectChannel::Close() as this deletes the sub-channel
     member field crashing the background thread. Just close the base
     sub-channel so breaks the threads I/O block.
   */
  if (IsOpen())
    return GetBaseWriteChannel()->Close();

  return PTrue;
}


void OpalTransport::CloseWait()
{
  PTRACE(3, "Opal\tTransport clean up on termination");

  Close();

  if (thread != NULL) {
    PAssert(thread->WaitForTermination(10000), "Transport thread did not terminate");
    if (thread == PThread::Current())
      thread->SetAutoDelete();
    else
      delete thread;
    thread = NULL;
  }
}


PBoolean OpalTransport::IsCompatibleTransport(const OpalTransportAddress &) const
{
  PAssertAlways(PUnimplementedFunction);
  return PFalse;
}


void OpalTransport::SetPromiscuous(PromisciousModes /*promiscuous*/)
{
}


OpalTransportAddress OpalTransport::GetLastReceivedAddress() const
{
  return GetRemoteAddress();
}


PBoolean OpalTransport::WriteConnect(WriteConnectCallback function, void * userData)
{
  return function(*this, userData);
}


void OpalTransport::AttachThread(PThread * thrd)
{
  if (thread != NULL) {
    PAssert(thread->WaitForTermination(10000), "Transport not terminated when reattaching thread");
    delete thread;
  }

  thread = thrd;
}


PBoolean OpalTransport::IsRunning() const
{
  if (thread == NULL)
    return PFalse;

  return !thread->IsTerminated();
}


/////////////////////////////////////////////////////////////////////////////

OpalTransportIP::OpalTransportIP(OpalEndPoint & end,
                                 PIPSocket::Address binding,
                                 WORD port)
  : OpalTransport(end),
    localAddress(binding),
    remoteAddress(0)
{
  localPort = port;
  remotePort = 0;
}


OpalTransportAddress OpalTransportIP::GetLocalAddress() const
{
  return OpalTransportAddress(localAddress, localPort, GetProtoPrefix());
}


PBoolean OpalTransportIP::SetLocalAddress(const OpalTransportAddress & newLocalAddress)
{
  if (!IsCompatibleTransport(newLocalAddress))
    return PFalse;

  if (!IsOpen())
    return newLocalAddress.GetIpAndPort(localAddress, localPort);

  PIPSocket::Address address;
  WORD port = 0;
  if (!newLocalAddress.GetIpAndPort(address, port))
    return PFalse;

  return localAddress == address && localPort == port;
}


OpalTransportAddress OpalTransportIP::GetRemoteAddress() const
{
  return OpalTransportAddress(remoteAddress, remotePort, GetProtoPrefix());
}


PBoolean OpalTransportIP::SetRemoteAddress(const OpalTransportAddress & address)
{
  if (IsCompatibleTransport(address))
    return address.GetIpAndPort(remoteAddress, remotePort);

  return PFalse;
}


/////////////////////////////////////////////////////////////////////////////

OpalTransportTCP::OpalTransportTCP(OpalEndPoint & ep,
                                   PIPSocket::Address binding,
                                   WORD port,
                                   PBoolean reuseAddr)
  : OpalTransportIP(ep, binding, port)
{
  reuseAddressFlag = reuseAddr;
}


OpalTransportTCP::OpalTransportTCP(OpalEndPoint & ep, PTCPSocket * socket)
  : OpalTransportIP(ep, INADDR_ANY, 0)
{
  Open(socket);
}


OpalTransportTCP::~OpalTransportTCP()
{
  CloseWait();
  PTRACE(4,"Opal\tDeleted transport " << *this);
}


PBoolean OpalTransportTCP::IsReliable() const
{
  return PTrue;
}


PBoolean OpalTransportTCP::IsCompatibleTransport(const OpalTransportAddress & address) const
{
  return (address.NumCompare(TcpPrefix) == EqualTo) ||
         (address.NumCompare(IpPrefix)  == EqualTo);
}


PBoolean OpalTransportTCP::Connect()
{
  if (IsOpen())
    return PTrue;

  PTCPSocket * socket = new PTCPSocket(remotePort);
  Open(socket);

  PReadWaitAndSignal mutex(channelPointerMutex);

  socket->SetReadTimeout(10000);

  OpalManager & manager = endpoint.GetManager();
  localPort = manager.GetNextTCPPort();
  WORD firstPort = localPort;
  for (;;) {
    PTRACE(4, "OpalTCP\tConnecting to "
           << remoteAddress << ':' << remotePort
           << " (local port=" << localPort << ')');
    if (socket->Connect(localPort, remoteAddress))
      break;

    int errnum = socket->GetErrorNumber();
    if (localPort == 0 || (errnum != EADDRINUSE && errnum != EADDRNOTAVAIL)) {
      PTRACE(1, "OpalTCP\tCould not connect to "
                << remoteAddress << ':' << remotePort
                << " (local port=" << localPort << ") - "
                << socket->GetErrorText() << '(' << errnum << ')');
      return SetErrorValues(socket->GetErrorCode(), errnum);
    }

    localPort = manager.GetNextTCPPort();
    if (localPort == firstPort) {
      PTRACE(1, "OpalTCP\tCould not bind to any port in range " <<
                manager.GetTCPPortBase() << " to " << manager.GetTCPPortMax());
      return SetErrorValues(socket->GetErrorCode(), errnum);
    }
  }

  socket->SetReadTimeout(PMaxTimeInterval);

  return OnOpen();
}


PBoolean OpalTransportTCP::ReadPDU(PBYTEArray & pdu)
{
  // Make sure is a RFC1006 TPKT
  switch (ReadChar()) {
    case 3 :  // Only support version 3
      break;

    default :  // Unknown version number
      SetErrorValues(ProtocolFailure, 0x80000000);
      // Do case for read error

    case -1 :
      return PFalse;
  }

  // Save timeout
  PTimeInterval oldTimeout = GetReadTimeout();

  // Should get all of PDU in 5 seconds or something is seriously wrong,
  SetReadTimeout(5000);

  // Get TPKT length
  BYTE header[3];
  PBoolean ok = ReadBlock(header, sizeof(header));
  if (ok) {
    PINDEX packetLength = ((header[1] << 8)|header[2]);
    if (packetLength < 4) {
      PTRACE(2, "H323TCP\tDwarf PDU received (length " << packetLength << ")");
      ok = PFalse;
    } else {
      packetLength -= 4;
      ok = ReadBlock(pdu.GetPointer(packetLength), packetLength);
    }
  }

  SetReadTimeout(oldTimeout);

  return ok;
}


PBoolean OpalTransportTCP::WritePDU(const PBYTEArray & pdu)
{
  // We copy the data into a new buffer so we can do a single write call. This
  // is necessary as we have disabled the Nagle TCP delay algorithm to improve
  // network performance.

  int packetLength = pdu.GetSize() + 4;

  // Send RFC1006 TPKT length
  PBYTEArray tpkt(packetLength);
  tpkt[0] = 3;
  tpkt[1] = 0;
  tpkt[2] = (BYTE)(packetLength >> 8);
  tpkt[3] = (BYTE)packetLength;
  memcpy(tpkt.GetPointer()+4, (const BYTE *)pdu, pdu.GetSize());

  return Write((const BYTE *)tpkt, packetLength);
}


PBoolean OpalTransportTCP::OnOpen()
{
  PIPSocket * socket = (PIPSocket *)GetReadChannel();

  // Get name of the remote computer for information purposes
  if (!socket->GetPeerAddress(remoteAddress, remotePort)) {
    PTRACE(1, "OpalTCP\tGetPeerAddress() failed: " << socket->GetErrorText());
    return PFalse;
  }

  // get local address of incoming socket to ensure that multi-homed machines
  // use a NIC address that is guaranteed to be addressable to destination
  if (!socket->GetLocalAddress(localAddress, localPort)) {
    PTRACE(1, "OpalTCP\tGetLocalAddress() failed: " << socket->GetErrorText());
    return PFalse;
  }

#ifndef __BEOS__
  if (!socket->SetOption(TCP_NODELAY, 1, IPPROTO_TCP)) {
    PTRACE(1, "OpalTCP\tSetOption(TCP_NODELAY) failed: " << socket->GetErrorText());
  }

  // make sure do not lose outgoing packets on close
  const linger ling = { 1, 3 };
  if (!socket->SetOption(SO_LINGER, &ling, sizeof(ling))) {
    PTRACE(1, "OpalTCP\tSetOption(SO_LINGER) failed: " << socket->GetErrorText());
    return PFalse;
  }
#endif

  PTRACE(3, "OpalTCP\tStarted connection to "
         << remoteAddress << ':' << remotePort
         << " (if=" << localAddress << ':' << localPort << ')');

  return PTrue;
}


const char * OpalTransportTCP::GetProtoPrefix() const
{
  return TcpPrefix;
}


/////////////////////////////////////////////////////////////////////////////

OpalTransportUDP::OpalTransportUDP(OpalEndPoint & ep,
                                   PIPSocket::Address binding,
                                   WORD localPort,
                                   PBoolean reuseAddr)
  : OpalTransportIP(ep, binding, localPort)
  , manager(ep.GetManager())
{
  PMonitoredSockets * sockets = PMonitoredSockets::Create(binding.AsString(), reuseAddr);
  if (sockets->Open(localPort)) {
    Open(new PMonitoredSocketChannel(sockets, PFalse));
    PTRACE(3, "OpalUDP\tBinding to interface: " << localAddress << ':' << localPort);
  }
  else {
    PTRACE(1, "OpalUDP\tCould not bind to interface: " << localAddress << ':' << localPort);
  }
}


OpalTransportUDP::OpalTransportUDP(OpalEndPoint & ep,
                                   const PBYTEArray & packet,
                                   const PMonitoredSocketsPtr & listener,
                                   const PString & iface,
                                   PIPSocket::Address remAddr,
                                   WORD remPort)
  : OpalTransportIP(ep, PIPSocket::GetDefaultIpAny(), 0)
  , manager(ep.GetManager())
  , preReadPacket(packet)
{
  remoteAddress = remAddr;
  remotePort = remPort;

  PMonitoredSocketChannel * socket = new PMonitoredSocketChannel(listener, PTrue);
  socket->SetRemote(remAddr, remPort);
  socket->SetInterface(iface);
  socket->GetLocal(localAddress, localPort, !manager.IsLocalAddress(remoteAddress));
  Open(socket);

  PTRACE(3, "OpalUDP\tBinding to interface: " << localAddress << ':' << localPort);
}


OpalTransportUDP::~OpalTransportUDP()
{
  CloseWait();
  PTRACE(4,"Opal\tDeleted transport " << *this);
}


PBoolean OpalTransportUDP::IsReliable() const
{
  return PFalse;
}


PBoolean OpalTransportUDP::IsCompatibleTransport(const OpalTransportAddress & address) const
{
  return (address.NumCompare(UdpPrefix) == EqualTo) ||
         (address.NumCompare(IpPrefix)  == EqualTo);
}


PBoolean OpalTransportUDP::Connect()
{	
  if (remotePort == 0)
    return PFalse;

  if (remoteAddress.IsAny() || remoteAddress.IsBroadcast()) {
    remoteAddress = PIPSocket::Address::GetBroadcast();
    PTRACE(3, "OpalUDP\tBroadcast connect to port " << remotePort);
  }
  else {
    PTRACE(3, "OpalUDP\tStarted connect to " << remoteAddress << ':' << remotePort);
  }

  if (PAssertNULL(writeChannel) == NULL)
    return PFalse;

  PMonitoredSocketsPtr bundle = ((PMonitoredSocketChannel *)writeChannel)->GetMonitoredSockets();
  if (bundle->IsOpen())
    return PTrue;

  OpalManager & manager = endpoint.GetManager();

  bundle->SetSTUN(manager.GetSTUN(remoteAddress));

  localPort = manager.GetNextUDPPort();
  WORD firstPort = localPort;
  while (!bundle->Open(localPort)) {
    localPort = manager.GetNextUDPPort();
    if (localPort == firstPort) {
      PTRACE(1, "OpalUDP\tCould not bind to any port in range " <<
	      manager.GetUDPPortBase() << " to " << manager.GetUDPPortMax());
      return PFalse;
    }
  }

  return PTrue;
}


PString OpalTransportUDP::GetInterface() const
{
  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket != NULL)
    return socket->GetInterface();

  return OpalTransportIP::GetInterface();
}


void OpalTransportUDP::EndConnect(const PString & iface)
{
  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket != NULL)
    socket->SetInterface(iface);

  OpalTransportIP::EndConnect(iface);
}


OpalTransportAddress OpalTransportUDP::GetLocalAddress() const
{
  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket != NULL) {
    OpalTransportUDP * thisWritable = const_cast<OpalTransportUDP *>(this);
    socket->GetLocal(thisWritable->localAddress, thisWritable->localPort, !manager.IsLocalAddress(remoteAddress));
  }
  return OpalTransportIP::GetLocalAddress();
}


PBoolean OpalTransportUDP::SetLocalAddress(const OpalTransportAddress & newLocalAddress)
{
  if (OpalTransportIP::GetLocalAddress().IsEquivalent(newLocalAddress))
    return PTrue;

  if (!IsCompatibleTransport(newLocalAddress))
    return PFalse;

  if (!newLocalAddress.GetIpAndPort(localAddress, localPort))
    return PFalse;

  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket != NULL)
    socket->GetMonitoredSockets()->Open(localPort);

  return OpalTransportIP::SetLocalAddress(newLocalAddress);
}


PBoolean OpalTransportUDP::SetRemoteAddress(const OpalTransportAddress & address)
{
  if (!OpalTransportIP::SetRemoteAddress(address))
    return PFalse;

  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket != NULL)
    socket->SetRemote(remoteAddress, remotePort);

  return PTrue;
}


void OpalTransportUDP::SetPromiscuous(PromisciousModes promiscuous)
{
  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket != NULL) {
    socket->SetPromiscuous(promiscuous != AcceptFromRemoteOnly);
    if (promiscuous == AcceptFromAnyAutoSet)
      socket->SetRemote(PIPSocket::GetDefaultIpAny(), 0);
  }
}


OpalTransportAddress OpalTransportUDP::GetLastReceivedAddress() const
{
  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)readChannel;
  if (socket == NULL)
    return OpalTransport::GetLastReceivedAddress();

  PIPSocket::Address addr;
  WORD port;
  socket->GetLastReceived(addr, port);
  if (addr.IsAny() || port == 0)
    return OpalTransport::GetLastReceivedAddress();

  return OpalTransportAddress(addr, port, UdpPrefix);
}


PBoolean OpalTransportUDP::Read(void * buffer, PINDEX length)
{
  if (preReadPacket.GetSize() > 0) {
    lastReadCount = PMIN(length, preReadPacket.GetSize());
    memcpy(buffer, preReadPacket, lastReadCount);
    preReadPacket.SetSize(0);
    return PTrue;
  }

  return OpalTransportIP::Read(buffer, length);
}


PBoolean OpalTransportUDP::ReadPDU(PBYTEArray & packet)
{
  if (preReadPacket.GetSize() > 0) {
    packet = preReadPacket;
    preReadPacket.SetSize(0);
    return PTrue;
  }

  if (!Read(packet.GetPointer(10000), 10000)) {
    packet.SetSize(0);
    return PFalse;
  }

  packet.SetSize(GetLastReadCount());
  return PTrue;
}


PBoolean OpalTransportUDP::WritePDU(const PBYTEArray & packet)
{
  return Write((const BYTE *)packet, packet.GetSize());
}


PBoolean OpalTransportUDP::WriteConnect(WriteConnectCallback function, void * userData)
{
  PMonitoredSocketChannel * socket = (PMonitoredSocketChannel *)writeChannel;
  if (socket == NULL)
    return PFalse;

  PMonitoredSocketsPtr bundle = socket->GetMonitoredSockets();
  PIPSocket::Address address;
  GetRemoteAddress().GetIpAddress(address);
  PStringArray interfaces = bundle->GetInterfaces(PFalse, address);

  PBoolean ok = PFalse;
  for (PINDEX i = 0; i < interfaces.GetSize(); i++) {
    PTRACE(4, "OpalUDP\tWriting to interface " << i << " - \"" << interfaces[i] << '"');
    socket->SetInterface(interfaces[i]);
    if (function(*this, userData))
      ok = PTrue;
  }

  return ok;
}


const char * OpalTransportUDP::GetProtoPrefix() const
{
  return UdpPrefix;
}


//////////////////////////////////////////////////////////////////////////

#if P_SSL

#include <ptclib/pssl.h>

static PBoolean SetSSLCertificate(PSSLContext & sslContext,
                             const PFilePath & certificateFile,
                                        PBoolean create,
                                   const char * dn = NULL)
{
  if (create && !PFile::Exists(certificateFile)) {
    PSSLPrivateKey key(1024);
    PSSLCertificate certificate;
    PStringStream name;
    if (dn != NULL)
      name << dn;
    else {
      name << "/O=" << PProcess::Current().GetManufacturer()
           << "/CN=" << PProcess::Current().GetName() << '@' << PIPSocket::GetHostName();
    }
    if (!certificate.CreateRoot(name, key)) {
      PTRACE(0, "MTGW\tCould not create certificate");
      return PFalse;
    }
    certificate.Save(certificateFile);
    key.Save(certificateFile, PTrue);
  }

  return sslContext.UseCertificate(certificateFile) &&
         sslContext.UsePrivateKey(certificateFile);
}

OpalTransportTCPS::OpalTransportTCPS(OpalEndPoint & ep,
                                     PIPSocket::Address binding,
                                     WORD port,
                                     PBoolean reuseAddr)
  : OpalTransportTCP(ep, binding, port, reuseAddr)
{
  sslContext = new PSSLContext;
}


OpalTransportTCPS::OpalTransportTCPS(OpalEndPoint & ep, PTCPSocket * socket)
  : OpalTransportTCP(ep, PIPSocket::GetDefaultIpAny(), 0)
{
  sslContext = new PSSLContext;
  PSSLChannel * sslChannel = new PSSLChannel(sslContext);
  if (!sslChannel->Open(socket))
    delete sslChannel;
  else
    Open(sslChannel);
}


OpalTransportTCPS::~OpalTransportTCPS()
{
  CloseWait();
  delete sslContext;
  PTRACE(4,"Opal\tDeleted transport " << *this);
}


PBoolean OpalTransportTCPS::IsCompatibleTransport(const OpalTransportAddress & address) const
{
  return (address.NumCompare(TcpPrefix)  == EqualTo) ||
         (address.NumCompare(IpPrefix)   == EqualTo) ||
         (address.NumCompare(TcpsPrefix) == EqualTo);
}


PBoolean OpalTransportTCPS::Connect()
{
  if (IsOpen())
    return PTrue;

  PTCPSocket * socket = new PTCPSocket(remotePort);

  PReadWaitAndSignal mutex(channelPointerMutex);

  socket->SetReadTimeout(10000);

  OpalManager & manager = endpoint.GetManager();
  localPort = manager.GetNextTCPPort();
  WORD firstPort = localPort;
  for (;;) {
    PTRACE(4, "OpalTCPS\tConnecting to "
           << remoteAddress << ':' << remotePort
           << " (local port=" << localPort << ')');
    if (socket->Connect(localPort, remoteAddress))
      break;

    int errnum = socket->GetErrorNumber();
    if (localPort == 0 || (errnum != EADDRINUSE && errnum != EADDRNOTAVAIL)) {
      PTRACE(1, "OpalTCPS\tCould not connect to "
                << remoteAddress << ':' << remotePort
                << " (local port=" << localPort << ") - "
                << socket->GetErrorText() << '(' << errnum << ')');
      return SetErrorValues(socket->GetErrorCode(), errnum);
    }

    localPort = manager.GetNextTCPPort();
    if (localPort == firstPort) {
      PTRACE(1, "OpalTCP\tCould not bind to any port in range " <<
                manager.GetTCPPortBase() << " to " << manager.GetTCPPortMax());
      return SetErrorValues(socket->GetErrorCode(), errnum);
    }
  }

  socket->SetReadTimeout(PMaxTimeInterval);

  PString certificateFile = endpoint.GetSSLCertificate();
  if (!SetSSLCertificate(*sslContext, certificateFile, PTrue)) {
    PTRACE(1, "OpalTCPS\tCould not load certificate \"" << certificateFile << '"');
    return PFalse;
  }

  PSSLChannel * sslChannel = new PSSLChannel(sslContext);
  if (!sslChannel->Connect(socket)) {
    delete sslChannel;
    return PFalse;
  }

  return Open(sslChannel);
}

PBoolean OpalTransportTCPS::OnOpen()
{
  PSSLChannel * sslChannel = dynamic_cast<PSSLChannel *>(GetReadChannel());
  if (sslChannel == NULL)
    return PFalse;

  PIPSocket * socket = dynamic_cast<PIPSocket *>(sslChannel->GetReadChannel());

  // Get name of the remote computer for information purposes
  if (!socket->GetPeerAddress(remoteAddress, remotePort)) {
    PTRACE(1, "OpalTCPS\tGetPeerAddress() failed: " << socket->GetErrorText());
    return PFalse;
  }

  // get local address of incoming socket to ensure that multi-homed machines
  // use a NIC address that is guaranteed to be addressable to destination
  if (!socket->GetLocalAddress(localAddress, localPort)) {
    PTRACE(1, "OpalTCPS\tGetLocalAddress() failed: " << socket->GetErrorText());
    return PFalse;
  }

#ifndef __BEOS__
  if (!socket->SetOption(TCP_NODELAY, 1, IPPROTO_TCP)) {
    PTRACE(1, "OpalTCPS\tSetOption(TCP_NODELAY) failed: " << socket->GetErrorText());
  }

  // make sure do not lose outgoing packets on close
  const linger ling = { 1, 3 };
  if (!socket->SetOption(SO_LINGER, &ling, sizeof(ling))) {
    PTRACE(1, "OpalTCP\tSetOption(SO_LINGER) failed: " << socket->GetErrorText());
    return PFalse;
  }
#endif

  PTRACE(3, "OpalTCPS\tStarted connection to "
         << remoteAddress << ':' << remotePort
         << " (if=" << localAddress << ':' << localPort << ')');

  return PTrue;
}


const char * OpalTransportTCPS::GetProtoPrefix() const
{
  return TcpsPrefix;
}

//////////////////////////////////////////////////////////////////////////

OpalListenerTCPS::OpalListenerTCPS(OpalEndPoint & ep,
                                 PIPSocket::Address binding,
                                 WORD port,
                                 PBoolean exclusive)
  : OpalListenerTCP(ep, binding, port, exclusive)
{
  Construct();
}


OpalListenerTCPS::OpalListenerTCPS(OpalEndPoint & ep,
                                    const OpalTransportAddress & binding,
                                    OpalTransportAddress::BindOptions option)
  : OpalListenerTCP(ep, binding, option)
{
  Construct();
}


OpalListenerTCPS::~OpalListenerTCPS()
{
  delete sslContext;
}


void OpalListenerTCPS::Construct()
{
  sslContext = new PSSLContext();

  PString certificateFile = endpoint.GetSSLCertificate();
  if (!SetSSLCertificate(*sslContext, certificateFile, PTrue)) {
    PTRACE(1, "OpalTCPS\tCould not load certificate \"" << certificateFile << '"');
  }
}


OpalTransport * OpalListenerTCPS::Accept(const PTimeInterval & timeout)
{
  if (!listener.IsOpen())
    return NULL;

  listener.SetReadTimeout(timeout); // Wait for remote connect

  PTRACE(4, "TCPS\tWaiting on socket accept on " << GetLocalAddress());
  PTCPSocket * socket = new PTCPSocket;
  if (!socket->Accept(listener)) {
    if (socket->GetErrorCode() != PChannel::Interrupted) {
      PTRACE(1, "Listen\tAccept error:" << socket->GetErrorText());
      listener.Close();
    }
    delete socket;
    return NULL;
  }

  OpalTransportTCPS * transport = new OpalTransportTCPS(endpoint);
  PSSLChannel * ssl = new PSSLChannel(sslContext);
  if (!ssl->Accept(socket)) {
    PTRACE(1, "TCPS\tAccept failed: " << ssl->GetErrorText());
    delete transport;
    delete ssl;
    delete socket;
    return NULL;
  }

  if (transport->Open(ssl))
    return transport;

  PTRACE(1, "TCPS\tFailed to open transport, connection not started.");
  delete transport;
  delete ssl;
  delete socket;
  return NULL;
}

const char * OpalListenerTCPS::GetProtoPrefix() const
{
  return TcpsPrefix;
}


#endif
