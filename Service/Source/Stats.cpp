//const float Analyzer::FrequencyRanges[]  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
//const float Analyzer::VoltageRanges[]    = {-INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY};
//const float Analyzer::THDRanges[]        = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY};
//const float Analyzer::HarmonicsAllowed[] = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};


//void Analyzer::ResetStats() {
//  memset(pStats.Voltage, 0, (sizeof(VoltageRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  memset(pStats.Frequency, 0, (sizeof(FrequencyRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  memset(pStats.THD, 0, (sizeof(THDRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  memset(pStats.Harmonics, 0, 2 * sizeof(uint16_t));
//}


//  pStats.Voltage   = (uint16_t*)malloc((sizeof(VoltageRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  pStats.Frequency = (uint16_t*)malloc((sizeof(FrequencyRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  pStats.THD       = (uint16_t*)malloc((sizeof(THDRanges) / sizeof(float) - 1) * sizeof(uint16_t));
//  ResetStats();
//ExecSentence("PREPARE SAVE_1H       AS INSERT INTO aggreg_1h (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14)");

//  if(params->Type == AGGREG_1H) {
//    BindArrayUInt16(pStats.Voltage, sizeof(VoltageRanges) / sizeof(float) - 1);     //estadisticas de tension
//    BindArrayUInt16(pStats.Frequency, sizeof(FrequencyRanges) / sizeof(float) - 1); //estadisticas de frequencia
//    BindArrayUInt16(pStats.THD, sizeof(THDRanges) / sizeof(float) - 1);             //estadisticas de distorsion harmonica
//    BindArrayUInt16(pStats.Harmonics, 2);                                           //estadisticas de harmonicos en rango
//  }