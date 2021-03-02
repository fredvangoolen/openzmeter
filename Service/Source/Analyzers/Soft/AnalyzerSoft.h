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
#include "Worker.h"
#include "ProviderLocks.h"
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
#include "AnalyzerSoftStorage.h"
#include "AnalyzerSoftFrequency.h"
#include "AnalyzerSoftSync.h"
#include "AnalyzerSoftMQTT.h"
#include "AnalyzerSoftAlarms.h"

#include "AnalyzerSeries.h"
#include "AnalyzerRates.h"
#include "AnalyzerAccess.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace httplib;
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft : public Analyzer {
  private : //Not modified after creation
    const float                pSamplerate;              //Velocidad de muestreo
    const string               pConfigPath;              //Path to configuration
    const uint8_t              pVoltageChannels;         //Voltage channels
    const uint8_t              pCurrentChannels;         //Current channels
    AnalyzerAccess             pAccess;                  //Access control class
    AnalyzerSoftZeroCrossing   pZeroCross;               //Zero crosing and block builder
    AnalyzerSoftFrequency      pFrequency;               //Frequency meter
    AnalyzerSoftFFTs           pFFTs;                    //FFT executor
    AnalyzerSoftAggreg200MS    pAggreg200MS;             //200ms aggregation
    AnalyzerSoftAggreg3S       pAggreg3S;                //3 seconds aggregation
    AnalyzerSoftAggreg1M       pAggreg1M;                //1 minute aggregation
    AnalyzerSoftAggreg10M      pAggreg10M;               //10 minutes aggregation
    AnalyzerSoftAggreg15M      pAggreg15M;               //15 minutes aggregation
    AnalyzerSoftAggreg1H       pAggreg1H;                //1 hour aggregation


    AnalyzerRates              pRates;                   //Rate handle class
    AnalyzerSeries             pSeries;                  //Read stored data
    AnalyzerSoftEvents         pEvents;                  //Events detector
    AnalyzerSoftAlarms         pAlarms;                  //Alarms control
    AnalyzerSoftSync           pSync;                    //Remote sync object
    AnalyzerSoftMQTT           pMQTT;                    //MQTT Control object
    AnalyzerSoftGA            *pGA;                      //Geometric algebra calculator
    AnalyzerSoftStorage       *pStorage;                 //Control stored data
    AnalyzerSoftBlock         *pWorkingBlock;            //Current block
  private :
    volatile bool              pTerminate;               //Terminate thread
    deque<AnalyzerSoftBlock*>  pPendingBlocks;           //Pending blocks to analyze
    pthread_t                  pThread;                  //Analyzer thread
    pthread_mutex_t            pMutex;                   //Locks access to analysis shared variables
    pthread_cond_t             pCond;                    //Thread wake condition
    float                      pNominalVoltage;          //Configured nominal voltage
    float                      pNominalFrequency;        //Configured nominal frequency
    bool                       pSave200MS;               //Save 10/12 cycles aggregation to database
    ProviderLocks              pNowDataLock;             //GetSeriesNow access control
  private :
    void Thread();
    void Config(const json &options);
    json GetSeriesNow(const json &series);
    json GetSeriesNow();
  public :
    void GetAnalyzer(const json &params, Response &response) override;
    void SetAnalyzer(const json &params, Response &response) override;
    void GetStatus(const json &params, Response &response) override;
    void GetSeries(const json &params, Response &response) override;
    void SetSeries(const json &params, Response &response) override;
    void GetSeriesNow(const json &params, Response &response) override;
    void GetCosts(const json &params, Response &response) override;
    void GetBills(const json &params, Response &response) override;
    void GetWaveStream(const json &params, Response &response) override;
    void GetSeriesRange(const json &params, Response &response) override;
    void DelSeriesRange(const json &params, Response &response) override;
    void GetEvents(const json &params, Response &response) override;
    void SetEvents(const json &params, Response &response) override;
    void GetEventsRange(const json &params, Response &response) override;
    void DelEventsRange(const json &params, Response &response) override;
    void GetEvent(const json &params, Response &response) override;
    void SetEvent(const json &params, Response &response) override;
    void DelEvent(const json &params, Response &response) override;
    void TestAlarm(const json &params, Response &response) override;
    void GetAlarms(const json &params, Response &response) override;
    void SetAlarms(const json &params, Response &response) override;
    void GetAlarmsRange(const json &params, Response &response) override;
    void DelAlarm(const json &params, Response &response) override;
    void DelAlarmsRange(const json &params, Response &response) override;
    void SetRemote(const json &params, Response &response) override;
    bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) override;
  public :
    static json CheckConfig(const json &config, const Analyzer::AnalyzerMode mode, const bool except);
  protected :
    virtual void ResultsReady(AnalyzerSoftAggreg200MS &params);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    void BufferAppend(float *voltageSamples, float *currentSamples);
};
//----------------------------------------------------------------------------------------------------------------------