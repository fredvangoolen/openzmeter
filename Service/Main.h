#pragma once
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
//#include <fftw3.h>

//#define MAX_FRAME                     3688         //Longitud maxima de muestras de un frame (236ms)
//Definiciones relaccionadas con la adquisicion de datos desde el puerto USB ===========================================
//#define ADQUISITION_VOLT_OFFSET     348.00         //Real_Value = (Read_Value - Offset) * Gain
//#define ADQUISITION_VOLT_GAIN      0.01160        
//#define ADQUISITION_CURR_OFFSET   32768.00         //Real_Value = (Read_Value - Offset) * Gain
//#define ADQUISITION_CURR_GAIN  -0.00078125
#define ADQUISITION_SAMPLEFREQ    15625.00         //Frequencia de muestreo
//#define ADQUISITION_DEVICE  "/dev/ttyACM0"         //Puerto desde el que se hace la lectura
#define ADQUISITION_BUFFERSIZE     1048576         //Tamaño del buffer circular para captura (en grupos de muestras)
#define ADQUISITION_SAFESAMPLES  MAX_FRAME         //Numero de muestras "seguras" antes y despues del punto de lectura 
//Definiciones relaccionadas con almacenamiento de datos a la BD =======================================================
//#define DB_QUEUESIZE                  1024         //Longitud de la cola de transacciones
//#define DB_CONNECTIONSTRING "user=zmeter password=swordfish dbname=zmeter"
//Definiciones del servidor de peticiones ==============================================================================

#define REQUESTSERVER_BUFSIZE          128
#define REQUESTSERVER_PERM            0666
#define REQUESTSERVER_CLIENTS           50
//Definiciones para el modulo de proceso ===============================================================================
#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//Definiciones para deteccion de eventos ===============================================================================
//#define EVENT_VOLTAGE               230.00        //Tension nominal
//#define EVENT_FREQ                   50.00        //Frecuencia nominal
//#define EVENT_RVC                     05.0        //Variacion en % para generar RVC
//#define EVENT_DIP_SWELL               10.0        //Variacion en % para generar subida o bajada
//#define EVENT_INTERRUPTION            90.0        //Bajada en % para generar interrupcion
//#define EVENT_HYSTERESIS              02.5        //Histeresis en % para cerrar evento
//#define EVENT_PREINIT                  312        //Muestras antes del inicio del evento a guardar
//#define EVENT_POSTINIT                 468        //Muestras tras el inicio del evento a guardar
//#define EVENT_PREEND                   468        //Muestras previas al fin del evento a guardar
//#define EVENT_POSTEND                  312        //Muestras tras finalizar el evento a guardar
//Otras definiciones ===================================================================================================
#define MAX_CURRENT                  12.00        //Corriente maxima
// Enumeraciones =======================================================================================================
enum Event_Type { SWELL, DIP, INTERRUPTION, RVC };
enum DB_Type    { PARAMS_10C, PARAMS_3S, PARAMS_1M, PARAMS_10M, PARAMS_1H, EVENT, PRICES };
// Estructuras de datos comunes entre los modulos ======================================================================
typedef struct {
//  enum Event_Type Type;                 //Tipo de evento (puede cambiar durante el evento)
  uint64_t        Start;                //Timestamp de inicio
//  uint64_t        Duration;             //Duracion en samples
//  float           Voltage;              //Tension maxima o minima durante el evento
//  float           Change;               //Porcentage de desviacion del evento
////  float           Samples[EVENT_PREINIT + EVENT_POSTINIT + EVENT_PREEND + EVENT_POSTEND];
} Event_t;
