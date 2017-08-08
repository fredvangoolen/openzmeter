/* ================================== zMeter ===========================================================================
 * File:     Events.c
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/param.h>
#include "Main.h"
//#include "globalVars.h"
// Tipos de datos ======================================================================================================
typedef struct {
  float     PrevRMS[100];
  int       PrevRMS_Len;
  int       PrevRMSWrite;
  uint8_t   StableCycles;
  uint32_t  StableLen;
  Event_t  *RunningEvent;
} RVCEventDetector_t;
// Variables del modulo ================================================================================================
RVCEventDetector_t Events;

// Funciones del modulo ================================================================================================
float PercentChange(float in, float reference) {
  return (in / reference) * 100;
}
char *EventType_ToString(enum Event_Type type) {
  switch(type) {
    case RVC:
      return "RVC";
    case DIP:
      return "DIP";
    case SWELL:
      return "SWELL";
    case INTERRUPTION:
      return "INTERRUPTION";
    default:
      return "UNKNOWN";
  }
}
void Events_Init() {
  Events.PrevRMSWrite = 0;
  Events.PrevRMS_Len  = 0;
  Events.RunningEvent = NULL;
  Events.StableCycles = 0;
  Events.StableLen    = 0;
}
bool Events_Detect(Work_t *work) {
  for(int i = 0; i < work->Cycles_Len; i++) {
    float CycleRMS = 0;
    uint32_t CycleLen = work->Cycles[i].End - work->Cycles[i].Start;
    for(int i = work->Cycles->Start; i < work->Cycles->End; i++) CycleRMS += pow(work->VoltageSamples[i], 2);
    CycleRMS = sqrt(CycleRMS / CycleLen);
    Events.PrevRMS[Events.PrevRMSWrite] = CycleRMS;
    Events.PrevRMSWrite = (Events.PrevRMSWrite + 1) % 100;
    if(Events.PrevRMS_Len < 100) Events.PrevRMS_Len++;
    float RMS_Avg = 0.0;
    for(int i = 0; i < Events.PrevRMS_Len; i++) RMS_Avg += Events.PrevRMS[i];
    RMS_Avg = RMS_Avg / Events.PrevRMS_Len;
    if((Events.StableCycles == 100) && (Events.RunningEvent == NULL) && (fabs(100.0 - PercentChange(CycleRMS, RMS_Avg)) > EVENT_RVC)) {
      Events.RunningEvent = (Event_t*)malloc(sizeof(Event_t));
      Events.RunningEvent->Type    = RVC;
      Events.RunningEvent->Voltage = EVENT_VOLTAGE;
    }    
    if(abs(100.0 - PercentChange(CycleRMS, EVENT_VOLTAGE)) > EVENT_DIP_SWELL) {
      if(Events.RunningEvent == NULL) {
        Events.RunningEvent = (Event_t*)malloc(sizeof(Event_t));
        Events.RunningEvent->Type = (CycleRMS > EVENT_VOLTAGE) ? SWELL : DIP;
        Events.RunningEvent->Voltage = EVENT_VOLTAGE;
      }
      if(Events.RunningEvent->Type != INTERRUPTION) Events.RunningEvent->Type  = (CycleRMS > EVENT_VOLTAGE) ? SWELL : DIP;
    }
    if(PercentChange(CycleRMS, EVENT_VOLTAGE) < (100.0 - EVENT_INTERRUPTION)) Events.RunningEvent->Type = INTERRUPTION;
    if((PercentChange(CycleRMS, EVENT_VOLTAGE) < (100.0 - EVENT_INTERRUPTION)) || (CycleLen > (ADQUISITION_SAMPLEFREQ / (EVENT_FREQ * 1.7)))) {
      if(Events.RunningEvent == NULL) {
        Events.RunningEvent = (Event_t*)malloc(sizeof(Event_t));
        Events.RunningEvent->Voltage = EVENT_VOLTAGE;
      }
      Events.RunningEvent->Type = EVENT_INTERRUPTION;
    }
    if(Events.RunningEvent != NULL) {
      if(Events.RunningEvent->Voltage == EVENT_VOLTAGE) {
        Events.RunningEvent->Start    = work->Timestamp + (work->Cycles[i].Start * 1000.0) / ADQUISITION_SAMPLEFREQ;
        Events.RunningEvent->Duration = 0;
        Events.RunningEvent->Change   = 100;
        memset(Events.RunningEvent->Samples, 0, sizeof(Events.RunningEvent->Samples));
        Adquisition_SamplesCopy(Events.RunningEvent->Samples, work->Samples - work->Cycles[i].Start + EVENT_PREINIT, EVENT_PREINIT + EVENT_POSTINIT);
        Events.RunningEvent->Samples[EVENT_PREINIT + EVENT_POSTINIT - 1] = NAN;
      }
      float Change = PercentChange(CycleRMS, RMS_Avg); 
      Events.RunningEvent->Duration += work->Cycles[i].End - work->Cycles[i].Start;
      if(fabs(100.0 - Change) > fabs(100.0 - Events.RunningEvent->Change)) {
        Events.RunningEvent->Change  = Change;
        Events.RunningEvent->Voltage = CycleRMS;
        DB_Save(EVENT, Events.RunningEvent, false);
      }
    }
    if((Events.RunningEvent != NULL) && (Events.RunningEvent->Type != RVC) && (fabs(100.0 - PercentChange(CycleRMS, EVENT_VOLTAGE)) < EVENT_HYSTERESIS)) {
      if(Events.RunningEvent->Duration > (EVENT_POSTINIT + EVENT_PREEND)) {
        Adquisition_SamplesCopy(&Events.RunningEvent->Samples[EVENT_PREINIT + EVENT_POSTINIT], work->Samples - work->Cycles[i].End + EVENT_PREEND, EVENT_PREEND + EVENT_POSTEND);
      } else {
        //Increment = Samples - POSTINIT - POSTEND;
        Adquisition_SamplesCopy(&Events.RunningEvent->Samples[EVENT_PREINIT + Events.RunningEvent->Duration / 2], work->Samples - work->Cycles[i].End + Events.RunningEvent->Duration / 2, Events.RunningEvent->Duration / 2 + EVENT_POSTEND);
      }
      DB_Save(EVENT, Events.RunningEvent, true);
      Events.RunningEvent = NULL;
    }
    if(fabs(100.0 - PercentChange(CycleRMS, RMS_Avg)) > ((Events.RunningEvent == NULL) ? EVENT_RVC : EVENT_HYSTERESIS)) {
      Events.StableLen = 0;
      Events.StableCycles = 0;
    } else {
      Events.StableLen += work->Cycles[i].End - work->Cycles[i].Start;
      if(Events.StableCycles < 100) {
        Events.StableCycles++;
      } else {
        if((Events.RunningEvent != NULL) && (Events.RunningEvent->Type == RVC)) {
          Events.RunningEvent->Duration = Events.RunningEvent->Duration - Events.StableLen;
          if(Events.RunningEvent->Duration > (EVENT_POSTINIT + EVENT_PREEND)) {
            Adquisition_SamplesCopy(&Events.RunningEvent->Samples[EVENT_PREINIT + EVENT_POSTINIT], work->Samples - work->Cycles[i].End + EVENT_PREEND, EVENT_PREEND + EVENT_POSTEND);
          } else {
            //Increment = Samples - POSTINIT - POSTEND;
            Adquisition_SamplesCopy(&Events.RunningEvent->Samples[EVENT_PREINIT + Events.RunningEvent->Duration / 2], work->Samples - work->Cycles[i].End + Events.RunningEvent->Duration / 2, Events.RunningEvent->Duration / 2 + EVENT_POSTEND);
          }
          DB_Save(EVENT, Events.RunningEvent, true);
          Events.RunningEvent = NULL;
        }
      }
    }
  }
  return (Events.RunningEvent != NULL);
}