#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include "types.h"
#include "constants.h"
#include "globalVars.h"
#include "calculation_module.h"
#include "processing.h"
#include "debug.h"
#include "thpool.h"
#include "leds.h"
#include "Main.h"
//#include "watchdog.h"
#include <pthread.h>
#include <errno.h>
#include "Adquisition.h"
#include "RequestServer.h"

void LogMsg(const char *level, const char *msg, ...);
/* Handles SIGINT */
void exithandler(int x) {
  printf("sigint caughte\n");
  fflush(stdout);
//  if(IS_PRU_ADC_ENABLED == 1) {
    //        pruio_destroy(PRU_IO);
//    printf("PRUIO DESTROYED\n");
//    fflush(stdout);
//  }

  printf("print_trace: got signal %d\n", x);

  void *array[32]; /* Array to store backtrace symbols */
  size_t size; /* To store the exact no of values stored */
  char **strings; /* To store functions from the backtrace list in ARRAY */
  size_t nCnt;

  size = backtrace(array, 32);

  strings = backtrace_symbols(array, size);

  /* prints each string of function names of trace*/
  for(nCnt = 0; nCnt < size; nCnt++)
    fprintf(stderr, "%s\n", strings[nCnt]);

  fflush(stderr);
  exit(-1);
}

/* Handles SIGSEGV */
void segfaulthandler(int x) {
  printf("segfault caught\n");
  fflush(stdout);
//  if(IS_PRU_ADC_ENABLED == 1) {
//    if(Buffer.SerialIO.fd != -1) close(Buffer.SerialIO.fd);
    //        pruio_destroy(PRU_IO);
//    printf("PRUIO DESTROYED\n");
//    fflush(stdout);
//  }

  printf("print_trace: got signal %d\n", x);

  void *array[32]; /* Array to store backtrace symbols */
  size_t size; /* To store the exact no of values stored */
  char **strings; /* To store functions from the backtrace list in ARRAY */
  size_t nCnt;

  size = backtrace(array, 32);

  strings = backtrace_symbols(array, size);

  /* prints each string of function names of trace*/
  for(nCnt = 0; nCnt < size; nCnt++)
    fprintf(stderr, "%s\n", strings[nCnt]);

  exit(-1);
}

//int sign(float x) {
//  return(x > 0) - (x < 0);
//}
/*
 * Obtención de 10 ciclos de tensión y conversión de short a float (d) para adaptarlo a librería FFTW.
 **/
//int zeroCrossing(float *in, struct cycle* cycle_indexes, int* readCycles, int maxCycles, int offset, int maxSamples, int* firstSampleIsZero, int* lastCycleLength) {
//  int i;
//  int cycleIndex = 0;
//  int outLength = 0;
//  int numCycles;
//  int zeroCrossings[maxSamples / 2];
//  int numZeroCrossings = 0;
//  float tempOut[maxSamples];
  /**
   * Convert to float and get zero crossings.
   */
//  if(*firstSampleIsZero) {//Set first zero crossing at index 0.
//    zeroCrossings[numZeroCrossings] = 0;
//    numZeroCrossings++;
//  }
//  for(i = 0; i < maxSamples /*&& numZeroCrossings < maxCycles + 1*/; i++) {
//    if(in[i + offset] < 200) {//Correction to scaled zero if sample ~=0
//      tempOut[i] = 0;
//    } else {
//      tempOut[i] = (((float) in[i + offset] / (float) MAX2E12)*2 - 1);
//      tempOut[i] = (((float)(in[i + offset] & 0x0FFF) - 877.54) / 25.0);
//      ((Buffer[i * 2 + 0] & 0x0FFF) - 877.54) * 32;
//    }
//    if(i > 0) {
      //printf("%04d | %f => %f \n", i, tempOut[i - 1], tempOut[i]);
      //fflush(stdout);
//      if(sign(tempOut[i - 1]) < 0 && sign(tempOut[i - 1]) < sign(tempOut[i])) {//Passing by zero (from negative to zero or positive)
        //printf("PASO POR CERO! \n");
        //fflush(stdout);
        //if (numPassingByZero % 2 == 0) {//Cycle completion
        //printf("CICLO %d! \n", cycleIndex + 1);
        //fflush(stdout);
//        zeroCrossings[numZeroCrossings] = i;
//        numZeroCrossings++;
        //}
//      }
//    }
//  }
//  int isInterruption = 0;
  /**
   * Analyze zero crossings to determine cycles.
   */
//  numCycles = 0;
//  if(numZeroCrossings > 1) {
//    int j = 0;
//    int cycleLength = *lastCycleLength;
//    if(cycleLength == -1) {
//      cycleLength = CONFIG.electrical.sample_rate / CONFIG.electrical.nominal_frequency;
//    }
//    int groupGap = 3000 / TIME_STEP_PER_SAMPLE; //36 ms in samples
//    int cycleStartIndex = -1;
//    int zcInGroup[1000];
//    int numZcInGroup = 0;
//    int groupOpen = 0;
//    if(!*firstSampleIsZero) {
//      j = zeroCrossings[0]; //Set index to first zero crossing.
//    }
//    for(i = 0; i <= numZeroCrossings; i++) {
//      if(i == numZeroCrossings || zeroCrossings[i] < j - groupGap || zeroCrossings[i] > j + groupGap) {//It is not included in group or we are in the last step
//        if(groupOpen) {
//          int medianZcInGroup = -1;
//          if(numZcInGroup == 0) {//Zero crossing not found
//            cycleStartIndex = -1; //Drop the start of the cycle.
//            isInterruption = 1;
//          } else if(numZcInGroup % 2 == 0) {
//            int median1 = zcInGroup[numZcInGroup / 2]; //Median value 1
//            int median2 = zcInGroup[numZcInGroup / 2 - 1]; //Median value 2
//            int distance1 = abs(j - zeroCrossings[median1]); //Distance from median 1
//            int distance2 = abs(j - zeroCrossings[median2]); //Distance from median 2
            //Take the nearest median to the theoric position.
//            if(distance1 <= distance2) {
//              medianZcInGroup = median1;
//            } else {
//              medianZcInGroup = median2;
//            }
//          } else {
//            medianZcInGroup = zcInGroup[numZcInGroup / 2]; //Median
//          }

//          if(cycleStartIndex != -1) {//A cycle has begun.
//            cycle_indexes[numCycles].index = numCycles;
//            cycle_indexes[numCycles].start = cycleStartIndex;
//            cycle_indexes[numCycles].end = zeroCrossings[medianZcInGroup];
//            cycleLength = zeroCrossings[medianZcInGroup] - cycleStartIndex;
//            numCycles++;
//            if(numCycles >= maxCycles) {
//              break;
//            }
//          }
//          cycleStartIndex = zeroCrossings[medianZcInGroup]; //Beginning of a cycle

//          j = zeroCrossings[medianZcInGroup] + cycleLength; //Position in the chosen zero crossing + cycle length
          //Close previous group
//          numZcInGroup = 0;
//          groupOpen = 0;
//        }
//      }
//      if(i < numZeroCrossings && zeroCrossings[i] >= j - groupGap && zeroCrossings[i] <= j + groupGap) {//It is included in group.
//        zcInGroup[numZcInGroup] = i;
//        numZcInGroup++;
//        groupOpen = 1;
//      }
//    }

//  }




//  if(numCycles >= 10 && isInterruption == 0) {//10 cycles with no interruption
//    outLength = cycle_indexes[9].end - cycle_indexes[0].start;
//    *firstSampleIsZero = 1;
//  } else {//Less than 10 cycles or interruption
//    outLength = maxSamples;
//    *firstSampleIsZero = 0;
//  }
//  int startPosition = 0;
//  if(numCycles > 0) {
//    if(numCycles > 10) {
//      numCycles = 10; //We only want ten cycles.
//    }
//    *lastCycleLength = cycle_indexes[numCycles - 1].end - cycle_indexes[numCycles - 1].start;
//    startPosition = cycle_indexes[0].start;
//  } else {
//    *lastCycleLength = -1;
//  }
//  *readCycles = numCycles;
//  *out = NULL;
//  *out = calloc(outLength, sizeof(float));
//  memcpy(*out, tempOut + startPosition, outLength * sizeof(float));
//  debug_print_2("\nReservando salida: %d para puntero %p\n", outLength, out);
//  debug_print_2("\nPost reserva: %f\n", *out[0]);
  //fflush(stdout);
//  return outLength;
//}

//void unshort2float(unsigned short *x, float *d, int offset, int n) {
//  int i;
//  for(i = 0; i < n; i++) {
//    if(x[i + offset] < 200) {//Correction to scaled zero if sample ~=0
//      d[i] = 0;
//    } else {
//      d[i] = (((float) x[i + offset] / (float) MAX2E12)*2 - 1);
//    }
//  }
//}

int64_t startDaemon() {
  signal(SIGINT, exithandler);
  signal(SIGKILL, exithandler);
  signal(SIGTERM, exithandler);
  signal(SIGSEGV, segfaulthandler);

//uint64_t sequenceId = 0;
  int      numSignals = 2;
  Samples_Len = CONFIG.electrical.sample_rate;
  
  
  int32_t sampleRateNanosec = 1e9 / CONFIG.electrical.sample_rate;
//  int workingSamplesPerSignal = CONFIG.electrical.sample_rate; //Working size in samples: 10 signal cycles
//  uint32_t packageSize = workingSamplesPerSignal * numSignals; //Working size in samples of all signals
  int numLastSamplesToSave = 2 * CONFIG.electrical.sample_rate / CONFIG.electrical.nominal_frequency; //2 cycles.

//  TIME_STEP_PER_SAMPLE = sampleRateNanosec / 1000; //Conversion to usec
  uint64_t processedSamples = 0;

  threadpool thpool = thpool_init(NUM_THREADS);

//  Samples_Voltage = malloc(Samples_Len * sizeof(float));
//  Samples_Current = malloc(Samples_Len * sizeof(float));

//  uint16_t **samples = calloc(sizeof(uint16_t*), numSignals);
//  remainingSamples = calloc(sizeof(uint16_t*), numSignals);
//  for(int i = 0; i < numSignals; i++) samples[i] = calloc(workingSamplesPerSignal * 2, sizeof(uint16_t));

//  long numRemainingSamples = 0;

  WORK_LIST.head = NULL;
  pthread_mutex_init(&WORK_LIST.lock, NULL); //Intialize work list lock.

  FREQUENCY_LIST.head = NULL;
  pthread_mutex_init(&FREQUENCY_LIST.lock, NULL); //Intialize frequency list lock.

  RMS_RVC.processedTime = -1;
  RMS_RVC.numRmsUsed = 0;
  RMS_RVC.rms_i = -1;
  RMS_RVC.prev_rms_index = 0;
  RMS_RVC.runningEvents = NULL;
  RMS_RVC.numRunningEvents = 0;
  RMS_RVC.allowedChanges = 1;


  Adquisition_Start("/dev/ttyACM0", 1024 * 1024 * 2, 50.0);

    struct timeval time;
    gettimeofday(&time, NULL);
    int64_t lastTimestamp = -1;
    int64_t timeUsec = (((unsigned long long) time.tv_sec) * 1000000) + time.tv_usec;

    struct tm *ptm;
    ptm = localtime(&time.tv_sec);
    int firstTenSecDiff = ptm->tm_sec % 10;
    int64_t tenSecTimestamp = (((unsigned long long) time.tv_sec + firstTenSecDiff) * 1000000);
    int j = 0;
//    int firstSampleIsZero = 0;
//    int lastCycleLength = -1;
//    float* voltageLastCycle;
    int numVoltageLastCycle = -1;
    int numSeconds = 0;
//    pthread_mutex_unlock(&REQUEST_SERVER_LOCK); //Request server start

    
    uint64_t SequenceID = 0;
    while(1) {
       struct processing_work *Work = Adquisition_GetWork();
       if((Work->numCycles == 20) && (++SequenceID % 10 == 0)) RequestServer_SetSamples(Work->Timestamp, Work->voltageSamples, Work->currentSamples, Work->cycleIndexes[19].end);

       pthread_mutex_lock(&WORK_LIST.lock);
        if(WORK_LIST.head == NULL) {
          Work->next = NULL;
        } else {
          WORK_LIST.head->prev = Work;
          Work->next = WORK_LIST.head;
        }
        WORK_LIST.head = Work;
        pthread_mutex_unlock(&WORK_LIST.lock);

        thpool_add_work(thpool, (void*) processing, Work);
        continue;
      
      //      Adquisition_Get(&Samples_Voltage[numRemainingSamples], &Samples_Current[numRemainingSamples], Samples_Len - numRemainingSamples);
      
      



      gettimeofday(&time, NULL);
      int64_t timeUsec2 = ((unsigned long long) time.tv_sec * 1000000) + time.tv_usec;
     // fprintf(stdout, "Ciclo %d | Time: %" PRId64 "\n", j + 1);



      numSeconds++;
      /**
       * STEP 2: If we have taken multiple of ten minutes => Resync clock (timestamp)
       */
      if(numSeconds % 600 == 0) {//Ten minutes
        gettimeofday(&time, NULL);
        timeUsec = ((unsigned long long) time.tv_sec * 1000000) + time.tv_usec;
        numSeconds = 0;
      }
      if(numSeconds % 30 == 0) {
        if(checkConnection()) {
          setLed(LED_INTERNET_CONNECTIVITY, LED_ON);
        } else {
          setLed(LED_INTERNET_CONNECTIVITY, LED_OFF);
        }
      }

//      Samples_Timestamp = timeUsec - numRemainingSamples*TIME_STEP_PER_SAMPLE;
//      int totalLength = Samples_Len + numRemainingSamples;
//      int numSamplesToProcess = totalLength;
 
//      pthread_mutex_unlock(&Samples_Mutex);
 
//      continue;
      /**
       * STEP 6: While the remaining samples to be processed > 10 cycles of samples
       */

      /**
       * Low pass Filter to all samples
       */

      /**
       * Do zero crossing
       */
      int offset = 0;
      int maxInterval = CONFIG.electrical.sample_rate * 236 / 1000; //Max. of 236 ms in samples to try to get 10 cycles using zeroCrossing().
      while(1 != 1 /*&& numSamplesToProcess > maxInterval*/) { //While we can take 236ms, keep processing
//        float *voltage_in;
//        float *current_in;
        struct cycle* cycle_indexes = calloc(20, sizeof(struct cycle));
        int readCycles;
        int signalLength = 0;

        
/*       ESTABA SIN COMENTAR 
        
        
        if(signalLength = zeroCrossing(Samples_Voltage, cycle_indexes, &readCycles, 20, offset, maxInterval, &firstSampleIsZero, &lastCycleLength)) {//We success getting the cycles
          numSamplesToProcess -= signalLength;
//          current_in = malloc(sizeof(float)*signalLength);
//          unshort2float(samples[0], current_in, offset, signalLength); //Convert current signal samples to float.
          offset += signalLength;
          if(readCycles > 0) {
            offset += cycle_indexes[0].start;
          }
        }
*/
        /*if (sequenceId % 5 == 0) {
            fprintf(stderr, "[%jd] %d\n", sequenceId, readCycles);
        }*/

        //                fprintf(stderr, "\n\n\n** Secuencia: %jd | num ciclos: %d | longitud: %d | Comienzo ciclo 1: %d**\n\n", sequenceId, readCycles, signalLength, cycle_indexes[0].start);
        //                for (i = 0; i < signalLength; i++) {
        //                    fprintf(stderr, "%f\n", voltage_in[i]);
        //                }

        //debug_print("\nSecuencia: %jd: | Longitud señal: %d | Ciclos leidos: %d\n", sequenceId, signalLength, readCycles);
        //debug_print("\nIteracion: %d: | Longitud señal: %d | Ciclos leidos: %d\n", j, signalLength, readCycles);

        /**
         * Insert work in thread pool.
         */
//        struct processing_work* Work = malloc(sizeof(struct processing_work));
//        Work->sequence_id = SequenceId++;
//        Work->from = processedSamples;
//        Work->voltageSamples = voltage_in;
//        Work->currentSamples = current_in;
//        Work->cycleIndexes = cycle_indexes;
//        Work->numCycles = readCycles;
//        Work->index10SecLimit = -1;
//        Work->numVoltageSamples = signalLength;
//        Work->numVoltageLastCycleSamples = numVoltageLastCycle;
//        Work->voltageLastCycleSamples = voltageLastCycle;
//        Work->prev = NULL;
//        Work->lastTimestamp = lastTimestamp;
//        Work->timestamp = Samples_Timestamp;
//        lastTimestamp = Samples_Timestamp;
//        Work->tenSecTimestamp = tenSecTimestamp;
        pthread_mutex_lock(&WORK_LIST.lock);
        if(WORK_LIST.head == NULL) {
          Work->next = NULL;
        } else {
          WORK_LIST.head->prev = Work;
          Work->next = WORK_LIST.head;
        }
        WORK_LIST.head = Work;
        pthread_mutex_unlock(&WORK_LIST.lock);

        thpool_add_work(thpool, (void*) processing, Work);

        /**
         * Save last cycles for the next interval
         */
      numVoltageLastCycle = numLastSamplesToSave;
//      voltageLastCycle = calloc(numVoltageLastCycle, sizeof(float));
      for(int i = 0; i < numLastSamplesToSave; i++) {
//        voltageLastCycle[i] = voltage_in[signalLength - numLastSamplesToSave + i];
      }
      Samples_Timestamp = Samples_Timestamp + signalLength * TIME_STEP_PER_SAMPLE;
      if(Samples_Timestamp > tenSecTimestamp) {
        tenSecTimestamp += TIME_USEC_TEN_SECONDS;
      }
    }
    processedSamples += offset;
//    numRemainingSamples = totalLength - offset;
    
    

//    memcpy(Samples_Voltage, &Samples_Voltage[offset], sizeof(float) * numRemainingSamples);
//    memcpy(Samples_Current, &Samples_Current[offset], sizeof(float) * numRemainingSamples);

    
//    for(int i = 0; i < numSignals; i++) {
//      memmove(samples[i], samples[i] + offset, sizeof(uint16_t) * numRemainingSamples); //Copy remaining samples
//    }
    timeUsec += 1e6; //Add one second to reference timestamp
    j++;
  }
  Adquisition_Stop(&Adquisition);
  thpool_wait(thpool);
  thpool_destroy(thpool);
//  for(int i = 0; i < numSignals; i++) free(samples[i]);
//  free(samples);
  free(Samples_Voltage);
  free(Samples_Current);
  return processedSamples;
}

