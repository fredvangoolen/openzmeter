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
#include "AnalyzerSoftFFTs.h"
#include "AnalyzerSoftRates.h"
#include "AnalyzerSoftBlock.h"
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
#include "AnalyzerSoftCyclicHistogram.h"
#include "AnalyzerSoftJoltAlgorithm.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace httplib;
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAlarms;
class AnalyzerSoft : public Analyzer {
  private : //Not modified after creation
    const float                pSamplerate;              //Velocidad de muestreo
    const string               pConfigPath;              //Path to configuration
    const uint8_t              pVoltageChannels;         //Voltage channels
    const uint8_t              pCurrentChannels;         //Current channels
    const AnalyzerMode         pMode;                    //Operation mode
  private : //Internal classes
    AnalyzerSoftSync          *pSync;                    //Remote sync object
    AnalyzerSoftMQTT          *pMQTT;                    //MQTT Control object
    AnalyzerSoftRates         *pRates;                   //Rate handle class
    AnalyzerSoftBlock         *pInputBlock;              //Block maker class
    AnalyzerSoftAlarms        *pAlarms;                  //Alarms control
    AnalyzerSoftEvents        *pEvents;                  //Events detector
    AnalyzerSoftFrequency     *pFrequency;               //Frequency meter
    AnalyzerSoftFFTs          *pFFTs;                    //FFT executor
    AnalyzerSoftGA            *pGA;                      //Geometric algebra calculator
    AnalyzerSoftAggreg200MS   *pAggreg200MS;             //200ms aggregation
    AnalyzerSoftAggreg3S      *pAggreg3S;                //3 seconds aggregation
    AnalyzerSoftAggreg1M      *pAggreg1M;                //1 minute aggregation
    AnalyzerSoftAggreg10M     *pAggreg10M;               //10 minutes aggregation
    AnalyzerSoftAggreg15M     *pAggreg15M;               //15 minutes aggregation
    AnalyzerSoftAggreg1H      *pAggreg1H;                //1 hour aggregation
    AnalyzerSoftStorage       *pStorage;                 //Control stored data
    AnalyzerSoftCyclicHistogram *pCyclicHistogram;       //Cyclic Histogram analyzer
    AnalyzerSoftJoltAlgorithm *pJoltAlgorithm;           //Jolt Algorithm analyzer
  private :
    pthread_mutex_t            pMutex;                   //Locks access to analysis shared variables
    bool                       pSave200MS;               //Save 10/12 cycles aggregation to database
    ProviderLocks              pNowDataLock;             //GetSeriesNow access control
  private : //Workers
    Worker                    *pAnalyzerWorker;          //Analyzer worker
  private :
    void Analyzer_Func();
    void Configure(const json &options);
    void PeriodicTasks();
  public :
    void GetAnalyzer(const json &params, Response &response) override;
    void SetAnalyzer(const json &params, Response &response) override;
    void GetStatus(const json &params, Response &response) override;
    void GetSeries(const json &params, Response &response) override;
    void GetSeriesNow(const json &params, Response &response) override;
    void GetCyclicHistogram(const json &params, Response &response) override;
    void GetJoltData(const json &params, Response &response) override;
    void GetCosts(const json &params, Response &response) override;
    void GetBills(const json &params, Response &response) override;
    void GetWaveStream(const json &params, Response &response) override;
    void GetSeriesRange(const json &params, Response &response) override;
    void DelSeriesRange(const json &params, Response &response) override;
    void GetEvents(const json &params, Response &response) override;
    void GetEventsRange(const json &params, Response &response) override;
    void DelEventsRange(const json &params, Response &response) override;
    void GetEvent(const json &params, Response &response) override;
    void SetEvent(const json &params, Response &response) override;
    void DelEvent(const json &params, Response &response) override;
    void GetAlarmsDef(const json &params, Response &response) override;
    void TestAlarm(const json &params, Response &response) override;
    void AddAlarmDef(const json &params, Response &response) override;
    void GetAlarmDef(const json &params, Response &response) override;
    void SetAlarmDef(const json &params, Response &response) override;
    void DelAlarmDef(const json &params, Response &response) override;
    void GetAlarms(const json &params, Response &response) override;
    void GetAlarmsRange(const json &params, Response &response) override;
    void DelAlarm(const json &params, Response &response) override;
    void DelAlarmsRange(const json &params, Response &response) override;
    void GetRates(const json &params, Response &response) override;
    void AddRate(const json &params, Response &response) override;
    void GetRate(const json &params, Response &response) override;
    void SetRate(const json &params, Response &response) override;
    void DelRate(const json &params, Response &response) override;
    bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) override;
  public :
    static json CheckConfig(const json &config, const Analyzer::AnalyzerMode mode);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    bool BufferAppend(float *voltageSamples, float *currentSamples, const uint32_t buffUsage);
    bool IsWorking();
};
//----------------------------------------------------------------------------------------------------------------------