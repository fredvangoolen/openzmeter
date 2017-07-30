/**
 *   @file types.h
 *   @brief Data types used in the application
 *
 *   @author Pedro Sánchez Alguacil
 */

#ifndef _TYPES_H
#define	_TYPES_H

#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include "Main.h"
typedef struct {
    const char* database;
    const char* host;
    int port;
    const char* user;
    const char* pass;
} database_configuration;

typedef struct {
    int sample_rate;
    int nominal_frequency;
    float nominal_voltage;
    float voltage_scale;
    int voltage_offset;
    float current_scale;
    int current_offset;
    int num_stored_harmonics;
} electrical_configuration;

typedef struct {
    float fixed_price;
    const char* country;
    void * country_config;
} budget_configuration;

typedef struct {
    float cpc;
    int contracted_power;
    const char* rate_type;
    float vat;
    float discount_percentage;
    float electricity_tax_percentage;
    float meter_rating;
} budget_spain_configuration;

typedef struct {
    database_configuration database;
    electrical_configuration electrical;
    budget_configuration budget;
} configuration;

/**
 * @brief Struct which represents a complex number.
 */
//struct oscilloscope {
//    float* voltageSamples;
//    int numVoltageSamples;
//    float* currentSamples;
//    int numCurrentSamples;
//    int64_t timestamp;
//    /**
//     * @brief Mutex lock for read/write
//     **/
//    pthread_mutex_t lock;
//};

/******************************
 * Energetrics legacy USED
 *****************************/

/**
 * @brief Struct which represents a complex number.
 */
typedef struct n_complex {
    /**
     * @brief Real component.
     */
    float r;
    /**
     * @brief Imaginary component
     */
    float i;
} complex;



struct interest_parameters {
    float f_i;
    float RMS_V;
    float RMS_I;
    float Active_Power;
    
    float Energy;
    
    float THDu;
    float THDi;

    
    
    float Fundamental_Active_Power;
    float Q;
    float Q_1;
    float S;
    float S_1;
    float S_N;
    float N;
    float D;
    float pf_1;
    float pf;

    complex* voltage_harmonics;
    complex* current_harmonics;
    float* voltage_harmonics_RMS;
    float* current_harmonics_RMS;
};

struct event_list {
    /**
     * @brief Head pending events (newer)
     **/
    struct Event_t* head;
    /**
     * @brief Mutex lock for concurrent manipulation of the list.
     **/
    pthread_mutex_t lock;
};

/**
 * @brief Running event part
 **/
struct running_event_part {
//    enum Event_Type type;
    int64_t startPart;
    int64_t endPart;
    float U_max;
    float U_ss;
    /**
     * @brief Next pending work (older)
     */
    struct running_event_part* next;
    /**
     * @brief Previous pending work (newer)
     */
    struct running_event_part* prev;
};

/**
 * @brief Running event part
 **/


/**
 * @brief RMS-RVC
 **/
//struct rms_rvc {
//    float PrevRMS[100];
//    int   PrevRMS_Len;
//    int   PrevRMSWrite;
//
////    float rms_i;
//    float rms_iBeforeEvent;
//    int ss;
////    int state;
//    
////    float firstRMS;
////    int64_t processedTime;
//    long stableTime;
//    long metTime;
//    struct Event_t* runningEvents;
//    int numRunningEvents;
//    int allowedChanges;
//};
/**
 * @brief Linked list for same type intervals.
 **/
struct processing_work_list {
    /**
     * @brief Head pending work (newer)
     **/
    Work_t* head;
    /**
     * @brief Mutex lock for concurrent manipulation of the list.
     **/
    pthread_mutex_t lock;
};


/**
 * @brief Struct which...
 **/
struct frequency_interval {
    /**
     * @brief End of the interval timestamp.
     **/
    long endTime;
    /**
     * @brief Start of the interval timestamp.
    long startTime;*/
    /**
     * @brief Number of cycles
     **/
    long cycleCount;
    /**
     * @brief Processed time (usec)
     **/
    long processedTime;
    /**
     * @brief Mutex lock for concurrent manipulation of the struct.
     **/
    pthread_mutex_t lock;
    /**
     * @brief Next active computing interval.
     **/
    struct frequency_interval* next;
    /**
     * @brief Next active computing interval.
     **/
    struct frequency_interval* prev;
};

/**
 * @brief Linked list for same type intervals.
 **/
struct frequency_interval_list {
    /**
     * @brief First interval.
     **/
    struct frequency_interval* head;
    /**
     * @brief Mutex lock for concurrent manipulation of the list.
     **/
    pthread_mutex_t lock;
};

/******************************
 * Energetrics legacy
 *****************************/

enum system_type {
    /**
     * @brief System is single-phase.
     */
    SINGLE_PHASE = 1,
    /**
     * @brief System is three-phase.
     */
    THREE_PHASE = 3
};

enum acquisition_order {
    ALL_CURRENT_FIRST,
    ALL_VOLTAGE_FIRST,
    ALTERNATE_CURRENT_FIRST,
    ALTERNATE_VOLTAGE_FIRST
};

enum phase_label {
    SINGLE,
    R = 1,
    S = 2,
    T = 3,
    N = 4
};

enum property_label {
    V = 0,
    I = 1
};

enum frequencyGroups {
    /**
     * @brief Harmonics only.
     */
    H = 0,
    /**
     * @brief Harmonic group.
     */
    HG = 1,
    /**
     * @brief Harmonic subgroup.
     */
    SG = 2,
    /**
     * @brief Interharmonic group.
     */
    IG = 3,
    /**
     * @brief Interharmonic subgroup.
     */
    IS = 4
};

enum threshold_status {
    CORRECT = 0,
    OVER = 1,
    UNDER = -1,
};

/**
 * @brief Struct which contains the global parameters essential values during a running period of the system.
 */
struct system_period {
    /**
     * @brief Sample rate.
     */
    int sample_rate;
    /**
     * @brief Process interval.
     */
    int process_interval;
    /**
     * @brief Fundamental frequency.
     */
    int fundamental_freq;
    /**
     * @brief If must calculate half cycle RMS or not.
     */
    int half_cycle_rms;
    /**
     * @brief Interharm tolerance.
     */
    float interharm_tolerance;
    /**
     * @brief Agregation intervals
     */
    char* agreg_intervals;
};

typedef struct t_interharm {
    int index;
    complex coef;
} interharmonic;

/**
 * @brief Struct which defines a plugin parameter.
 */
typedef struct t_param {
    /**
     * @brief Parameter name.
     */
    char* name;
    /**
     * @brief Parameter data type.
     */
    char* type;
    /**
     * @brief Minimum value (number type) or length (string type) of the parameter.
     */
    int min;
    /**
     * @brief Maximum value (number type) or length (string type) of the parameter.
     */
    int max;
    /**
     * @brief Possible options values (only filled for option type)
     */
    char** options;
    /**
     * @brief Number of possible options.
     */
    int numOptions;
    /**
     * @brief Value of the parameter
     **/
    char* value;
} param_def;

/**
 * @brief Struct which stores a set of samples and its timestamp.
 */
struct dataPacket {
    /**
     * @brief Seconds (UNIX time).
     */
    long seconds;
    /**
     * @brief Milliseconds.
     */
    short milliseconds;
    /**
     * @brief Receptor whose samples are going to be processed.
     */
    struct receptor* recept;
    /**
     * @brief Samples obtained from acquisition module.
     */
    unsigned short* data;
    /**
     * @brief Last set of samples obtained from acquisition module.
     */
    unsigned short* lastData;
};

struct otherSet {
    float Vt;
    float It;
    float Pt;
    float P1;
    float Qt;
    float Q1;
    float St;
    float S1;
    float cos_phiT;
    float cos_phi1;
    float THDv;
    float THDi;
};

struct agregationInterval {
    long interval;
    long rowLimit;
    long numPreviousCount;
};

/**
 * @brief Struct which stores the values of the electrical parameters of interest.
 **/
struct computing_storage {
    /**
     * @brief Energy parameters set array ([phase])
     **/
    struct otherSet* energySets;
    /**
     * @brief Total current RMS for neutral phase
     **/
    float It_N;
    /**
     * @brief Total current RMS for neutral phase ([phase][frequency_group_type][property])
     **/
    float**** freqGroupRMS;
};

/**
 * @brief Struct which stores the electrical parameters of interest of the
 * receptor related to an active interval (not completed).
 **/
struct computing_interval {
    /**
     * @brief End of the interval timestamp.
     **/
    long end_time;
    /**
     * @brief Number of additions made.
     **/
    long count;
    /**
     * @brief Mutex lock for concurrent manipulation of the struct.
     **/
    pthread_mutex_t lock;
    /**
     * Values of electrical parameters of interest.
     **/
    struct computing_storage values;
    /**
     * @brief Next active computing interval.
     **/
    struct computing_interval* next;
};

/**
 * @brief Linked list for same type intervals.
 **/
struct computing_interval_list {
    /**
     * @brief First interval.
     **/
    struct computing_interval* head;
    /**
     * @brief Mutex lock for concurrent manipulation of the list.
     **/
    pthread_mutex_t lock;
};

/**
 * @brief Detection states per phase.
 **/
struct detection_states {
    /**
     * @brief Voltage status.
     **/
    enum threshold_status voltage_status;
    /**
     * @brief Time when last voltage event occurred.
     **/
    long voltage_status_time;
    /**
     * @brief Milliseconds when last voltage event occurred.
     **/
    short voltage_status_ms;
    /**
     * @brief THD_v status.
     **/
    enum threshold_status thdv_status;
    /**
     * @brief Time when last THD_v event occurred.
     **/
    long thdv_status_time;
    /**
     * @brief Milliseconds when last THD_v event occurred.
     **/
    short thdv_status_ms;
    /**
     * @brief Mutex lock for concurrent manipulation of the struct.
     **/
    pthread_mutex_t lock;
};

struct receptor {
    /**
     * @brief Receptor index in the database.
     */
    int index;
    /**
     * @brief Receptor label.
     */
    char* label;
    /**
     * @brief Receptor description.
     */
    char* description;
    /**
     * @brief System type (One-phase or Three-phase).
     */
    enum system_type type;
    /**
     * @brief If receptor has neutral phase.
     */
    int neutral;
    /**
     * @brief Pointer to aquisition file.
     */
    FILE* acquisitionFile;
    /**
     * @brief Path to aquisition file.
     */
    char* acquisitionPath;
    /**
     * @brief Phase order.
     */
    enum phase_label* phaseOrder;
    /**
     * @brief Voltage indexes to get the samples of each phase.
     */
    int* voltage_indexes;
    /**
     * @brief Current indexes to get the samples of each phase.
     */
    int* current_indexes;
    /**
     * @brief Table indexes for the active plugins which operate with the receptor.
     */
    int* table_plugin_indexes;
    /**
     * @brief Row limits for database round robin mechanism.
     */
    int64_t* table_plugin_row_limits;
    /**
     * @brief Acquisition order.
     */
    enum acquisition_order order;
    /**
     * @brief Voltage scale.
     */
    int voltageScale;
    /**
     * @brief Current scale.
     */
    int currentScale;
    /**
     * @brief Nominal voltage.
     */
    int nominalVoltage;
    /**
     * @brief List to store and manage agregation intervals and other measures.
     **/
    struct computing_interval_list* interval_sets;
    /**
     * @brief Struct to store (per phase) the voltage, current and THDv status (event detection)
     **/
    struct detection_states* detection_st;
    /**
     * @brief If receptor has changed since last system intialization.
     */
    int isNew;
    /**
     * @brief Pointer to next receptor.
     */
    struct receptor* next;
};

struct receptor_counter {
    timer_t timerId;
    struct itimerspec timeParameters;
    struct sigevent readHandler;
    union sigval handlerAttr;
    long sec;
    short millisec;
    struct timeval tv;
    struct receptor* associatedReceptor;
    struct receptor_counter* next;
    unsigned short* lastData;
    int isFirstDataPacket;
};

/*
 * Estructura para almacenar los datos y funciones del plugin de análisis
 **/

/**
 * @brief Struct that stores pointers on a plugin for conversion to frequency domain.
 */
struct plugin_dom_frec {
    /**
     * @brief Plugin key name.
     */
    char* key;
    /**
     * @brief Pointer to initialize function.
     */
    int (*initialize)(int, char**);
    /**
     * @brief Pointer to execute function.
     */
    int (*execute)(float*, complex***);
    /**
     * @brief Pointer to unload function.
     */
    int (*unload)();
    /**
     * @brief Pointer to getHarmonicsHop function.
     */
    int (*getHarmonicsHop)(int);
    /**
     * @brief Pointer to getFirstHarmonicPosition function.
     */
    int (*getFirstHarmonicPosition)(int);
    /**
     * @brief Pointer to getFrecuency function.
     */
    int (*getFrecuency)(int, int);
    /**
     * @brief Pointer to out_nframes variable.
     */
    //int *out_nframes;
    /**
     * @brief Pointer to frame_length variable.
     */
//    int *frame_length;
    /**
     * @brief If electrical parameters of interest must be calculated using plugin results.
     **/
    int hasParamCalculation;
    /**
     * @brief If it's a new plugin or its parameters have changed.
     **/
    int isNew;
    /**
     * @brief Plugin parameters.
     **/
    param_def* params;
    /**
     * @brief Number of plugin parameters.
     **/
    int numParams;
    /**
     * @brief Pointer to next plugin.
     */
    struct plugin_dom_frec* next;
};

#endif	/* _TYPES_H */