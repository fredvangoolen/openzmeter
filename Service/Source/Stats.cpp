#include <math.h>
#include <string.h>
#include "Stats.h"
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
const float Stats::FrequencyRanges[]  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
const float Stats::VoltageRanges[]    = {-INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY};
const float Stats::THDRanges[]        = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY};
const float Stats::HarmonicsAllowed[] = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
//----------------------------------------------------------------------------------------------------------------------
Stats::Stats(uint8_t channels, Database *db, float volt, float freq) {
  pDB = db;
  pChannels = channels;
  pNominalFreq = freq;
  pNominalVoltage = volt;
  pVoltage   = (uint16_t*)malloc((sizeof(VoltageRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  pFrequency = (uint16_t*)malloc((sizeof(FrequencyRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  pTHD       = (uint16_t*)malloc((sizeof(THDRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  pDB->ExecSentence("PREPARE SAVE_STATS AS INSERT INTO aggreg_stat_1h (sampletime_ext, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES($1, $2, $3, $4, $5)");
  ResetStats();
}
void Stats::ResetStats() {
  memset(pVoltage, 0, (sizeof(VoltageRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  memset(pFrequency, 0, (sizeof(FrequencyRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  memset(pTHD, 0, (sizeof(THDRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  memset(pHarmonics, 0, 2 * sizeof(uint16_t));
}
void Stats::SaveStats(uint64_t timestamp) {
  pDB->PrepareSentence("SAVE_STATS(?, ?, ?, ?, ?)");
  pDB->BindSingleUInt64(timestamp);
  pDB->BindArrayUInt16(pVoltage, sizeof(VoltageRanges) / sizeof(float) - 1);     //estadisticas de tension
  pDB->BindArrayUInt16(pFrequency, sizeof(FrequencyRanges) / sizeof(float) - 1); //estadisticas de frequencia
  pDB->BindArrayUInt16(pTHD, sizeof(THDRanges) / sizeof(float) - 1);             //estadisticas de distorsion harmonica
  pDB->BindArrayUInt16(pHarmonics, 2);                                           //estadisticas de harmonicos en rango
  pDB->ExecSentence();
  ResetStats(); 
}
float Stats::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
void Stats::AppendFreq(float freq, bool flag) {
  if(flag == true) return;
  float Variation = PercentChange(freq, pNominalFreq);
  for(uint Range = 1; Range < (sizeof(FrequencyRanges) / sizeof(float)); Range++) {
    if((Variation >= FrequencyRanges[Range - 1]) && (Variation < FrequencyRanges[Range])) pFrequency[Range - 1]++;
  }
}
void Stats::AppendVoltage(float *voltage, bool flag) {
  if(flag == true) return;
  for(int i = 0; i < pChannels; i++) {
    float Variation = PercentChange(voltage[i], pNominalVoltage);
    for(uint Range = 1; Range < (sizeof(VoltageRanges) / sizeof(float)); Range++) {
      if((Variation >= VoltageRanges[Range - 1]) && (Variation < VoltageRanges[Range])) pVoltage[Range - 1]++;
    }
  }
}
void Stats::AppendTHD(float *thd, bool flag) {
  if(flag == true) return;
  for(int i = 0; i < pChannels; i++) {
    for(uint Range = 1; Range < (sizeof(THDRanges) / sizeof(float)); Range++) {
      if((thd[i] >= THDRanges[Range - 1]) && (thd[i] < THDRanges[Range])) pTHD[Range - 1]++;
    }
  }
}
void Stats::AppendHarmonics(float *harmonics, bool flag) {
  if(flag == true) return;
  for(int i = 0; i < pChannels; i++) {
    bool HarmonicsInRange = true;
    for(int n = 0; n < 25; n++) {
      float Variation = PercentChange(harmonics[i * 50 + n], pNominalVoltage);
      if(Variation > HarmonicsAllowed[n]) {
        HarmonicsInRange = false;
        break;
       }
    }
    if(HarmonicsInRange == true) {
      pHarmonics[0]++;
    } else {
      pHarmonics[1]++;
    }
  }
}


