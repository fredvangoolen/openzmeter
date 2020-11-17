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
#include "AnalyzerSoftRatesProvider.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRatesProvider::AnalyzerSoftRatesProvider(const string &serial, const time_zone &tz, const uint8_t group) : RatesEvalProvider(tz) {
  pRateDB     = new Database(serial);
  pDataDB     = new Database(serial);
  pGroup      = group;
//  pLastSample = 0;
}
AnalyzerSoftRatesProvider::~AnalyzerSoftRatesProvider() {
  delete pRateDB;
  delete pDataDB;
}
Rate *AnalyzerSoftRatesProvider::InitRates(const uint64_t from, const uint64_t to) {
  pRateDB->ExecSentenceAsyncResult("SELECT jsonb_build_object('Name', name, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) || template::jsonb AS result FROM rates WHERE ordering = " + pRateDB->Bind(pGroup) + " AND " +
                                   "enddate >= " + pRateDB->Bind(format("%Y-%m-%d", chrono::system_clock::time_point(chrono::milliseconds(from)), utc_time_zone())) + " AND " +
                                   "startdate <= " + pRateDB->Bind(format("%Y-%m-%d", chrono::system_clock::time_point(chrono::milliseconds(to)), utc_time_zone())) + " " +
                                   "ORDER BY startdate ASC");
  if(pLoadedRate != NULL) {
    delete pLoadedRate;
    pLoadedRate = NULL;
  }
  if(pRateDB->FecthRow() == true) {
    pLoadedRate = new Rate(pRateDB->ResultJSON("result"), pTimeZone);
    return pLoadedRate;
  } else {
    return NULL;
  }
}
RatesEvalProvider::Sample_t *AnalyzerSoftRatesProvider::InitSamples(const uint64_t from, const uint64_t to) {
  pDataDB->ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive FROM aggreg_15m WHERE sampletime >= " + pDataDB->Bind(from) + " ORDER BY sampletime ASC LIMIT 1000");
  if(pDataDB->FecthRow() == true) {
    if(pLoadedSample == NULL) pLoadedSample = (Sample_t*)malloc(sizeof(Sample_t));
    pLoadedSample->Active   = pDataDB->ResultFloat("active");
    pLoadedSample->Reactive = pDataDB->ResultFloat("reactive");
    pLoadedSample->Time     = pDataDB->ResultUInt64("sampletime");
  } else {
    if(pLoadedSample != NULL) {
      free(pLoadedSample);
      pLoadedSample = NULL;
    }
  }
  return pLoadedSample;
}
Rate *AnalyzerSoftRatesProvider::NextRate() {
  if(pLoadedRate != NULL) {
    delete pLoadedRate;
    pLoadedRate = NULL;
  }
  if(pRateDB->FecthRow() == true) {
    pLoadedRate = new Rate(pRateDB->ResultJSON("result"), pTimeZone);
  }
  return pLoadedRate;
}
RatesEvalProvider::Sample_t *AnalyzerSoftRatesProvider::NextSample() {
  if(pDataDB->FecthRow() == true) {
    if(pLoadedSample == NULL) pLoadedSample = (Sample_t*)malloc(sizeof(Sample_t));
    pLoadedSample->Active   = pDataDB->ResultFloat("active");
    pLoadedSample->Reactive = pDataDB->ResultFloat("reactive");
    pLoadedSample->Time     = pDataDB->ResultUInt64("sampletime");
  } else {
    if(pLoadedSample == NULL) return NULL;
    pDataDB->ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive FROM aggreg_15m WHERE sampletime >= " + pDataDB->Bind(pLoadedSample->Time + 1) + " ORDER BY sampletime ASC LIMIT 1000");
    pDataDB->FecthRow();
    if(pDataDB->FecthRow() == true) {
      pLoadedSample->Active   = pDataDB->ResultFloat("active");
      pLoadedSample->Reactive = pDataDB->ResultFloat("reactive");
      pLoadedSample->Time     = pDataDB->ResultUInt64("sampletime");
    } else {
      if(pLoadedSample != NULL) {
        free(pLoadedSample);
        pLoadedSample = NULL;
      }
    }
  }
  return pLoadedSample;
}
// ---------------------------------------------------------------------------------------------------------------------