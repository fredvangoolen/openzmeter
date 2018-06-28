/* ================================== zMeter ===========================================================================
 * File:     Events.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Database.h"
//#include "Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
#define EVENT_RVC                      5.0        //Variacion en % para generar RVC
#define EVENT_RVC_HYSTERESIS           3.0        //Variacion en % para finalizar RVC
#define EVENT_DIP_SWELL               10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION            90.0        //Bajada en % para generar interrupcion
#define EVENT_HYSTERESIS              06.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_ENVELOPETIME             0.1        //Tiempo en segundos que se guarda antes y despues de un evento
//----------------------------------------------------------------------------------------------------------------------
class Events : public Database {
  private :
    enum EventType  { EVT_NONE, EVT_SWELL, EVT_DIP, EVT_INTERRUPTION, EVT_RVC, EVT_GLITCH };
    typedef struct {
      float    *RMS;
      uint64_t  Timestamp;
      uint32_t  Len;
    } EventCycle_t;
    typedef struct {
      enum EventType Type;                 //Tipo de evento (puede cambiar durante el evento)
      uint64_t        Start;                //Timestamp de inicio
      uint64_t        Duration;             //Duracion en samples
      float           Voltage;              //Tension maxima o minima durante el evento
      float           Change;               //Porcentage de desviacion del evento
    } Event_t;
  private :
    bool            pTriphase;    
    float           pSampleRate;
    EventCycle_t    pPrevCycles[100];     //Informacion de ciclos anteriores para calculo de eventos
    uint8_t         pPrevCyclesLen;       //Tamaño de buffer de ciclos
    uint8_t         pPrevCyclesWrite;     //Posicion actual del buffer de ciclos
    float          *pStationaryRMS;       //Valores de tension estacionarios
    bool            pPrevStationary;      //Almacena el estado del flag de regimen estacionario
    Event_t         pRunningEvent;        //Evento en curso
  private :
    void  SaveEvent(uint8_t cycle, bool end);
    float PercentChange(float in, float reference);
  public :
//    void EventsDetect(Analyzer::Work_t *work);
    Events();
    ~Events();      
};