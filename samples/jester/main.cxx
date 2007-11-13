/*
 * main.cxx
 *
 * Jester - a tester of the Opal jitter buffer
 *
 * Copyright (c) 2006 Derek J Smithies
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
 * The Original Code is Jester
 *
 * The Initial Developer of the Original Code is Derek J Smithies
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: main.cxx,v $
 * Revision 1.14  2007/02/24 09:29:08  dereksmithies
 * Add ability to turn off the first packet in an audio burst. Test the jitter buffer
 * can cope with missing the first packet.
 *
 * Revision 1.13  2007/01/14 22:18:35  dereksmithies
 * MOdify the period when doing silence detect, to be 69sec off,  6 sec on.
 *
 * Revision 1.12  2007/01/14 20:52:32  dereksmithies
 * Report available audio devices if it fails to open the specified device.
 *
 * Revision 1.11  2007/01/13 00:05:40  rjongbloed
 * Fixed compilation on DevStudio 2003
 *
 * Revision 1.10  2007/01/12 10:00:57  dereksmithies
 * bring it up to date so it compiles.
 *
 * Revision 1.9  2007/01/11 09:20:41  dereksmithies
 * Use the new OpalJitterBufer class, allowing easy access to the jitter buffer's internal
 * variables. Play output audio to the specified sound device.
 *
 * Revision 1.8  2006/12/08 09:00:20  dereksmithies
 * Add mutex protection of a pointer.
 * Change default to send packets at a non uniform rate, so they go at 0 20 60 80 120 140 180 etc.
 * Add ability to suppress the sending of packets, so we simulate silence suppression.
 *
 * Revision 1.7  2006/12/02 07:31:00  dereksmithies
 * Add more options - duration of each packet.
 *
 * Revision 1.6  2006/12/02 04:16:13  dereksmithies
 * Get it to terminate correctly.
 * Report on when each frame is commited, and when each frame is received.
 *
 * Revision 1.5  2006/11/23 07:55:15  rjongbloed
 * Fixed sample app build due to RTP session class API breakage.
 *
 * Revision 1.4  2006/10/02 13:30:51  rjongbloed
 * Added LID plug ins
 *
 * Revision 1.3  2006/08/25 06:04:44  dereksmithies
 * Add to the docs on the functions.  Add a new thread to generate the frames,
 * which helps make the operation of the jitterbuffer clearer.
 *
 * Revision 1.2  2006/07/29 13:42:20  rjongbloed
 * Fixed compiler warning
 *
 * Revision 1.1  2006/06/19 09:32:09  dereksmithies
 * Initial cut of a program to test the jitter buffer in OPAL.
 *
 *
 */

#ifdef P_USE_PRAGMA
#pragma implementation "main.h"
#endif


#include <ptlib.h>

#include <opal/buildopts.h>
#include <rtp/rtp.h>

#include "main.h"
#include "../../version.h"


#define new PNEW


PCREATE_PROCESS(JesterProcess);

PBoolean       keepRunning;

///////////////////////////////////////////////////////////////
JesterJitterBuffer::JesterJitterBuffer():
    IAX2JitterBuffer()
{

}

JesterJitterBuffer::~JesterJitterBuffer()
{

}

void JesterJitterBuffer::Close(PBoolean /*reading */ )
{
CloseDown();
}

/////////////////////////////////////////////////////////////////////////////

JesterProcess::JesterProcess()
  : PProcess("Derek Smithies Code Factory", "Jester",
             1, 1, ReleaseCode, 0)
{
}


void JesterProcess::Main()
{
  // Get and parse all of the command line arguments.
  PArgList & args = GetArguments();
  args.Parse(
             "a-audiodevice:"
	     "j-jitter:"
	     "s-silence."
             "h-help."
	     "m-marker."
#if PTRACING
             "o-output:"
             "t-trace."
#endif
	     "v-version."
	     "w-wavfile:"
          , PFalse);


  if (args.HasOption('h') ) {
      cout << "Usage : " << GetName() << " [options] \n"
	  
	  "General options:\n"
	  "  -a --audiodevice      : audio device to play the output on\n"
	  "  -i --iterations #     : number of packets to ask for  (default is 80)\n"
	  "  -s --silence          : simulate silence suppression. - so audio is sent in bursts.\n"
	  "  -j --jitter [min-]max : size of the jitter buffer in ms (100-1000) \n"
	  "  -m --marker           : turn some of the marker bits off, that indicate speech bursts\n"
#if PTRACING
	  "  -t --trace            : Enable trace, use multiple times for more detail.\n"
	  "  -o --output           : File for trace output, default is stderr.\n"
#endif

	  "  -h --help             : This help message.\n"
	  "  -v --version          : report version and program info.\n"
	  "  -w --wavfile          : audio file from which the source data is read from \n"
	  "\n"
	  "\n";
      return;
  }

  if (args.HasOption('v')) {
      cout << GetName()  << endl
	   << " Version " << GetVersion(PTrue) << endl
	   << " by " << GetManufacturer() << endl
	   << " on " << GetOSClass() << ' ' << GetOSName() << endl
	   << " (" << GetOSVersion() << '-' << GetOSHardware() << ")\n\n";
      return;
  }

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
                     PTrace::Timestamp|PTrace::Thread|PTrace::FileAndLine);
#endif

  if (args.HasOption('a'))
      audioDevice = args.GetOptionString('a');
  else
      audioDevice =  PSoundChannel::GetDefaultDevice(PSoundChannel::Player);

  minJitterSize = 100;
  maxJitterSize = 1000;
 
  if (args.HasOption('j')) {
      unsigned minJitterNew;
      unsigned maxJitterNew;
      PStringArray delays = args.GetOptionString('j').Tokenise(",-");

      if (delays.GetSize() > 1) {
	  minJitterNew = delays[0].AsUnsigned();
	  maxJitterNew = delays[1].AsUnsigned();
      } else {
	  maxJitterNew = delays[0].AsUnsigned();
	  minJitterNew = maxJitterNew;
      }

      if (minJitterNew >= 20 && minJitterNew <= maxJitterNew && maxJitterNew <= 1000) {
	  minJitterSize = minJitterNew;
	  maxJitterSize = maxJitterNew;
      } else {
	  cout << "Jitter should be between 20 milliseconds and 1 seconds, is "
	       << 20 << '-' << 1000 << endl;
      }
  } 
  cerr << "Set jitter buffer size to " << minJitterSize << ".." << maxJitterSize << " ms" << endl;

  silenceSuppression = args.HasOption('s');

  markerSuppression = args.HasOption('m');

  if (args.HasOption('w'))
      wavFile = args.GetOptionString('w');
  else {
      wavFile = "../../../contrib/openam/sample_message.wav";
  }
  
  bytesPerBlock = 640;

  if (!PFile::Exists(wavFile)) {
      cerr << "the audio file " << wavFile << " does not exist." << endl;
      cerr << "Terminating now" << endl;
      return;
  }

  if (!player.Open(audioDevice, PSoundChannel::Player, 1, 8000, 16)) {
      cerr <<  "Failed to open the sound device " << audioDevice 
	   << " to write the jittered audio to"  << endl;
      cerr << endl 
	   << "available devices are " << endl;
      PStringList namesPlay = PSoundChannel::GetDeviceNames(PSoundChannel::Player);
      for (PINDEX i = 0; i < namesPlay.GetSize(); i++)
	  cerr << i << "  " << namesPlay[i] << endl;

      cerr << endl;
      cerr << "Terminating now" << endl;      
      return;
  }

  jitterBuffer.SetDelay(8 * minJitterSize, 8 * maxJitterSize);
  jitterBuffer.Resume();

  keepRunning = PTrue;
  generateTimestamp = 0;
  consumeTimestamp = 0;

  PThread * writer = PThread::Create(PCREATE_NOTIFIER(GenerateUdpPackets), 0,
				     PThread::NoAutoDeleteThread,

				     PThread::NormalPriority,
				     "generate");

  PThread::Sleep(10);

  PThread * reader = PThread::Create(PCREATE_NOTIFIER(ConsumeUdpPackets), 0,
				     PThread::NoAutoDeleteThread,
				     PThread::NormalPriority,
				     "consume");


  ManageUserInput();

  writer->WaitForTermination();

  reader->WaitForTermination();

  delete writer;
  delete reader;  
}


void JesterProcess::GenerateUdpPackets(PThread &, INT )
{
    PAdaptiveDelay delay;
    PBoolean lastFrameWasSilence = PTrue;
    PWAVFile soundFile(wavFile);
    generateIndex = 0;
    PINDEX talkSequenceCounter = 0;

    while(keepRunning) {
	generateTimestamp =  (bytesPerBlock * 2) + ((generateIndex  * bytesPerBlock) >> 1);
	//Silence period, 10 seconds cycle, with 3 second on time.
	if (silenceSuppression && ((generateIndex % 1000) > 200)) {
	    PTRACE(3, "Don't send this frame - silence period");
	    if (lastFrameWasSilence == PFalse) {
		PTRACE(3, "Stop Audio here");
		cout << "Stop audio at " << PTime() << endl;
		talkSequenceCounter++;
	    }
	    lastFrameWasSilence = PTrue;
	} else {
	    RTP_DataFrame *frame = new RTP_DataFrame;
	    if (lastFrameWasSilence) {
		PTRACE(3, "StartAudio here");
		cout << "Start Audio at " << PTime() << endl;
	    }
	    frame->SetMarker(lastFrameWasSilence);
	    lastFrameWasSilence = PFalse;
	    frame->SetPayloadType(RTP_DataFrame::L16_Mono);
	    frame->SetSyncSource(0x12345678);
	    frame->SetSequenceNumber((WORD)(generateIndex + 100));
	    frame->SetPayloadSize(bytesPerBlock);
	    
	    frame->SetTimestamp(generateTimestamp);
	    
	    PTRACE(3, "GenerateUdpPacket    iteration " << generateIndex
		   << " with time of " << frame->GetTimestamp() << " rtp time units");
	    memset(frame->GetPayloadPtr(), 0, frame->GetPayloadSize());
	    if (!soundFile.Read(frame->GetPayloadPtr(), frame->GetPayloadSize())) {
		soundFile.Close();
		soundFile.Open();
		PTRACE(3, "Reopen the sound file, as have reached the end of it");
	    }
//	    cerr << " " << silenceSuppression << "  " << markerSuppression << "  " << frame->GetMarker() << "  " << (talkSequenceCounter & 1) << endl;
	    if (silenceSuppression && markerSuppression && frame->GetMarker() && (talkSequenceCounter & 1))
		cerr << "Suppress speech frame" << endl;
	    else
		jitterBuffer.NewFrameFromNetwork(frame);
	}

	delay.Delay(30);
#if 1
	switch (generateIndex % 2) 
	{
	    case 0: 
		break;
	    case 1: PThread::Sleep(30);
		break;
	}
#endif
	generateIndex++;
    }
    PTRACE(3, "End of generate udp packets ");
}


void JesterProcess::ConsumeUdpPackets(PThread &, INT)
{
  RTP_DataFrame readFrame;
  PBYTEArray silence(bytesPerBlock);
  consumeTimestamp = 0;
  consumeIndex = 0;

  while(keepRunning) {

      PBoolean success = jitterBuffer.ReadData(consumeTimestamp, readFrame);
      PTime lastWriteTime;
      if (success && (readFrame.GetPayloadSize() > 0)) {
	  consumeTimestamp = readFrame.GetTimestamp();
	  PTRACE(3, "Write audio to sound device, " << readFrame.GetPayloadSize() << " bytes");
	  player.Write(readFrame.GetPayloadPtr(), readFrame.GetPayloadSize());
	  PTRACE(3, "Play audio from the  buffer to sound device, ts=" << consumeTimestamp);
      }
      else {
	  PTRACE(3, "Write silence to sound device, " << bytesPerBlock << " bytes");
	  player.Write(silence, bytesPerBlock);
	  PTRACE(3, "Play audio from silence buffer to sound device, ts=" << consumeTimestamp);	 
      }
      PTime thisTime;
      PTRACE(3, "Write to sound device took " << (thisTime - lastWriteTime).GetMilliSeconds() << " ms");

      consumeTimestamp += (bytesPerBlock / 2);
      consumeIndex++;
  }

  jitterBuffer.CloseDown();

  PTRACE(3, "End of consume udp packets ");
}

void JesterProcess::ManageUserInput()
{
   PConsoleChannel console(PConsoleChannel::StandardInput);

   PStringStream help;
   help << "Select:\n";
   help << "  X   : Exit program\n"
        << "  Q   : Exit program\n"
	<< "  T   : Read and write process report their current timestamps\n"
	<< "  R   : Report iteration counts\n"
	<< "  J   : Report some of the internal variables in the jitter buffer\n"
        << "  H   : Write this help out\n";

   PThread::Sleep(100);


 for (;;) {
    // display the prompt
    cout << "(Jester) Command ? " << flush;

    // terminate the menu loop if console finished
    char ch = (char)console.peek();
    if (console.eof()) {
      cout << "\nConsole gone - menu disabled" << endl;
      goto endAudioTest;
    }

    console >> ch;
    PTRACE(3, "console in audio test is " << ch);
    switch (tolower(ch)) {
        case 'q' :
        case 'x' :
            goto endAudioTest;
	case 'r' :
	    cout << "        generate thread=" << generateIndex << "    consume thread=" << consumeIndex << endl;
	    break;
        case 'h' :
            cout << help ;
            break;
	case 't' :
	    cerr << "        Timestamps are " << generateTimestamp << "/" << consumeTimestamp << " (generate/consume)" << endl;
	    DWORD answer;
	    if (generateTimestamp > consumeTimestamp)
		answer = generateTimestamp - consumeTimestamp;
	    else
		answer = consumeTimestamp - generateTimestamp;
	    cerr << "        RTP difference " << answer << "          Milliseconds difference is "  << (answer/8) << endl;
	    break;
	case 'j' :
	    cerr << "        Target Jitter Time is  " << jitterBuffer.GetTargetJitterTime() << endl;
	    cerr << "        Current depth is       " << jitterBuffer.GetCurrentDepth() << endl;
	    cerr << "        Current Jitter Time is " << jitterBuffer.GetCurrentJitterTime() << endl;
	    break;
        default:
            ;
    }
  }

endAudioTest:
  keepRunning = PFalse;
  cout  << "end audio test" << endl;
}



// End of File ///////////////////////////////////////////////////////////////
