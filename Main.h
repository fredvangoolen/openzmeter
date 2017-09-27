#pragma once
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
#include <fftw3.h>

#define MAX_FRAME                     3688         //Longitud maxima de muestras de un frame (236ms)
//Definiciones relaccionadas con la adquisicion de datos desde el puerto USB ===========================================
#define ADQUISITION_USB_VID         0x0483         //Product ID
#define ADQUISITION_PID             0x7270         //Device ID
#define ADQUISITION_ENDPOINT_IN       0x81         //Endpoint de lectura de datos
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
#define EVENT_VOLTAGE               230.00        //Tension nominal
#define EVENT_FREQ                   50.00        //Frecuencia nominal
#define EVENT_RVC                     05.0        //Variacion en % para generar RVC
#define EVENT_DIP_SWELL               10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION            90.0        //Bajada en % para generar interrupcion
#define EVENT_HYSTERESIS              02.5        //Histeresis en % para cerrar evento
#define EVENT_PREINIT                  312        //Muestras antes del inicio del evento a guardar
#define EVENT_POSTINIT                 468        //Muestras tras el inicio del evento a guardar
#define EVENT_PREEND                   468        //Muestras previas al fin del evento a guardar
#define EVENT_POSTEND                  312        //Muestras tras finalizar el evento a guardar
//Otras definiciones ===================================================================================================
#define MAX_CURRENT                  12.00        //Corriente maxima
// Enumeraciones =======================================================================================================
enum Event_Type { SWELL, DIP, INTERRUPTION, RVC };
enum DB_Type    { PARAMS_10C, PARAMS_3S, PARAMS_1M, PARAMS_10M, PARAMS_1H, EVENT, PRICES };
// Estructuras de datos comunes entre los modulos ======================================================================
typedef struct {
  bool Debug;
  struct {
    char *Listen;
    char *Log;
    char *Pid;
  } Global;
  struct {
    const char *Database;
    const char *Host;
    int         Port;
    const char *User;
    const char *Password;
    int         QueueLen;
    int         MaxRows;
  } DB;
  struct {
    int   sample_rate;
    int   nominal_frequency;
    float nominal_voltage;
    float VoltageGain;
    int   VoltageOffset;
    float CurrentGain;
    int   CurrentOffset;
  } Electrical;
  struct {
    float  fixed_price;
    const char* country;
    void * country_config;
} budget_configuration;
struct {
    float cpc;
    int contracted_power;
    const char* rate_type;
    float vat;
    float discount_percentage;
    float electricity_tax_percentage;
    float meter_rating;
} budget_spain_configuration;
} Config_t;
typedef struct {
  float Real;                           //Parte real
  float Imag;                           //Parte imaginaria
} Complex_t;
typedef struct {
  float     *Input;                     //Muestras de entrada
  uint32_t   Len;                       //Longitud de la transformada
  fftwf_plan Plan;                      //Plan para realizar la transformada
  pthread_t  Thread;                    //Hilo de ejecucion
  Complex_t  Result[1];                 //Datos de salida
} FFT_t;
typedef struct {
  uint32_t Start;                       //Muestra dentro del bloque que inicia el ciclo
  uint32_t End;                         //Muestra dentro del bloque que finaliza el ciclo
} Cycle_t;
typedef struct {
  enum Event_Type Type;                 //Tipo de evento (puede cambiar durante el evento)
  uint64_t        Start;                //Timestamp de inicio
  uint64_t        Duration;             //Duracion en samples
  float           Voltage;              //Tension maxima o minima durante el evento
  float           Change;               //Porcentage de desviacion del evento
  float           Samples[EVENT_PREINIT + EVENT_POSTINIT + EVENT_PREEND + EVENT_POSTEND];
} Event_t;
typedef struct {
  float      RMS_V;                       //Valor RMS de la tension durante el bloque
  float      RMS_I;                       //Valor RMS de la intensidad durante el bloque
  float      Active_Power;                //Potencia activa (P)
  float      Apparent_Power;              //Potencia aparente (S)
  float      Power_Factor;                //Factor de potencia (cos(PHI))
  float      Fundamental_Active_Power;    //Potencia activa del primer armonico (P_1)
  float      Fundamental_Apparent_Power;  //Potencia aparente fundamental (S_1)
  float      Fundamental_Reactive_Power;  //Potencia reactiva fundamental (Q_1)
  float      Fundamental_Power_Factor;    //Fundamental power factor
  float      Fundamental_Reactive_Energy; //Energia reactiva
  float      Active_Energy;               //Energia consumida
  float      THD_V;                       //Distorsion harmonica de la tension
  float      THD_I;                       //Distorsion harmonica de la corriente
  float      VoltageHarmonics[50];        //Armonicos del canal de tension
  float      CurrentHarmonics[50];        //Armonicos del canal de corriente
  float      Frequency;                   //Frecuencia
  float      Phi;                         //Angulo de Phi
  bool       Flag;                        //Periodo marcado
} Params_t;
typedef struct Work_t {
  uint64_t  SequenceID;                 //Numero de secuencia del bloque
  uint64_t  Timestamp;                  //Timestamp de la muestra inicial
  uint32_t  Samples;                    //Cantidad de muestras que componen el bloque
  float     VoltageSamples[MAX_FRAME];  //Muestras de tension   (El array almacena hasta 236mS)
  float     CurrentSamples[MAX_FRAME];  //Muestras de corriente (El array almacena hasta 236mS) 
  Cycle_t   Cycles[20];                 //Informacion de los ciclos del bloque
  uint8_t   Cycles_Len;                 //Numero de ciclos en el bloque
} Work_t;
//Variables globales ===================================================================================================
extern Config_t Config;
//Funciones de utilidad para diversos modulos ==========================================================================
void LogMsg(const char *level, const char *msg, ...);
// Funciones exportadas del modulo de CAPTURA ==========================================================================
bool    Adquisition_Start(char *device, uint32_t bufferLen, float baseFreq);
void    Adquisition_Stop();
Work_t *Adquisition_GetWork();
void    Adquisition_SetLEDs(float voltage, float current);
void    Adquisition_SamplesCopy(float *dest, int32_t offset, uint32_t len);
// Funciones exportadas del modulo de SERVIDOR==========================================================================
bool RequestServer_Start(char *path);
void RequestServer_Stop();
void RequestServer_SetValues(uint64_t timestamp, Params_t *params, float *voltage, float *current, float *fftv, float *ffti, uint32_t lensamples, uint32_t lenfft);
// Funciones exportadas del modulo de BASE DE DATOS ====================================================================
bool DB_Start(char *connString, uint16_t bufferLen, uint32_t *lastprice);
void DB_Save(enum DB_Type type, uint32_t node, uint64_t timestamp, void *data, bool free);
void DB_Stop();
// Funciones exportadas del modulo de eventos ==========================================================================
void  Events_Init();
char *EventType_ToString(enum Event_Type type);
bool  Events_Detect(Work_t *data);
// Funciones exportadas del modulo FFT =================================================================================
FFT_t *FFT_Start(float *data, uint32_t len);
void   FFT_Wait(FFT_t *fft);
// Funciones exportadas del modulo de analisis =========================================================================
bool Analyzer_Start();
void Analyzer_Stop();
// Funciones exportadas del modulo de configuracion ====================================================================
bool Config_Init(char *filename);
// Funciones exportadas del modulo de precios ==========================================================================
bool Prices_Start(uint32_t startTimestamp);
void Prices_Stop();
