// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
// Driver for the S1 high speed port of Belgian (Fluvius) digital electricity meters, as specified in e-MUCS H ed. 1.3
// section 4. The meter continuously outputs one 45 byte HDLC-like frame per sample over RS-422 at 2 Mbaud (8N1). Frames
// carry synchronous instantaneous voltage (L1..L3) and current (L1..L3, N) samples, the grid frequency measured by the
// meter and the sampling configuration. A USB to RS-422 adapter able to run at exactly 2 Mbaud is required (FTDI
// FT232R/FT232H/FT2232H, CP2102N...).
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <pthread.h>
#include "Device.h"
#include "AnalyzerSoft.h"
#include "DeviceS1Frame.h"
#include "DeviceS1Rate.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceS1 final : public Device {
  public :
    static const uint32_t BAUDRATE       = 2000000;  //Fixed S1 data rate
    static const uint8_t  MAX_GAP_FILL   = 16;       //Max lost frames filled holding the last sample
    typedef DeviceS1Frame::Frame_t Frame_t;
  private :
    static bool                     pRegistered;
  private :
    pthread_t                       pThread;           //Capture thread
    pthread_mutex_t                 pLock;             //Protects analyzer pointer and status
    AnalyzerSoft                   *pAnalyzer;         //Current analyzer (created when data arrives)
    volatile bool                   pTerminate;        //Thread stop flag
    string                          pPort;             //Serial port device path
    string                          pModeConfig;       //AUTO or forced analyzer mode
  private :
    Analyzer::AnalyzerMode          pMode;             //Mode of the current analyzer
    float                           pNominalRate;      //Nominal samplerate of the current analyzer
    bool                            pPerPeriod;        //Sampling type of the current analyzer
    bool                            pConnected;        //Serial port is open
    bool                            pReceiving;        //Valid frames are being received
    Frame_t                         pLastFrame;        //Last decoded frame (status)
    uint64_t                        pFramesOK;         //Valid frames counter
    uint64_t                        pCRCErrors;        //Frames dropped by CRC/format
    uint64_t                        pLostFrames;       //Frames lost (sequence gaps)
    uint64_t                        pInvalidSamples;   //Frames flagged as not valid by the meter
    uint64_t                        pResyncs;          //Bytes discarded searching frame start
    float                           pHeldVoltage[3];   //Last valid voltage samples (gap fill / invalid samples)
    float                           pHeldCurrent[4];   //Last valid current samples (gap fill / invalid samples)
    bool                            pHaveHeld;         //pHeldVoltage/pHeldCurrent are initialized
    uint8_t                         pLastSequence;     //Last frame sequence number
    bool                            pHaveSequence;     //pLastSequence is initialized
    DeviceS1Rate                    pRate;             //Real sample rate of the stream (capture thread only)
    uint32_t                        pNewSamples;       //Samples received since last pRate update (capture thread only)
    float                           pMeasuredRate;     //Last measured rate, 0 if not known yet (status)
  private :
    static bool  Register();
    static void  GetInstances();
    static json  CheckConfig(const json &config);
  private :
    DeviceS1();
    void    *Thread();
    void     ProcessFrame(const Frame_t &frame);
    bool     EnsureAnalyzer(const Frame_t &frame);
    uint32_t AnalyzersCount() override;
    void     ClearAnalyzers();
    void     Config(const json &config);
    void     GetDevice(HTTPRequest &req) override;
    void     SetDevice(HTTPRequest &req) override;
  public :
    ~DeviceS1();
};
//----------------------------------------------------------------------------------------------------------------------
