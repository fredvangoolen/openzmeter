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
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include "cctz/time_zone.h"
#include "cpp-httplib/httplib.h"
#include "Analyzer.h"
#include "ConsumerQueue.h"
#include "AnalyzerSoftFFTs.h"
#include "AnalyzerSoftBlock.h"
#include "AnalyzerSoftZeroCrossing.h"
#include "AnalyzerSoftEvents.h"
#include "AnalyzerSoftGA.h"
#include "AnalyzerSoftAggreg200MS.h"
#include "AnalyzerSoftAggreg3S.h"
#include "AnalyzerSoftAggreg1M.h"
#include "AnalyzerSoftAggreg10M.h"
#include "AnalyzerSoftAggreg15M.h"
#include "AnalyzerSoftAggreg1H.h"
#include "AnalyzerSoftFrequency.h"
#include "AnalyzerSoftSync.h"
#include "AnalyzerSoftMQTT.h"
#include "AnalyzerSoftAlarms.h"
#include "AnalyzerSeries.h"
#include "AnalyzerRates.h"
#include "AnalyzerAccess.h"
#include "AnalyzerSoftCyclicHistogram.h"
#include "AnalyzerSoftJoltAlgorithm.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace httplib;
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft : public Analyzer {
  private :
    const float                pSamplerate;              //Velocidad de muestreo
    const string               pConfigPath;              //Path to configuration
    const uint8_t              pVoltageChannels;         //Voltage channels
    const uint8_t              pCurrentChannels;         //Current channels
  private :
    AnalyzerAccess            *pAccess;                  //Access control class
    AnalyzerSoftZeroCrossing  *pZeroCross;               //Zero crosing and block builder
    AnalyzerSoftFrequency     *pFrequency;               //Frequency meter
    AnalyzerSoftFFTs          *pFFTs;                    //FFT executor
    AnalyzerSoftAggreg200MS   *pAggreg200MS;             //200ms aggregation
    AnalyzerSoftAggreg3S      *pAggreg3S;                //3 seconds aggregation
    AnalyzerSoftAggreg1M      *pAggreg1M;                //1 minute aggregation
    AnalyzerSoftAggreg10M     *pAggreg10M;               //10 minutes aggregation
    AnalyzerSoftAggreg15M     *pAggreg15M;               //15 minutes aggregation
    AnalyzerSoftAggreg1H      *pAggreg1H;                //1 hour aggregation
    AnalyzerRates             *pRates;                   //Rate handle class
    AnalyzerSeries            *pSeries;                  //Read stored data
    AnalyzerSoftEvents        *pEvents;                  //Events detector
    AnalyzerSoftMQTT          *pMQTT;                    //MQTT Control object
    AnalyzerSoftAlarms        *pAlarms;                  //Alarms control
    AnalyzerSoftSync          *pSync;                    //Remote sync object
    AnalyzerSoftBlock         *pWorkingBlock;            //Current block
    ConsumerQueue             *pConsumers;               //Block analysis consumers control
    AnalyzerSoftCyclicHistogram *pCyclicHistogram;       //Cyclic Histogram analyzer
    AnalyzerSoftJoltAlgorithm *pJoltAlgorithm;           //Jolt Algorithm analyzer
  private :
    volatile bool              pTerminate;               //Terminate thread
    deque<AnalyzerSoftBlock*>  pPendingBlocks;           //Pending blocks to analyze
    pthread_t                  pThread;                  //Analyzer thread
    pthread_mutex_t            pMutex;                   //Locks access to analysis shared variables
    pthread_cond_t             pCond;                    //Thread wake condition
    float                      pNominalVoltage;          //Configured nominal voltage
    float                      pNominalFrequency;        //Configured nominal frequency
    bool                       pSave200MS;               //Save 10/12 cycles aggregation to database
  private :
    void Thread();
    void Config(const json &options);
    json GetSeriesNow(const vector<string> &series);
    void HandleAction(const string &name, const json &params);
  public :
    void GetAnalyzer(HTTPRequest &req) override;
    void SetAnalyzer(HTTPRequest &req) override;
    void GetStatus(HTTPRequest &req) override;
    void GetSeries(HTTPRequest &req) override;
    void SetSeries(HTTPRequest &req) override;
    void GetSeriesNow(HTTPRequest &req) override;
    void GetCyclicHistogramConfig(HTTPRequest &req) override;
    void SetCyclicHistogramConfig(HTTPRequest &req) override;
    void GetCyclicHistogramData(HTTPRequest &req) override;
    void ClearCyclicHistogramData(HTTPRequest &req) override;
    void GetJoltData(HTTPRequest &req) override;
    void GetCosts(HTTPRequest &req) override;
    void GetBills(HTTPRequest &req) override;
    void GetWaveStream(HTTPRequest &req) override;
    void GetSeriesRange(HTTPRequest &req) override;
    void DelSeriesRange(HTTPRequest &req) override;
    void GetEvents(HTTPRequest &req) override;
    void SetEvents(HTTPRequest &req) override;
    void GetEventsRange(HTTPRequest &req) override;
    void DelEventsRange(HTTPRequest &req) override;
    void GetEvent(HTTPRequest &req) override;
    void SetEvent(HTTPRequest &req) override;
    void DelEvent(HTTPRequest &req) override;
    void TestAlarm(HTTPRequest &req) override;
    void GetAlarms(HTTPRequest &req) override;
    void SetAlarms(HTTPRequest &req) override;
    void SetAlarm(HTTPRequest &req) override;
    void DelAlarm(HTTPRequest &req) override;
    void GetAlarmsRange(HTTPRequest &req) override;
    void DelAlarmsRange(HTTPRequest &req) override;
    void SetRemote(HTTPRequest &req) override;
    bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) override;
  public :
    static json CheckConfig(const json &config, const Analyzer::AnalyzerMode mode, const bool except);
  protected :
    virtual void ResultsReady(AnalyzerSoftAggreg200MS *params);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    void BufferAppend(const float *voltageSamples, const float *currentSamples);
};
//----------------------------------------------------------------------------------------------------------------------