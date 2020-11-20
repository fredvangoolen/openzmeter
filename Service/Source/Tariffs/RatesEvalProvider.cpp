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
#include "RatesEvalProvider.h"
// ---------------------------------------------------------------------------------------------------------------------
RatesEvalProvider::RatesEvalProvider(const time_zone &tz, const json &rates, const json &samples) {
  pLoadedSample = NULL;
  pLoadedRate   = NULL;
  pTimeZone     = tz;
  pRates        = rates;
  pRatesIdx     = 0;
  pSamples      = samples;
  pSamplesIdx   = 0;
}
RatesEvalProvider::~RatesEvalProvider() {
  if(pLoadedRate) delete pLoadedRate;
  if(pLoadedSample) free(pLoadedSample);
}
Rate *RatesEvalProvider::InitRates(const uint64_t from, const uint64_t to) {
  while(pRates.size() > pRatesIdx) {
    json JsonRate = Rate::CheckConfig(pRates[pRatesIdx++]);
    if(JsonRate.is_string()) {
      printf("%s\n", JsonRate.dump().c_str());
      continue;
    }
    if(pLoadedRate != NULL) {
      delete pLoadedRate;
      pLoadedRate = NULL;
    }
    pLoadedRate = new Rate(JsonRate, pTimeZone);
    if(pLoadedRate->EndDate >= from) return pLoadedRate;
  }
  return NULL;
}
RatesEvalProvider::Sample_t *RatesEvalProvider::InitSamples(const uint64_t from, const uint64_t to) {
  while(pSamples.size() > pSamplesIdx) {
    json JsonSample = pSamples[pSamplesIdx++];
    if((JsonSample.contains("SampleTime") == false) || (JsonSample["SampleTime"].is_number() == false) || (JsonSample.contains("Active_Power") == false) || (JsonSample["Active_Power"].is_number() == false) || (JsonSample.contains("Reactive_Power") == false) || (JsonSample["Reactive_Power"].is_number() == false)) return NULL;
    if(pLoadedSample == NULL) pLoadedSample = (Sample_t*)malloc(sizeof(Sample_t));
    pLoadedSample->Time     = pSamples[pSamplesIdx]["SampleTime"];
    pLoadedSample->Active   = pSamples[pSamplesIdx]["Active_Power"];
    pLoadedSample->Reactive = pSamples[pSamplesIdx]["Reactive_Power"];
    pSamplesIdx++;
    if(pLoadedSample->Time >= from) return pLoadedSample;
  }
  return NULL;
}
Rate *RatesEvalProvider::NextRate() {
  if(pLoadedRate != NULL) {
    delete pLoadedRate;
    pLoadedRate = NULL;
  }
  if(pRates.size() > pRatesIdx) {
    json JsonRate = Rate::CheckConfig(pRates[pRatesIdx++]);
    if(JsonRate.is_string()) return NULL;
    pLoadedRate = new Rate(JsonRate, pTimeZone);
  }
  return pLoadedRate;
}
RatesEvalProvider::Sample_t *RatesEvalProvider::NextSample() {
  if(pLoadedSample == NULL) return NULL;
  if(pSamples.size() > pSamplesIdx) {
    pLoadedSample->Time     = pSamples[pSamplesIdx]["SampleTime"];
    pLoadedSample->Active   = pSamples[pSamplesIdx]["Active_Power"];
    pLoadedSample->Reactive = pSamples[pSamplesIdx]["Reactive_Power"];
    pSamplesIdx++;
    return pLoadedSample;
  } else {
    return NULL;
  }
}
// ---------------------------------------------------------------------------------------------------------------------