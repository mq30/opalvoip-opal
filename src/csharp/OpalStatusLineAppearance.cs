/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.9
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class OpalStatusLineAppearance : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal OpalStatusLineAppearance(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(OpalStatusLineAppearance obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~OpalStatusLineAppearance() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          OPALPINVOKE.delete_OpalStatusLineAppearance(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public string line {
    set {
      OPALPINVOKE.OpalStatusLineAppearance_line_set(swigCPtr, value);
    } 
    get {
      string ret = OPALPINVOKE.OpalStatusLineAppearance_line_get(swigCPtr);
      return ret;
    } 
  }

  public OpalLineAppearanceStates state {
    set {
      OPALPINVOKE.OpalStatusLineAppearance_state_set(swigCPtr, (int)value);
    } 
    get {
      OpalLineAppearanceStates ret = (OpalLineAppearanceStates)OPALPINVOKE.OpalStatusLineAppearance_state_get(swigCPtr);
      return ret;
    } 
  }

  public int appearance {
    set {
      OPALPINVOKE.OpalStatusLineAppearance_appearance_set(swigCPtr, value);
    } 
    get {
      int ret = OPALPINVOKE.OpalStatusLineAppearance_appearance_get(swigCPtr);
      return ret;
    } 
  }

  public string callId {
    set {
      OPALPINVOKE.OpalStatusLineAppearance_callId_set(swigCPtr, value);
    } 
    get {
      string ret = OPALPINVOKE.OpalStatusLineAppearance_callId_get(swigCPtr);
      return ret;
    } 
  }

  public string partyA {
    set {
      OPALPINVOKE.OpalStatusLineAppearance_partyA_set(swigCPtr, value);
    } 
    get {
      string ret = OPALPINVOKE.OpalStatusLineAppearance_partyA_get(swigCPtr);
      return ret;
    } 
  }

  public string partyB {
    set {
      OPALPINVOKE.OpalStatusLineAppearance_partyB_set(swigCPtr, value);
    } 
    get {
      string ret = OPALPINVOKE.OpalStatusLineAppearance_partyB_get(swigCPtr);
      return ret;
    } 
  }

  public OpalStatusLineAppearance() : this(OPALPINVOKE.new_OpalStatusLineAppearance(), true) {
  }

}