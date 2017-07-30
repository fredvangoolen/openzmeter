/**
 * @defgroup GlobalVars GlobalVars
 * @brief Contains all global variables of the application.
 */

/**
 *   @file globalVars.h
 *   @ingroup GlobalVars
 *   @brief Contains all global variables of the application.
 *
 *   @author Pedro Sánchez Alguacil
 */

#ifndef _GLOBALVARS_H
#define _GLOBALVARS_H

#include <stdio.h>
#include "types.h"
#include <poll.h>
//CONFIG STRUCT
//extern configuration CONFIG;


//THREADS
//extern int NUM_THREADS;

//LOG
/**
 * @brief Path to log file.
 */
//extern char* LOG_PATH;
/**
 * @brief Log file.
 */
//extern int VERBOSE_MODE;
/**
 * @brief Sample rate (in Hz)
 */
//extern int SAMPLE_RATE;
/**
 * @brief Interval of Time of samples we obtain in each reading (in miliseconds)
 */
//extern int PROCESS_INTERVAL; //Intervalo de procesamiento
/**
 * @brief Number of samples in a processing interval.
 */
//extern int SAMPLES_PER_INTERVAL; //Número de muestras/fase en cada lectura.
/**
 * @brief Precission in bytes of the samples.
 */
//extern int SAMPLE_PRECISSION; //Precisión de las muestras.
/**
 * @brief Length of a half cycle in samples.
 */
//extern int CYCLE_LENGTH; //Longitud de medio ciclo en muestras.
/**
 * @brief If cycle RMS must be calculated in each half cycle.
 **/
//extern int RMS_HALF_CYCLE;
/**
 * @brief Number of cycle intervals in a process interval.
 **/
//extern int NUM_CYCLES;
/**
 * @brief If harmonic groups must be calculated.
 **/
//extern int CALCULATE_HG;
/**
 * @brief If harmonic subgroups must be calculated.
 **/
//extern int CALCULATE_SG;
/**
 * @brief If interharmonic groups must be calculated.
 **/
//extern int CALCULATE_IG;
/**
 * @brief If interharmonic subgroups must be calculated.
 **/
//extern int CALCULATE_IS;

//GENERAL
/**
 * @brief Nominal voltage value.
 **/
//extern int NOMINAL_VOLTAGE;

//EVENT_DETECTION
/**
 * @brief Total Armonic Distortion limit.
 **/
//extern float THD_LIMIT;
/**
 * @brief Voltage upper limit.
 **/
//extern int VOLTAGE_UPPER_LIMIT;
/**
 * @brief Voltage lower limit.
 **/
//extern int VOLTAGE_LOWER_LIMIT;

//ELECTRICAL
/**
 * @brief Fundamental frecuency (in Hz).
 */
//extern int FUNDAMENTAL_FREQ;
/**
 * @brief Top frequency (to allow capturing 10 cycles when frequency changes up to FUNDAMENTAL_FREQ + TOP_FREQ)
 */
//extern int TOP_FREQ;
/**
 * @brief Maximum number of harmonics to consider.
 */
//extern int MAX_HARMONICS_LIMIT; //Número máximo de armónicos a guardar
/**
 * @brief Interharmonics capture tolerance (related to coefficient module).
 **/
//extern float INTERHARM_CAPTURE_TOLERANCE;
/**
 * @brief Coefficients storage time (in seconds)
 */
//extern long COEF_STORAGE_TIME;
/**
 * @brief Cycle or Half Cycle storage time (in seconds)
 */
//extern long CYCLERMS_STORAGE_TIME;
/**
 * @brief If delete old coefficients data.
 */
//extern int DELETE_COEF_OLD_DATA;
/**
 * @brief Agregation intervals.
 */
//extern struct agregationInterval* AGREGATION_INTERVALS;
/**
 * @brief Number of agregation intervals.
 */
//extern int NUM_AGREGATION_INTERVALS;


//extern uint32_t TIME_STEP_PER_SAMPLE;


//extern struct rms_rvc RMS_RVC;

//extern struct event_list EVENT_LIST;

//extern struct processing_work_list WORK_LIST;

//extern struct frequency_interval_list FREQUENCY_LIST;

//extern struct oscilloscope OSCILLOSCOPE;

//extern struct pollfd SerialIO;
//extern struct PRU_IO;

//extern pthread_mutex_t REQUEST_SERVER_LOCK;

//extern int VOLTAGE_SCALE;

//extern int CURRENT_SCALE;

//extern int IS_PRU_ADC_ENABLED;

#endif /* _GLOBALVARS_H */