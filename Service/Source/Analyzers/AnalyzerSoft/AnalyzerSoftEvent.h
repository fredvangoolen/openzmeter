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
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
#define EVENT_RVC                        5.0
#define EVENT_DIP_SWELL                 10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION              90.0        //Bajada en % para generar interrupcion
#define EVENT_INTERRUPTION_HYSTERESIS   40.0        //Interruption hysteresis
#define EVENT_HYSTERESIS                02.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_RAWENVELOPETIME            0.5        //Time in seconds before and after event to store RAW samples
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftEvent final {
  private :
    enum EventType   { EVT_NONE = 0x00, EVT_SWELL = 0x01, EVT_DIP = 0x02, EVT_INTERRUPTION = 0x04, EVT_RVC = 0x08 };
  private :
    uint8_t        pVoltageChannels;          //Voltage channels
    float          pNominalVoltage;           //Analyzer nominal voltage
    EventType      pType;                     //Running event type
    uint64_t       pStart;                    //Event start timestamp
    uint64_t       pSamplesDuration;          //Event duration in samples
    float          pReference[3];             //Voltage reference for event (Stationary for RVC, nominal for others)
    float          pPeakValue[3];             //max/min voltage variation along event
    float          pChangeValue[3];           //diference voltage of event
    uint32_t       pCyclesDuration;           //Event duration in samples
  private :
    float CalcChange(const float in, const float reference) const;
  public :
    AnalyzerSoftEvent(const Analyzer::AnalyzerMode mode, const float nominalVoltage);
    ~AnalyzerSoftEvent();
    bool IsRunning(const bool stationary, const float *cycle, const float *avg) const;


};
// ---------------------------------------------------------------------------------------------------------------------