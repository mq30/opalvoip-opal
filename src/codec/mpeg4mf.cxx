/*
 * mpeg4mf.cxx
 *
 * MPEG4 part 2 Media Format descriptions
 *
 * Open Phone Abstraction Library
 *
 * Copyright (c) 2012 Vox Lucida Pty. Ltd.
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
 * The Original Code is Open Phone Abstraction Library
 *
 * The Initial Developer of the Original Code is Vox Lucida
 *
 * Contributor(s): ______________________________________.
 *
 */

#include <ptlib.h>

#include <opal_config.h>

#if OPAL_VIDEO

#include "mpeg4mf_inc.cxx"

#include <opal/mediafmt.h>
#include <h323/h323caps.h>


class OpalMPEG4ProfileAndLevel : public OpalMediaOptionUnsigned
{
  public:
    OpalMPEG4ProfileAndLevel()
      : OpalMediaOptionUnsigned(ProfileAndLevel, true, OpalMediaOption::IntersectionMerge, 5, 1, 127)
    {
      OPAL_SET_MEDIA_OPTION_FMTP(this, ProfileAndLevel_SDP, "1");
      OPAL_SET_MEDIA_OPTION_H245(this, H245_ANNEX_E_PROFILE_LEVEL);
    }

    virtual bool Merge(const OpalMediaOption & option)
    {
      const OpalMediaOptionUnsigned & other = dynamic_cast<const OpalMediaOptionUnsigned &>(option);
      SetValue(MergeProfileAndLevelOption(GetValue(), other.GetValue()));
      return true;
    }
};


class OpalMPEG4FormatInternal : public OpalVideoFormatInternal
{
  public:
    OpalMPEG4FormatInternal()
      : OpalVideoFormatInternal(MPEG4FormatName, RTP_DataFrame::DynamicBase, MPEG4EncodingName,
                                PVideoFrameInfo::MaxWidth, PVideoFrameInfo::MaxHeight, 30, 16000000)
    {
      AddOption(new OpalMPEG4ProfileAndLevel());

      OpalMediaOption * option = new OpalMediaOptionUnsigned(H245ObjectId, true, OpalMediaOption::EqualMerge, 0, 0, 15);
      OPAL_SET_MEDIA_OPTION_H245(option, H245_ANNEX_E_OBJECT);
      OPAL_SET_MEDIA_OPTION_FMTP(option, Object_SDP, "0");
      AddOption(option);

      option = new OpalMediaOptionOctets(DCI, true);
      OPAL_SET_MEDIA_OPTION_H245(option, H245_ANNEX_E_DCI);
      OPAL_SET_MEDIA_OPTION_FMTP(option, DCI_SDP, "");
      AddOption(option);

#if OPAL_H323
      AddOption(new OpalMediaOptionString(PLUGINCODEC_MEDIA_PACKETIZATIONS, false, OpalPluginCodec_Identifer_MPEG4));
#endif
    }

    virtual PObject * Clone() const
    {
      return new OpalMPEG4FormatInternal(*this);
    }


    virtual bool ToNormalisedOptions()
    {
      return AdjustByOptionMaps(PTRACE_PARAM("ToNormalised",) MyToNormalised) && OpalVideoFormatInternal::ToNormalisedOptions();
    }

    virtual bool ToCustomisedOptions()
    {
      return AdjustByOptionMaps(PTRACE_PARAM("ToCustomised",) MyToCustomised) && OpalVideoFormatInternal::ToCustomisedOptions();
    }
};


#if OPAL_H323
extern const char MPEG4_Identifer[] = OpalPluginCodec_Identifer_MPEG4;
typedef OpalMediaFormatStaticH323<OpalVideoFormat, H323GenericVideoCapabilityTemplate<MPEG4_Identifer, GetOpalMPEG4>> OpalMPEG4Format;
#else
typedef OpalMediaFormatStatic<OpalVideoFormat> OpalMPEG4Format;
#endif



const OpalVideoFormat & GetOpalMPEG4()
{
  static OpalMPEG4Format const format(new OpalMPEG4FormatInternal);
  return format;
}


struct OpalKeyFrameDetectorMPEG4 : OpalVideoFormat::FrameDetector
{
  virtual OpalVideoFormat::FrameType GetFrameType(const BYTE * rtp, PINDEX size)
  {
    if (size < 4 || rtp[0] != 0 || rtp[1] != 0 || rtp[2] != 1)
      return OpalVideoFormat::e_UnknownFrameType;

    while (size > 4) {
      if (rtp[0] == 0 && rtp[1] == 0 && rtp[2] == 1) {
        if (rtp[3] == 0xb6) {
          switch ((rtp[4] & 0xC0) >> 6) {
            case 0:
              return OpalVideoFormat::e_IntraFrame;
            case 1:
              return OpalVideoFormat::e_InterFrame;
          }
        }
      }
      ++rtp;
      --size;
    }

    return OpalVideoFormat::e_NonFrameBoundary;
  }
};

PFACTORY_CREATE(OpalVideoFormat::FrameDetectFactory, OpalKeyFrameDetectorMPEG4, "MP4V-ES");


#endif // OPAL_VIDEO


// End of File ///////////////////////////////////////////////////////////////
