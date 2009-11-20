/*
 * handlers.cxx
 *
 * Session Initiation Protocol endpoint.
 *
 * Open Phone Abstraction Library (OPAL)
 *
 * Copyright (c) 2000 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Damien Sandras. 
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <opal/buildopts.h>

#if OPAL_SIP

#ifdef __GNUC__
#pragma implementation "handlers.h"
#endif

#include <sip/handlers.h>

#include <ptclib/pdns.h>
#include <ptclib/enum.h>
#include <sip/sipep.h>

#if OPAL_PTLIB_EXPAT
#include <ptclib/pxml.h>
#endif


#define new PNEW


#if PTRACING
ostream & operator<<(ostream & strm, SIPHandler::State state)
{
  static const char * const StateNames[] = {
    "Subscribed", "Subscribing", "Unavailable", "Refreshing", "Restoring", "Unsubscribing", "Unsubscribed"
  };
  if (state < PARRAYSIZE(StateNames))
    strm << StateNames[state];
  else
    strm << (unsigned)state;
  return strm;
}
#endif


////////////////////////////////////////////////////////////////////////////

SIPHandler::SIPHandler(SIPEndPoint & ep, const SIPParameters & params)
  : endpoint(ep)
  , authentication(NULL)
  , m_username(params.m_authID)
  , m_password(params.m_password)
  , m_realm(params.m_realm)
  , m_transport(NULL)
  , m_addressOfRecord(params.m_addressOfRecord)
  , m_remoteAddress(params.m_remoteAddress)
  , callID(SIPTransaction::GenerateCallID())
  , expire(params.m_expire)
  , originalExpire(params.m_expire)
  , offlineExpire(params.m_restoreTime)
  , authenticationAttempts(0)
  , state(Unavailable)
  , retryTimeoutMin(params.m_minRetryTime)
  , retryTimeoutMax(params.m_maxRetryTime)
  , m_proxy(params.m_proxyAddress)
{
  transactions.DisallowDeleteObjects();
  expireTimer.SetNotifier(PCREATE_NOTIFIER(OnExpireTimeout));

  PTRACE(4, "SIP\tConstructed handler for " << params.m_addressOfRecord);
}


SIPHandler::~SIPHandler() 
{
  expireTimer.Stop();

  if (m_transport) {
    m_transport->CloseWait();
    delete m_transport;
  }

  delete authentication;

  PTRACE(4, "SIP\tDeleted handler.");
}


bool SIPHandler::ShutDown()
{
  PSafeLockReadWrite mutex(*this);
  if (!mutex.IsLocked())
    return true;

  switch (state) {
    case Subscribed :
      SendRequest(Unsubscribing);
    case Unsubscribing :
      return transactions.IsEmpty();

    default :
      break;
  }

  for (PSafePtr<SIPTransaction> transaction(transactions, PSafeReference); transaction != NULL; ++transaction)
    transaction->Abort();

  return true;
}


void SIPHandler::SetState(SIPHandler::State newState) 
{
  PTRACE(4, "SIP\tChanging " << GetMethod() << " handler from " << state << " to " << newState
         << ", target=" << GetAddressOfRecord() << ", id=" << GetCallID());
  state = newState;
}


OpalTransport * SIPHandler::GetTransport()
{
  if (m_transport != NULL) {
    if (m_transport->IsOpen())
      return m_transport;

    m_transport->CloseWait();
    delete m_transport;
    m_transport = NULL;
  }

  if (m_proxy.IsEmpty()) {
    // Look for a "proxy" parameter to override default proxy
    const PStringToString & params = m_remoteAddress.GetParamVars();
    if (params.Contains(OPAL_PROXY_PARAM)) {
      m_proxy.Parse(params(OPAL_PROXY_PARAM));
      m_remoteAddress.SetParamVar(OPAL_PROXY_PARAM, PString::Empty());
    }
  }

  if (m_proxy.IsEmpty())
    m_proxy = endpoint.GetProxy();

  SIPURL url;
  if (!m_proxy.IsEmpty())
    url = m_proxy;
  else {
    url = m_remoteAddress;
    url.AdjustToDNS();
  }

  // Must specify a network interface or get infinite recursion
  m_transport = endpoint.CreateTransport(url, "*");
  return m_transport;
}


void SIPHandler::SetExpire(int e)
{
  expire = e;
  PTRACE(3, "SIP\tExpiry time for " << GetMethod() << " set to " << expire << " seconds.");

  // Only modify the originalExpire for future requests if IntervalTooBrief gives
  // a bigger expire time. expire itself will always reflect the proxy decision
  // (bigger or lower), but originalExpire determines what is used in future 
  // requests and is only modified if interval too brief
  if (originalExpire < e)
    originalExpire = e;

  // retry before the expire time.
  // if the expire time is more than 20 mins, retry 10mins before expiry
  // if the expire time is less than 20 mins, retry after half of the expiry time
  if (expire > 0 && state < Unsubscribing)
    expireTimer.SetInterval(0, (unsigned)(expire < 20*60 ? expire/2 : expire-10*60));
}


PBoolean SIPHandler::WriteSIPHandler(OpalTransport & transport, void * param)
{
  return param != NULL && ((SIPHandler *)param)->WriteSIPHandler(transport);
}


bool SIPHandler::WriteSIPHandler(OpalTransport & transport)
{
  SIPTransaction * transaction = CreateTransaction(transport);

  if (transaction != NULL) {
    for (PINDEX i = 0; i < m_mime.GetSize(); ++i) 
      transaction->GetMIME().SetAt(m_mime.GetKeyAt(i), PString(m_mime.GetDataAt(i)));
    if (state == Unsubscribing)
      transaction->GetMIME().SetExpires(0);
    if (authentication != NULL) {
      SIPAuthenticator auth(*transaction);
      authentication->Authorise(auth); // If already have info from last time, use it!
    }
    if (transaction->Start()) {
      transactions.Append(transaction);
      return true;
    }
  }

  PTRACE(2, "SIP\tDid not start transaction on " << transport);
  return false;
}


bool SIPHandler::ActivateState(SIPHandler::State newState, unsigned msecs)
{
  PTimeInterval startTick = PTimer::Tick();
  for (;;) {
    {
      PSafeLockReadWrite mutex(*this);
      if (!mutex.IsLocked())
        return false;

      if (SendRequest(newState))
        return true;
    }

    if ((PTimer::Tick() - startTick) > msecs)
      return false;

    PThread::Sleep(100);
  }
}


PBoolean SIPHandler::SendRequest(SIPHandler::State newState)
{
  expireTimer.Stop(false); // Stop automatic retry

  if (expire == 0)
    newState = Unsubscribing;

  switch (newState) {
    case Unsubscribing:
      switch (state) {
        case Subscribed :
        case Unavailable :
          break;  // Can try and do Unsubscribe

        case Subscribing :
        case Refreshing :
        case Restoring :
          PTRACE(2, "SIP\tCan't send " << newState << " request for " << GetMethod()
                 << " handler while in " << state << " state, target="
                 << GetAddressOfRecord() << ", id=" << GetCallID());
          return false; // Are in the process of doing something

        case Unsubscribed :
        case Unsubscribing:
          PTRACE(3, "SIP\tAlready doing " << state << " request for " << GetMethod()
                 << " handler, target=" << GetAddressOfRecord() << ", id=" << GetCallID());
          return true;  // Already done or doing it

        default :
          PAssertAlways(PInvalidParameter);
          return false;
      }
      break;

    case Subscribing :
    case Refreshing :
    case Restoring :
      switch (state) {
        case Subscribed :
        case Unavailable :
          break;  // Can do subscribe/refresh/restore

        case Refreshing :
        case Restoring :
          PTRACE(3, "SIP\tAlready doing " << state << " request for " << GetMethod()
                 << " handler, target=" << GetAddressOfRecord() << ", id=" << GetCallID());
          return true; // Already doing it

        case Subscribing :
        case Unsubscribing :
        case Unsubscribed :
          PTRACE(2, "SIP\tCan't send " << newState << " request for " << GetMethod()
                 << " handler while in " << state << " state, target="
                 << GetAddressOfRecord() << ", id=" << GetCallID());
          return false; // Can't restart as are on the way out

        default : // Are in the process of doing something
          PAssertAlways(PInvalidParameter);
          return false;
      }
      break;

    default :
      PAssertAlways(PInvalidParameter);
      return false;
  }

  SetState(newState);

  if (GetTransport() == NULL)
    OnFailed(SIP_PDU::Local_BadTransportAddress);
  else {
    // Restoring or first time, try every interface
    if (newState == Restoring || m_transport->GetInterface().IsEmpty()) {
      PWaitAndSignal mutex(m_transport->GetWriteMutex());
      if (m_transport->WriteConnect(WriteSIPHandler, this))
        return true;
    }
    else {
      // We contacted the server on an interface last time, assume it still works!
      if (WriteSIPHandler(*m_transport))
        return true;
    }

    OnFailed(SIP_PDU::Local_TransportError);
  }

  if (newState == Unsubscribing) {
    // Transport level error, probably never going to get the unsubscribe through
    SetState(Unsubscribed);
    return true;
  }

  PTRACE(4, "SIP\tRetrying " << GetMethod() << " in " << offlineExpire << " seconds.");
  expireTimer.SetInterval(0, offlineExpire); // Keep trying to get it back
  return true;
}


PBoolean SIPHandler::OnReceivedNOTIFY(SIP_PDU & /*response*/)
{
  return PFalse;
}


void SIPHandler::OnReceivedResponse(SIPTransaction & transaction, SIP_PDU & response)
{
  // Received a response, so collapse the forking on multiple interfaces.

  transactions.Remove(&transaction); // Take this transaction out of list

  // And kill all the rest
  PSafePtr<SIPTransaction> transToGo;
  while ((transToGo = transactions.GetAt(0)) != NULL) {
    transactions.Remove(transToGo);
    transToGo->Abort();
  }

  // Finally end connect mode on the transport
  m_transport->SetInterface(transaction.GetInterface());

  switch (response.GetStatusCode()) {
    case SIP_PDU::Failure_UnAuthorised :
    case SIP_PDU::Failure_ProxyAuthenticationRequired :
      OnReceivedAuthenticationRequired(transaction, response);
      break;

    case SIP_PDU::Failure_IntervalTooBrief :
      OnReceivedIntervalTooBrief(transaction, response);
      break;

    case SIP_PDU::Failure_TemporarilyUnavailable:
      OnReceivedTemporarilyUnavailable(transaction, response);
      break;

    case SIP_PDU::Failure_RequestTimeout :
      OnTransactionFailed(transaction);
      break;

    default :
      switch (response.GetStatusCode()/100) {
        case 1 :
          // Do nothing on 1xx
          break;

        case 2 :
          OnReceivedOK(transaction, response);
          break;

        default :
          OnFailed(response);
      }
  }
}


void SIPHandler::OnReceivedIntervalTooBrief(SIPTransaction & /*transaction*/, SIP_PDU & response)
{
  SetExpire(response.GetMIME().GetMinExpires());

  // Restart the transaction with new authentication handler
  State oldState = state;
  state = Unavailable;
  SendRequest(oldState);
}


void SIPHandler::OnReceivedTemporarilyUnavailable(SIPTransaction & /*transaction*/, SIP_PDU & response)
{
  OnFailed(SIP_PDU::Failure_TemporarilyUnavailable);

  unsigned retryAfter = response.GetMIME().GetInteger("Retry-After", offlineExpire);
  PTRACE(4, "SIP\tRetrying " << GetMethod() << " in " << retryAfter << " seconds.");
  expireTimer.SetInterval(0, retryAfter); // Have another go in a little bit
}


void SIPHandler::OnReceivedAuthenticationRequired(SIPTransaction & transaction, SIP_PDU & response)
{
  bool isProxy = response.GetStatusCode() == SIP_PDU::Failure_ProxyAuthenticationRequired;

#if PTRACING
  const char * proxyTrace = isProxy ? "Proxy " : "";
#endif
  PTRACE(3, "SIP\tReceived " << proxyTrace << "Authentication Required response");
  
  // Abort after some unsuccesful authentication attempts. This is required since
  // some implementations return "401 Unauthorized" with a different nonce at every
  // time.
  if (authenticationAttempts >= 10) {
    PTRACE(1, "SIP\tAborting after " << authenticationAttempts << " attempts to REGISTER/SUBSCRIBE");
    OnFailed(SIP_PDU::Failure_UnAuthorised);
    return;
  }
  ++authenticationAttempts;

  // authenticate 
  PString errorMsg;
  SIPAuthentication * newAuth = PHTTPClientAuthentication::ParseAuthenticationRequired(isProxy, response.GetMIME(), errorMsg);
  if (newAuth == NULL) {
    PTRACE(2, "SIP\t" << errorMsg);
    OnFailed(SIP_PDU::Failure_Forbidden);
    return;
  }

  // Try to find authentication parameters for the given realm,
  // if not, use the proxy authentication parameters (if any)
  PString authRealm = m_realm;
  PString username  = m_username;
  PString password  = m_password;
  if (endpoint.GetAuthentication(newAuth->GetAuthRealm(), authRealm, username, password)) {
    PTRACE (3, "SIP\tFound auth info for realm " << newAuth->GetAuthRealm());
  }
  else if (username.IsEmpty()) {
    const SIPURL & proxy = endpoint.GetProxy();
    if (!proxy.IsEmpty()) {
      PTRACE (3, "SIP\tNo auth info for realm " << newAuth->GetAuthRealm() << ", using proxy auth");
      username = proxy.GetUserName();
      password = proxy.GetPassword();
    }
    else {
      delete newAuth;
      PTRACE(1, "SIP\tAuthentication not possible yet.");
      OnFailed(SIP_PDU::Failure_TemporarilyUnavailable);
      if (expire > 0 && !transaction.IsCanceled()) {
        PTRACE(4, "SIP\tRetrying " << GetMethod() << " in " << offlineExpire << " seconds.");
        expireTimer.SetInterval(0, offlineExpire); // Keep trying to get it back
      }
      return;
    }
  }

  newAuth->SetUsername(username);
  newAuth->SetPassword(password);

  // check to see if this is a follow-on from the last authentication scheme used
  if (GetState() == Subscribing && authentication != NULL && *newAuth == *authentication) {
    delete newAuth;
    PTRACE(1, "SIP\tAuthentication already performed using current credentials, not trying again.");
    OnFailed(SIP_PDU::Failure_UnAuthorised);
    return;
  }

  // switch authentication schemes
  delete authentication;
  authentication = newAuth;
  m_realm    = newAuth->GetAuthRealm();
  m_username = username;
  m_password = password;

  // Restart the transaction with new authentication handler
  State oldState = state;
  state = Unavailable;
  SendRequest(oldState);
}


void SIPHandler::OnReceivedOK(SIPTransaction & /*transaction*/, SIP_PDU & response)
{
  response.GetMIME().GetProductInfo(m_productInfo);

  switch (GetState()) {
    case Unsubscribing :
      SetState(Unsubscribed);
      break;

    case Subscribing :
    case Refreshing :
    case Restoring :
      if (expire == 0)
        SetState(Unsubscribed);
      else
        SetState(Subscribed);
      break;

    default :
      PTRACE(2, "SIP\tUnexpected 200 OK in handler with state " << state);
  }

  // reset the number of unsuccesful authentication attempts
  authenticationAttempts = 0;
}


void SIPHandler::OnTransactionFailed(SIPTransaction & transaction)
{
  if (transactions.Remove(&transaction)) {
    OnFailed(transaction.GetStatusCode());

    if (expire > 0 && !transaction.IsCanceled()) {
      PTRACE(4, "SIP\tRetrying " << GetMethod() << " in " << offlineExpire << " seconds.");
      expireTimer.SetInterval(0, offlineExpire); // Keep trying to get it back
    }
  }
}


void SIPHandler::OnFailed(const SIP_PDU & response)
{
  OnFailed(response.GetStatusCode());
}


void SIPHandler::OnFailed(SIP_PDU::StatusCodes code)
{
  switch (code) {
    case SIP_PDU::Local_TransportError :
    case SIP_PDU::Local_Timeout :
    case SIP_PDU::Failure_RequestTimeout :
    case SIP_PDU::Local_BadTransportAddress :
    case SIP_PDU::Failure_TemporarilyUnavailable:
      if (GetState() != Unsubscribing) {
        SetState(Unavailable);
        break;
      }
      // Do next case to finalise Unsubscribe even though there was an error

    default :
      PTRACE(4, "SIP\tNot retrying " << GetMethod() << " due to error response " << code);
      expire = 0; // OK, stop trying
      expireTimer.Stop(false);
      SetState(Unsubscribed);
      ShutDown();
  }
}


void SIPHandler::OnExpireTimeout(PTimer &, INT)
{
  PSafeLockReadWrite lock(*this);
  if (!lock.IsLocked())
    return;

  switch (GetState()) {
    case Subscribed :
      PTRACE(2, "SIP\tStarting " << GetMethod() << " for binding refresh");
      if (SendRequest(Refreshing))
        return;
      break;

    case Unavailable :
      PTRACE(2, "SIP\tStarting " << GetMethod() << " for offline retry");
      if (SendRequest(Restoring))
        return;
      break;

    default :
      return;
  }

  SetState(Unavailable);
}


///////////////////////////////////////////////////////////////////////////////

SIPRegisterHandler::SIPRegisterHandler(SIPEndPoint & endpoint, const SIPRegister::Params & params)
  : SIPHandler(endpoint, params)
  , m_parameters(params)
  , m_sequenceNumber(0)
{
  // Foer some bizarre reason, even though REGISTER does not create a dialog,
  // some registrars insist that you have a from tag ...
  SIPURL local = params.m_localAddress.IsEmpty() ? params.m_addressOfRecord : params.m_localAddress;
  local.SetTag();
  m_parameters.m_localAddress = local.AsQuotedString();
}


SIPRegisterHandler::~SIPRegisterHandler()
{
  PTRACE(4, "SIP\tDeleting SIPRegisterHandler " << GetAddressOfRecord());
}


SIPTransaction * SIPRegisterHandler::CreateTransaction(OpalTransport & trans)
{
  SIPRegister::Params params = m_parameters;

  if (expire == 0 || GetState() == Unsubscribing) {
    params.m_expire = 0;
    if (params.m_contactAddress.IsEmpty())
      params.m_contactAddress = "*";
  }
  else {
    params.m_expire = expire;

    if (params.m_contactAddress.IsEmpty()) {
      PString userName = SIPURL(params.m_addressOfRecord).GetUserName();
      OpalTransportAddressArray interfaces = endpoint.GetInterfaceAddresses(true, &trans);
      if (params.m_compatibility == SIPRegister::e_CannotRegisterMultipleContacts) {
        // If translated by STUN then only the external address of the interface is used.
        SIPURL contact(userName, interfaces[0]);
        contact.Sanitise(SIPURL::ContactURI);
        params.m_contactAddress += contact.AsQuotedString();
      }
      else {
        OpalTransportAddress localAddress = trans.GetLocalAddress();
        unsigned qvalue = 1000;
        for (PINDEX i = 0; i < interfaces.GetSize(); ++i) {
          /* If fully compliant, put into the contact field all the bound
             interfaces. If special then we only put into the contact
             listeners that are on the same interface. If translated by STUN
             then only the external address of the interface is used. */
          if (params.m_compatibility != SIPRegister::e_CannotRegisterPrivateContacts || localAddress.IsEquivalent(interfaces[i], true)) {
            if (!params.m_contactAddress.IsEmpty())
              params.m_contactAddress += ", ";
            SIPURL contact(userName, interfaces[i]);
            contact.Sanitise(SIPURL::ContactURI);
            params.m_contactAddress += contact.AsQuotedString();
            params.m_contactAddress.sprintf(qvalue < 1000 ? ";q=0.%03u" : ";q=1", qvalue);
            qvalue -= 1000/interfaces.GetSize();
          }
        }
      }
    }
    else {
      // Sanitise the contact address URI provided
      SIPURL contact(params.m_contactAddress);
      contact.Sanitise(SIPURL::ContactURI);
      params.m_contactAddress = contact.AsQuotedString();
    }
  }

  return new SIPRegister(endpoint, trans, GetCallID(), m_sequenceNumber, params);
}


void SIPRegisterHandler::OnReceivedOK(SIPTransaction & transaction, SIP_PDU & response)
{
  State oldState = GetState();

  SIPHandler::OnReceivedOK(transaction, response);

  std::list<SIPURL> requestContacts, replyContacts;
  transaction.GetMIME().GetContacts(requestContacts);
  response.GetMIME().GetContacts(replyContacts);

  m_parameters.m_contactAddress.MakeEmpty();

  for (std::list<SIPURL>::iterator request = requestContacts.begin(); request != requestContacts.end(); ++request) {
    for (std::list<SIPURL>::iterator reply = replyContacts.begin(); reply != replyContacts.end(); ++reply) {
      if (*request == *reply) {
        PString expires = SIPMIMEInfo::ExtractFieldParameter(reply->GetFieldParameters(), "expires");
        if (expires.IsEmpty())
          SetExpire(response.GetMIME().GetExpires(endpoint.GetRegistrarTimeToLive().GetSeconds()));
        else
          SetExpire(expires.AsUnsigned());

        if (!m_parameters.m_contactAddress.IsEmpty())
          m_parameters.m_contactAddress += ", ";
        m_parameters.m_contactAddress += request->AsString();
      }
    }
  }

  response.GetMIME().GetProductInfo(m_productInfo);

  SendStatus(SIP_PDU::Successful_OK, oldState);
}


void SIPRegisterHandler::OnFailed(SIP_PDU::StatusCodes r)
{
  SendStatus(r, GetState());
  SIPHandler::OnFailed(r);
}


PBoolean SIPRegisterHandler::SendRequest(SIPHandler::State s)
{
  SendStatus(SIP_PDU::Information_Trying, s);
  m_sequenceNumber = endpoint.GetNextCSeq();
  return SIPHandler::SendRequest(s);
}


void SIPRegisterHandler::SendStatus(SIP_PDU::StatusCodes code, State state)
{
  SIPEndPoint::RegistrationStatus status;
  status.m_handler = this;
  status.m_addressofRecord = GetAddressOfRecord().AsString();
  status.m_productInfo = m_productInfo;
  status.m_reason = code;
  status.m_userData = m_parameters.m_userData;

  switch (state) {
    case Subscribing :
      status.m_wasRegistering = true;
      status.m_reRegistering = false;
      break;

    case Subscribed :
    case Refreshing :
      status.m_wasRegistering = true;
      status.m_reRegistering = true;
      break;

    case Unsubscribed :
    case Unavailable :
    case Restoring :
      status.m_wasRegistering = true;
      status.m_reRegistering = code/100 != 2;
      break;

    case Unsubscribing :
      status.m_wasRegistering = false;
      status.m_reRegistering = false;
      break;
  }

  endpoint.OnRegistrationStatus(status);
}


void SIPRegisterHandler::UpdateParameters(const SIPRegister::Params & params)
{
  if (!params.m_authID.IsEmpty())
    m_username = m_parameters.m_authID = params.m_authID;   // Adjust the authUser if required 
  if (!params.m_realm.IsEmpty())
    m_realm = m_parameters.m_realm = params.m_realm;   // Adjust the realm if required 
  if (!params.m_password.IsEmpty())
    m_password = m_parameters.m_password = params.m_password; // Adjust the password if required 

  if (params.m_expire > 0)
    SetExpire(m_parameters.m_expire = params.m_expire);

  m_parameters.m_contactAddress = params.m_contactAddress;
}


/////////////////////////////////////////////////////////////////////////

SIPSubscribeHandler::SIPSubscribeHandler(SIPEndPoint & endpoint, const SIPSubscribe::Params & params)
  : SIPHandler(endpoint, params)
  , m_parameters(params)
  , m_unconfirmed(true)
  , m_packageHandler(SIPEventPackageFactory::CreateInstance(params.m_eventPackage))
{
  callID = m_dialog.GetCallID();
}


SIPSubscribeHandler::~SIPSubscribeHandler()
{
  PTRACE(4, "SIP\tDeleting SIPSubscribeHandler " << GetAddressOfRecord());
  delete m_packageHandler;
}


SIPTransaction * SIPSubscribeHandler::CreateTransaction(OpalTransport &trans)
{ 
  // Do all the following here as must be after we have called GetTransport()
  // which sets various fields correctly for transmission
  if (!m_dialog.IsEstablished()) {
    m_dialog.SetRequestURI(GetAddressOfRecord());
    if (m_parameters.m_eventPackage.IsWatcher())
      m_parameters.m_localAddress = GetAddressOfRecord().AsString();

    m_dialog.SetRemoteURI(m_parameters.m_addressOfRecord);

    if (m_parameters.m_localAddress.IsEmpty())
      m_dialog.SetLocalURI(endpoint.GetRegisteredPartyName(m_parameters.m_addressOfRecord, *m_transport));
    else
      m_dialog.SetLocalURI(m_parameters.m_localAddress);

    m_dialog.SetProxy(m_proxy, true);
  }

  m_parameters.m_expire = state != Unsubscribing ? expire : 0;
  return new SIPSubscribe(endpoint, trans, m_dialog, m_parameters);
}


void SIPSubscribeHandler::OnFailed(const SIP_PDU & response)
{
  SIP_PDU::StatusCodes r = response.GetStatusCode();

  SendStatus(r, GetState());

  int newExpires = 0;
  PString dummy;
  
  switch (r) {
    case SIP_PDU::Failure_TransactionDoesNotExist:
      // Resubscribe as previous subscription totally lost, but dialog processing
      // may have altered the target so restore the original target address
      m_parameters.m_addressOfRecord = GetAddressOfRecord().AsString();
      endpoint.Subscribe(m_parameters, dummy);
      break;

    case SIP_PDU::Failure_IntervalTooBrief:
      // Resubscribe with altered expiry
      newExpires = response.GetMIME().GetExpires();
      if (newExpires > 0) {
        m_parameters.m_expire = newExpires;
        PString dummy;
        endpoint.Subscribe(m_parameters, dummy);
      }
      break;

    default:
      // fall through
      SIPHandler::OnFailed(r);
  }
}


PBoolean SIPSubscribeHandler::SendRequest(SIPHandler::State s)
{
  SendStatus(SIP_PDU::Information_Trying, s);
  return SIPHandler::SendRequest(s);
}


void SIPSubscribeHandler::SendStatus(SIP_PDU::StatusCodes code, State state)
{
  SIPEndPoint::SubscriptionStatus status;
  status.m_handler = this;
  status.m_addressofRecord = GetAddressOfRecord().AsString();
  status.m_productInfo = m_productInfo;
  status.m_reason = code;
  status.m_userData = m_parameters.m_userData;

  switch (state) {
    case Subscribing :
      status.m_wasSubscribing = true;
      status.m_reSubscribing = false;
      break;

    case Subscribed :
      if (m_unconfirmed) {
        status.m_wasSubscribing = true;
        status.m_reSubscribing = false;
        endpoint.OnSubscriptionStatus(status);
      }
      // Do next state

    case Refreshing :
      status.m_wasSubscribing = true;
      status.m_reSubscribing = true;
      break;

    case Unsubscribed :
    case Unavailable :
    case Restoring :
      status.m_wasSubscribing = true;
      status.m_reSubscribing = code/100 != 2;
      break;

    case Unsubscribing :
      status.m_wasSubscribing = false;
      status.m_reSubscribing = false;
      break;
  }

  if (!m_parameters.m_onSubcribeStatus.IsNULL()) 
    m_parameters.m_onSubcribeStatus(*this, status);

  endpoint.OnSubscriptionStatus(status);
}


void SIPSubscribeHandler::UpdateParameters(const SIPSubscribe::Params & params)
{
  if (!params.m_authID.IsEmpty())
    m_username = params.m_authID;   // Adjust the authUser if required 
  if (!params.m_realm.IsEmpty())
    m_realm = params.m_realm;   // Adjust the realm if required 
  if (!params.m_password.IsEmpty())
    m_password = params.m_password; // Adjust the password if required 

  m_parameters.m_contactAddress = params.m_contactAddress;

  if (params.m_expire > 0)
    SetExpire(params.m_expire);
}


void SIPSubscribeHandler::OnReceivedOK(SIPTransaction & transaction, SIP_PDU & response)
{
  State oldState = GetState();

  /* An "expire" parameter in the Contact header has no semantics
   * for SUBSCRIBE. RFC3265, 3.1.1.
   * An answer can only shorten the expires time.
   */
  SetExpire(response.GetMIME().GetExpires(originalExpire));

  SIPHandler::OnReceivedOK(transaction, response);

  m_dialog.Update(response);

  if (oldState == Unsubscribing)
    SendStatus(SIP_PDU::Successful_OK, oldState);
}


PBoolean SIPSubscribeHandler::OnReceivedNOTIFY(SIP_PDU & request)
{
  if (PAssertNULL(m_transport) == NULL)
    return false;

  if (m_unconfirmed) {
    SendStatus(SIP_PDU::Successful_OK, GetState());
    m_unconfirmed = false;
  }

  // If we received a NOTIFY before
  if (m_dialog.IsDuplicateCSeq(request.GetMIME().GetCSeqIndex())) {
    PTRACE(3, "SIP\tReceived duplicate NOTIFY");
    return request.SendResponse(*m_transport, SIP_PDU::Successful_OK, &endpoint);
  }

  SIP_PDU response(request, SIP_PDU::Failure_BadRequest);

  PString requestEvent = request.GetMIME().GetEvent();
  if (m_parameters.m_eventPackage != requestEvent) {
    PTRACE(2, "SIP\tNOTIFY received for incorrect event \"" << requestEvent << "\", requires \"" << m_parameters.m_eventPackage << '"');
    response.SetStatusCode(SIP_PDU::Failure_BadEvent);
    response.GetMIME().SetAt("Allow-Events", m_parameters.m_eventPackage);
    return request.SendResponse(*m_transport, response, &endpoint);
  }

  // check the ContentType
  PCaselessString requestContentType = request.GetMIME().GetContentType();
  if (!m_parameters.m_contentType.IsEmpty() && requestContentType != m_parameters.m_contentType) {
    PTRACE(2, "SIPPres\tNOTIFY contains unsupported Content-Type \""
           << requestContentType << "\", expecting \"" << m_parameters.m_contentType << '"');
    response.SetStatusCode(SIP_PDU::Failure_UnsupportedMediaType);
    response.GetMIME().SetAt("Accept", m_parameters.m_contentType);
    response.SetInfo("Unsupported Content-Type");
    return request.SendResponse(*m_transport, response, &endpoint);
  }

  PStringArray subscriptionStateInfo = request.GetMIME().GetSubscriptionState().Tokenise(';', false);
  if (subscriptionStateInfo.IsEmpty()) {
    PTRACE(2, "SIP\tNOTIFY received without Subscription-State");
    return request.SendResponse(*m_transport, SIP_PDU::Failure_BadRequest, &endpoint, NULL, "No Subscription-State field");
  }

  // Check the susbscription state
  PCaselessString subscriptionState = subscriptionStateInfo[0];
  if (subscriptionState == "terminated") {
    PTRACE(3, "SIP\tSubscription is terminated, state=" << GetState());
    request.SendResponse(*m_transport, SIP_PDU::Successful_OK, &endpoint);
    ShutDown();
    return true;
  }

  if (subscriptionState == "active" || subscriptionState == "pending") {
    PTRACE(3, "SIP\tSubscription is " << state);
    PString expire = SIPMIMEInfo::ExtractFieldParameter(state, "expire");
    if (!expire.IsEmpty())
      SetExpire(expire.AsUnsigned());
  }

  if (m_packageHandler != NULL)
    return request.SendResponse(*m_transport,
                                m_packageHandler->OnReceivedNOTIFY(*this, request)
                                        ? SIP_PDU::Successful_OK
                                        : SIP_PDU::Failure_BadRequest,
                                &endpoint);

  if (m_parameters.m_onNotify.IsNULL()) {
    PTRACE(2, "SIP\tNo handler for NOTIFY received for event \"" << requestEvent << '"');
    return request.SendResponse(*m_transport, SIP_PDU::Failure_BadEvent, &endpoint);
  }

  SIPSubscribe::NotifyCallbackInfo status(endpoint, *m_transport, request, response);

  m_parameters.m_onNotify(*this, status);

  if (status.m_sendResponse)
    request.SendResponse(*m_transport, response, &endpoint);

  return true;
}


class SIPMwiEventPackageHandler : public SIPEventPackageHandler
{
  virtual PCaselessString GetContentType() const
  {
    return "application/simple-message-summary";
  }

  virtual bool OnReceivedNOTIFY(SIPHandler & handler, SIP_PDU & request)
  {
    PString body = request.GetEntityBody();
    if (body.IsEmpty ())
      return true;

    // Extract the string describing the number of new messages
    static struct {
      const char * name;
      OpalManager::MessageWaitingType type;
    } const validMessageClasses[] = {
      { "voice-message",      OpalManager::VoiceMessageWaiting      },
      { "fax-message",        OpalManager::FaxMessageWaiting        },
      { "pager-message",      OpalManager::PagerMessageWaiting      },
      { "multimedia-message", OpalManager::MultimediaMessageWaiting },
      { "text-message",       OpalManager::TextMessageWaiting       },
      { "none",               OpalManager::NoMessageWaiting         }
    };
    PString msgs;
    PStringArray bodylines = body.Lines ();
    for (PINDEX z = 0 ; z < PARRAYSIZE(validMessageClasses); z++) {

      for (int i = 0 ; i < bodylines.GetSize () ; i++) {

        PCaselessString line (bodylines [i]);
        PINDEX j = line.FindLast(validMessageClasses[z].name);
        if (j != P_MAX_INDEX) {
          line.Replace(validMessageClasses[z].name, "");
          line.Replace (":", "");
          msgs = line.Trim ();
          handler.GetEndPoint().OnMWIReceived(handler.GetAddressOfRecord().AsString(), validMessageClasses[z].type, msgs);
          return true;
        }
      }
    }

    // Received MWI, unknown messages number
    handler.GetEndPoint().OnMWIReceived(handler.GetAddressOfRecord().AsString(), OpalManager::NumMessageWaitingTypes, "1/0");

    return true;
  }
};

static SIPEventPackageFactory::Worker<SIPMwiEventPackageHandler> mwiEventPackageHandler(SIPSubscribe::MessageSummary);


///////////////////////////////////////////////////////////////////////////////

#if P_EXPAT

static void ParseParticipant(PXMLElement * participantElement, SIPDialogNotification::Participant & participant)
{
  if (participantElement == NULL)
    return;

  PXMLElement * identityElement = participantElement->GetElement("identity");
  if (identityElement != NULL) {
    participant.m_identity = identityElement->GetData();
    participant.m_display = identityElement->GetAttribute("display");
  }

  PXMLElement * targetElement = participantElement->GetElement("target");
  if (targetElement == NULL)
    return;

  participant.m_URI = targetElement->GetAttribute("uri");

  PXMLElement * paramElement;
  PINDEX i = 0;
  while ((paramElement = targetElement->GetElement("param", i++)) != NULL) {
    PCaselessString name = paramElement->GetAttribute("pname");
    PCaselessString value = paramElement->GetAttribute("pvalue");
    if (name == "appearance" || // draft-anil-sipping-bla-04 version
        name == "x-line-id")    // draft-anil-sipping-bla-03 version
      participant.m_appearance = value.AsUnsigned();
    else if (name == "sip.byeless" || name == "+sip.byeless")
      participant.m_byeless = value == "true";
    else if (name == "sip.rendering" || name == "+sip.rendering") {
      if (value == "yes")
        participant.m_rendering = SIPDialogNotification::RenderingMedia;
      else if (value == "no")
        participant.m_rendering = SIPDialogNotification::NotRenderingMedia;
      else
        participant.m_rendering = SIPDialogNotification::RenderingUnknown;
    }
  }
}


class SIPDialogEventPackageHandler : public SIPEventPackageHandler
{
public:
  SIPDialogEventPackageHandler()
    : m_dialogNotifyVersion(1)
  {
  }

  virtual PCaselessString GetContentType() const
  {
    return "application/dialog-info+xml";
  }

  virtual bool OnReceivedNOTIFY(SIPHandler & handler, SIP_PDU & request)
  {
    // Check for empty body, if so then is OK, just a ping ...
    if (request.GetEntityBody().IsEmpty())
      return true;

    PXML xml;
    if (!xml.Load(request.GetEntityBody()))
      return false;

    PXMLElement * rootElement = xml.GetRootElement();
    if (rootElement == NULL || rootElement->GetName() != "dialog-info")
      return false;

    SIPDialogNotification info(rootElement->GetAttribute("entity"));
    if (info.m_entity.IsEmpty())
      return false;

    PINDEX index = 0;
    PXMLElement * dialogElement;
    while ((dialogElement = rootElement->GetElement("dialog", index)) != NULL) {
      info.m_callId = dialogElement->GetAttribute("call-id");
      info.m_local.m_dialogTag = dialogElement->GetAttribute("local-tag");
      info.m_remote.m_dialogTag = dialogElement->GetAttribute("remote-tag");

      PXMLElement * stateElement = dialogElement->GetElement("state");
      if (stateElement == NULL)
        info.m_state = SIPDialogNotification::Terminated;
      else {
        PCaselessString str = stateElement->GetData();
        for (info.m_state = SIPDialogNotification::LastState; info.m_state > SIPDialogNotification::FirstState; --info.m_state) {
          if (str == info.GetStateName())
            break;
        }

        str = stateElement->GetAttribute("event");
        for (info.m_eventType = SIPDialogNotification::LastEvent; info.m_eventType >= SIPDialogNotification::FirstEvent; --info.m_eventType) {
          if (str == info.GetEventName())
            break;
        }

        info.m_eventCode = stateElement->GetAttribute("code").AsUnsigned();
      }

      ParseParticipant(dialogElement->GetElement("local"), info.m_local);
      ParseParticipant(dialogElement->GetElement("remote"), info.m_remote);
      handler.GetEndPoint().OnDialogInfoReceived(info);
      index++;
    }

    if (index == 0)
      handler.GetEndPoint().OnDialogInfoReceived(info);
    return true;
  }

  virtual PString OnSendNOTIFY(SIPHandler & handler, const PObject * data)
  {
    PStringStream body;
    body << "<?xml version=\"1.0\"?>\r\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\""
         << m_dialogNotifyVersion++ << "\" state=\"partial\" entity=\""
         << handler.GetAddressOfRecord() << "\">\r\n";

    std::map<PString, SIPDialogNotification>::iterator iter;

    const SIPDialogNotification * info = dynamic_cast<const SIPDialogNotification *>(data);
    if (info != NULL) {
      if (info->m_state != SIPDialogNotification::Terminated)
        m_activeDialogs[info->m_callId] = *info;
      else {
        iter = m_activeDialogs.find(info->m_callId);
        if (iter != m_activeDialogs.end())
          m_activeDialogs.erase(iter);

        body << *info;
      }
    }

    for (iter = m_activeDialogs.begin(); iter != m_activeDialogs.end(); ++iter)
      body << iter->second;

    body << "</dialog-info>\r\n";
    return body;
  }

  unsigned m_dialogNotifyVersion;
  std::map<PString, SIPDialogNotification> m_activeDialogs;
};

static SIPEventPackageFactory::Worker<SIPDialogEventPackageHandler> dialogEventPackageHandler(SIPSubscribe::Dialog);

#endif // P_EXPAT


///////////////////////////////////////////////////////////////////////////////

SIPDialogNotification::SIPDialogNotification(const PString & entity)
  : m_entity(entity)
  , m_initiator(false)
  , m_state(Terminated)
  , m_eventType(NoEvent)
  , m_eventCode(0)
{
}


PString SIPDialogNotification::GetStateName(States state)
{
  static const char * const Names[] = {
    "terminated",
    "trying",
    "proceeding",
    "early",
    "confirmed"
  };
  if (state < PARRAYSIZE(Names) && Names[state] != NULL)
    return Names[state];

  return psprintf("<%u>", state);
}


PString SIPDialogNotification::GetEventName(Events state)
{
  static const char * const Names[] = {
    "cancelled",
    "rejected",
    "replaced",
    "local-bye",
    "remote-bye",
    "error",
    "timeout"
  };
  if (state < PARRAYSIZE(Names) && Names[state] != NULL)
    return Names[state];

  return psprintf("<%u>", state);
}


static void OutputParticipant(ostream & strm, const char * name, const SIPDialogNotification::Participant & participant)
{
  if (participant.m_URI.IsEmpty())
    return;

  strm << "    <" << name << ">\r\n";

  if (!participant.m_identity.IsEmpty()) {
    strm << "      <identity";
    if (!participant.m_display.IsEmpty())
      strm << " display=\"" << participant.m_display << '"';
    strm << '>' << participant.m_identity << "</identity>\r\n";
  }

  strm << "      <target uri=\"" << participant.m_URI << "\">\r\n";

  if (participant.m_appearance >= 0)
    strm << "        <param pname=\"appearance\" pval=\"" << participant.m_appearance << "\"/>\r\n"
            "        <param pname=\"x-line-id\" pval=\"" << participant.m_appearance << "\"/>\r\n";

  if (participant.m_byeless)
    strm << "        <param pname=\"sip.byeless\" pval=\"true\"/>\r\n";

  if (participant.m_rendering >= 0)
    strm << "        <param pname=\"sip.rendering\" pval=\"" << (participant.m_rendering > 0 ? "yes" : "no") << "\"/>\r\n";

  strm << "      </target>\r\n"
       << "    </" << name << ">\r\n";
}


void SIPDialogNotification::PrintOn(ostream & strm) const
{
  if (m_dialogId.IsEmpty())
    return;

  // Start dialog XML tag
  strm << "  <dialog id=\"" << m_dialogId << '"';
  if (!m_callId)
    strm << " call-id=\"" << m_callId << '"';
  if (!m_local.m_dialogTag)
    strm << " local-tag=\"" << m_local.m_dialogTag << '"';
  if (!m_remote.m_dialogTag)
    strm << " remote-tag=\"" << m_remote.m_dialogTag << '"';
  strm << " direction=\"" << (m_initiator ? "initiator" : "receiver") << "\">\r\n";

  // State XML tag & value
  strm << "    <state";
  if (m_eventType > SIPDialogNotification::NoEvent) {
    strm << " event=\"" << GetEventName() << '"';
    if (m_eventCode > 0)
      strm << " code=\"" << m_eventCode << '"';
  }
  strm << '>' << GetStateName() << "</state>\r\n";

  // Participant XML tags (local/remopte)
  OutputParticipant(strm, "local", m_local);
  OutputParticipant(strm, "remote", m_remote);

  // Close out dialog tag
  strm << "  </dialog>\r\n";
}

/////////////////////////////////////////////////////////////////////////

SIPNotifyHandler::SIPNotifyHandler(SIPEndPoint & endpoint,
                                   const PString & targetAddress,
                                   const SIPEventPackage & eventPackage,
                                   const SIPDialogContext & dialog)
  : SIPHandler(endpoint, SIPParameters(targetAddress, dialog.GetRemoteURI().AsString()))
  , m_eventPackage(eventPackage)
  , m_dialog(dialog)
  , m_reason(Deactivated)
  , m_packageHandler(SIPEventPackageFactory::CreateInstance(eventPackage))
{
  callID = m_dialog.GetCallID();
}


SIPNotifyHandler::~SIPNotifyHandler()
{
  delete m_packageHandler;
}


SIPTransaction * SIPNotifyHandler::CreateTransaction(OpalTransport & trans)
{
  PString state;
  if (expire > 0 && GetState() != Unsubscribing)
    state.sprintf("active;expires=%u", expire);
  else {
    state = "terminated;reason=";
    static const char * const ReasonNames[] = {
      "deactivated",
      "probation",
      "rejected",
      "timeout",
      "giveup",
      "noresource"
    };
    state += ReasonNames[m_reason];
  }

  return new SIPNotify(endpoint, trans, m_dialog, m_eventPackage, state, body);
}


PBoolean SIPNotifyHandler::SendRequest(SIPHandler::State state)
{
  // If times out, i.e. Refreshing, then this is actually a time out unsubscribe.
  if (state == Refreshing)
    m_reason = Timeout;

  return SIPHandler::SendRequest(state == Refreshing ? Unsubscribing : state);
}


bool SIPNotifyHandler::SendNotify(const PObject * body)
{
  if (!LockReadWrite())
    return false;

  if (m_packageHandler != NULL)
    SetBody(m_packageHandler->OnSendNOTIFY(*this, body));
  else if (body == NULL)
    SetBody(PString::Empty());
  else {
    PStringStream str;
    str << *body;
    SetBody(str);
  }

  UnlockReadWrite();

  return ActivateState(Subscribing, endpoint.GetNonInviteTimeout().GetInterval());
}


/////////////////////////////////////////////////////////////////////////

SIPPublishHandler::SIPPublishHandler(SIPEndPoint & endpoint,
                                     const SIPSubscribe::Params & params,
                                     const PString & b)
  : SIPHandler(endpoint, params)
  , m_parameters(params)
{
  body = b;
}


SIPPublishHandler::~SIPPublishHandler()
{
  PTRACE(4, "SIP\tDeleting SIPPublishHandler " << GetAddressOfRecord());
}


SIPTransaction * SIPPublishHandler::CreateTransaction(OpalTransport & transport)
{
  if (state == Unsubscribing)
    return NULL;

  m_parameters.m_expire = expire;
  return new SIPPublish(endpoint,
                        transport,
                        GetCallID(),
                        m_sipETag,
                        m_parameters,
                        (GetState() == Refreshing) ? PString::Empty() : body);
}


void SIPPublishHandler::OnReceivedOK(SIPTransaction & transaction, SIP_PDU & response)
{
  PString newETag = response.GetMIME().GetSIPETag();

  if (!newETag.IsEmpty())
    m_sipETag = newETag;

  SetExpire(response.GetMIME().GetExpires(originalExpire));

  SIPHandler::OnReceivedOK(transaction, response);
}


///////////////////////////////////////////////////////////////////////

static PAtomicInteger DefaultTupleIdentifier;

SIPPresenceInfo::SIPPresenceInfo(State state)
  : OpalPresenceInfo(state)
  , m_tupleId(PString::Printf, "T%08X", ++DefaultTupleIdentifier)
{
}


void SIPPresenceInfo::PrintOn(ostream & strm) const
{
  if (m_entity.IsEmpty())
    return;

  if (m_activities.GetSize() > 0)
    strm << setfill(',') << m_activities << setfill(' ');
  else {
    switch (m_state) {
      case Unchanged :
        strm << "Unchanged";
        break;

      case NoPresence :
        strm << "Closed";
        break;

      default:
        if (m_note.IsEmpty())
          strm << "Open";
        else
          strm << m_note;
    }
  }
}


PString SIPPresenceInfo::AsXML() const
{
  if (m_entity.IsEmpty() || m_tupleId.IsEmpty()) {
    PTRACE(1, "SIP\tCannot encode Presence XML as no address or no id.");
    return PString::Empty();
  }

  PCaselessString entity = m_entity;
  if (entity.NumCompare("sip:") != PObject::EqualTo && entity.NumCompare("pres:") != PObject::EqualTo)
    entity.Splice("pres:", 0);

  PStringStream xml;

  xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
    "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"" << entity << "\">\r\n"
         "  <tuple id=\"" << m_tupleId << "\">\r\n"
         "    <status>\r\n";
  if (m_state != Unchanged)
    xml << "      <basic>" << (m_state != NoPresence ? "open" : "closed") << "</basic>\r\n";
  xml << "    </status>\r\n"
         "    <contact priority=\"1\">" << (m_contact.IsEmpty() ? entity : m_contact) << "</contact>\r\n";

  if (!m_note.IsEmpty()) {
    PString note = m_note;
    note.Replace("<", "&lt;");
    note.Replace(">", "&gt;");
    note.Replace("&", "&amp;");
    xml << "    <note xml:lang=\"en\">" << note << "</note>\r\n";
  }

  xml << "  </tuple>\r\n"
         "</presence>\r\n";

  return xml;
}


/////////////////////////////////////////////////////////////////////////

SIPMessageHandler::SIPMessageHandler (SIPEndPoint & endpoint, const PString & to, const PString & b, const PString & c, const PString & id)
  : SIPHandler(endpoint, SIPParameters(to, c))
{
  body   = b;
  callID = id;
  SetState(Subscribed);
}


SIPMessageHandler::~SIPMessageHandler ()
{
  PTRACE(4, "SIP\tDeleting SIPMessageHandler " << GetAddressOfRecord());
}


SIPTransaction * SIPMessageHandler::CreateTransaction(OpalTransport & transport)
{ 
  if (state == Unsubscribing)
    return NULL;

  SetExpire(originalExpire);
  return new SIPMessage(endpoint, transport, m_proxy, GetAddressOfRecord(), GetCallID(), body, m_localAddress);
}


void SIPMessageHandler::OnFailed(SIP_PDU::StatusCodes reason)
{ 
  endpoint.OnMessageFailed(GetAddressOfRecord(), reason);
  SIPHandler::OnFailed(reason);
}


void SIPMessageHandler::OnExpireTimeout(PTimer &, INT)
{
  SetState(Unavailable);
}


void SIPMessageHandler::SetBody(const PString & b)
{
  SIPHandler::SetBody (b);
}

/////////////////////////////////////////////////////////////////////////

SIPPingHandler::SIPPingHandler(SIPEndPoint & endpoint, const PURL & to)
: SIPHandler(endpoint, SIPParameters(to.AsString()))
{
}


SIPTransaction * SIPPingHandler::CreateTransaction(OpalTransport &t)
{
  if (state == Unsubscribing)
    return NULL;

  return new SIPPing(endpoint, t, GetAddressOfRecord(), body);
}


//////////////////////////////////////////////////////////////////

/* All of the bwlow search loops run through the list with only
   PSafeReference rather than PSafeReadOnly, even though they are
   reading fields from the handler instances. We can get away with
   this becuase the information being tested, e.g. AOR, is constant
   for the life of the handler instance, once constructed.

   We need to use PSafeReference as there are some cases where
   deadlocks can occur when locked handlers look for information
   from other handlers.
 */
unsigned SIPHandlersList::GetCount(SIP_PDU::Methods meth, const PString & eventPackage) const
{
  unsigned count = 0;
  for (PSafePtr<SIPHandler> handler(m_handlersList, PSafeReference); handler != NULL; ++handler)
    if (handler->GetState () == SIPHandler::Subscribed &&
        handler->GetMethod() == meth &&
        (eventPackage.IsEmpty() || handler->GetEventPackage() == eventPackage))
      count++;
  return count;
}


PStringList SIPHandlersList::GetAddresses(bool includeOffline, SIP_PDU::Methods meth, const PString & eventPackage) const
{
  PStringList addresses;
  for (PSafePtr<SIPHandler> handler(m_handlersList, PSafeReference); handler != NULL; ++handler)
    if ((includeOffline ? handler->GetState () != SIPHandler::Unsubscribed
                        : handler->GetState () == SIPHandler::Subscribed) &&
        handler->GetMethod() == meth &&
        (eventPackage.IsEmpty() || handler->GetEventPackage() == eventPackage))
      addresses.AppendString(handler->GetAddressOfRecord().AsString());
  return addresses;
}


static PString MakeUrlKey(const PURL & aor, SIP_PDU::Methods method, const PString & eventPackage = PString::Empty())
{
  PStringStream key;

  key << method << '\n' << aor;

  if (!eventPackage.IsEmpty())
    key << '\n' << eventPackage;

  return key;
}


/**
  * called when a handler is added
  */

void SIPHandlersList::Append(SIPHandler * obj)
{
  if (obj == NULL)
    return;

  PWaitAndSignal m(m_extraMutex);

  PSafePtr<SIPHandler> handler = m_handlersList.FindWithLock(*obj, PSafeReference);
  if (handler == NULL)
    handler = m_handlersList.Append(obj, PSafeReference);

  // add entry to call to handler map
  handler->m_byCallID = m_byCallID.insert(IndexMap::value_type(handler->GetCallID(), handler));

  // add entry to url and package map
  handler->m_byAorAndPackage = m_byAorAndPackage.insert(IndexMap::value_type(MakeUrlKey(handler->GetAddressOfRecord(), handler->GetMethod(), handler->GetEventPackage()), handler));

  // add entry to username/realm map
  if (!handler->GetUsername().IsEmpty())
    handler->m_byAuthIdAndRealm = m_byAuthIdAndRealm.insert(IndexMap::value_type(handler->GetUsername() + '\n' + handler->GetRealm(), handler));

  handler->m_byAorUserAndRealm = m_byAorUserAndRealm.insert(IndexMap::value_type(handler->GetAddressOfRecord().GetUserName() + '\n' + handler->GetRealm(), handler));
}


/**
  * Called when a handler is removed
  */
void SIPHandlersList::Remove(SIPHandler * handler)
{
  if (handler == NULL)
    return;

  PWaitAndSignal m(m_extraMutex);

  if (m_handlersList.Remove(handler)) {
    if (handler->m_byAorUserAndRealm.second)
      m_byAorUserAndRealm.erase(handler->m_byAorUserAndRealm.first);

    if (handler->m_byAuthIdAndRealm.second)
      m_byAuthIdAndRealm.erase(handler->m_byAuthIdAndRealm.first);

    if (handler->m_byAorAndPackage.second)
      m_byAorAndPackage.erase(handler->m_byAorAndPackage.first);

    if (handler->m_byCallID.second)
      m_byCallID.erase(handler->m_byCallID.first);
  }
}


PSafePtr<SIPHandler> SIPHandlersList::FindBy(IndexMap & by, const PString & key, PSafetyMode mode)
{
  PSafePtr<SIPHandler> ptr;
  {
    PWaitAndSignal m(m_extraMutex);

    IndexMap::iterator r = by.find(key);
    if (r == by.end())
      return NULL;

    ptr = r->second;
  }

  if (ptr->GetState() != SIPHandler::Unsubscribed)
    return ptr.SetSafetyMode(mode) ? ptr : NULL;

  PTRACE(3, "SIP\tHandler " << *ptr << " unsubscribed, awaiting shutdown.");
  while (!ptr->ShutDown())
    PThread::Sleep(100);

  Remove(ptr);
  return NULL;
}


PSafePtr<SIPHandler> SIPHandlersList::FindSIPHandlerByCallID(const PString & callID, PSafetyMode mode)
{
  return FindBy(m_byCallID, callID, mode);
}


PSafePtr<SIPHandler> SIPHandlersList::FindSIPHandlerByUrl(const PURL & aor, SIP_PDU::Methods method, PSafetyMode mode)
{
  return FindBy(m_byAorAndPackage, MakeUrlKey(aor, method), mode);
}


PSafePtr<SIPHandler> SIPHandlersList::FindSIPHandlerByUrl(const PURL & aor, SIP_PDU::Methods method, const PString & eventPackage, PSafetyMode mode)
{
  return FindBy(m_byAorAndPackage, MakeUrlKey(aor, method, eventPackage), mode);
}


/**
 * Find the SIPHandler object with the specified authRealm
 */
PSafePtr<SIPHandler> SIPHandlersList::FindSIPHandlerByAuthRealm (const PString & authRealm, const PString & userName, PSafetyMode mode)
{
  PSafePtr<SIPHandler> ptr;

  if (!userName.IsEmpty()) {
    // look for a match to exact user name and realm
    if ((ptr = FindBy(m_byAuthIdAndRealm, userName + '\n' + authRealm, mode)) != NULL) {
      PTRACE(4, "SIP\tLocated existing credentials for ID \"" << userName << "\" at realm \"" << authRealm << '"');
      return ptr;
    }

      // look for a match to exact user name and empty realm
    if ((ptr = FindBy(m_byAuthIdAndRealm, userName + '\n', mode)) != NULL) {
      PTRACE(4, "SIP\tLocated existing credentials for ID \"" << userName << "\" without realm");
      return ptr;
    }

    // look for a match to exact user name and realm
    if ((ptr = FindBy(m_byAorUserAndRealm, userName + '\n' + authRealm, mode)) != NULL) {
      PTRACE(4, "SIP\tLocated existing credentials for ID \"" << userName << "\" at realm \"" << authRealm << '"');
      return ptr;
    }

      // look for a match to exact user name and empty realm
    if ((ptr = FindBy(m_byAorUserAndRealm, userName + '\n', mode)) != NULL) {
      PTRACE(4, "SIP\tLocated existing credentials for ID \"" << userName << "\" without realm");
      return ptr;
    }
  }

  // look for a match to realm without users
  for (PSafePtr<SIPHandler> handler(m_handlersList, PSafeReference); handler != NULL; ++handler) {
    if (handler->GetRealm() == authRealm && handler.SetSafetyMode(mode)) {
      PTRACE(4, "SIP\tLocated existing credentials for realm \"" << authRealm << '"');
      return handler;
    }
  }

  return NULL;
}


/**
 * Find the SIPHandler object with the specified registration host.
 * For example, in the above case, the name parameter
 * could be "sip.seconix.com" or "seconix.com".
 */
PSafePtr<SIPHandler> SIPHandlersList::FindSIPHandlerByDomain(const PString & name, SIP_PDU::Methods meth, PSafetyMode mode)
{
  for (PSafePtr<SIPHandler> handler(m_handlersList, PSafeReference); handler != NULL; ++handler) {
    if ( handler->GetMethod() == meth &&
         handler->GetState() != SIPHandler::Unsubscribed &&
        (handler->GetAddressOfRecord().GetHostName() == name ||
         handler->GetAddressOfRecord().GetHostAddress().IsEquivalent(name)) &&
         handler.SetSafetyMode(mode))
      return handler;
  }
  return NULL;
}


#endif // OPAL_SIP
