/* ================================== zMeter ===========================================================================
 * File:     Tools.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Tools.h"
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//----------------------------------------------------------------------------------------------------------------------
bool  Tools::Daemonized = false;
bool  Tools::Debug = false;
FILE *Tools::LogFile = NULL;
char *Tools::PidFileName  = (char*)"/var/run/openZmeter.pid";
//----------------------------------------------------------------------------------------------------------------------
void Tools::LogMsg(const char *level, const char *msg, ...) {
  if((Debug == false) && (strcmp(level, "DEBUG") == 0)) return;
  va_list args;
  va_start(args, msg);
  time_t Now;
  struct tm *TimeStamp;
  char DateStr[80];
  time(&Now);
  TimeStamp = localtime(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X] ", TimeStamp);
  if(Daemonized == false) {
    flockfile(stdout);
    if(strcmp(level, "ERROR") == 0) {
      printf("\033[1;31m%s\033[0m", DateStr);
    } else if(strcmp(level, "DEBUG") == 0) {
      printf("\033[1;33m%s\033[0m", DateStr);
    } else if(strcmp(level, "INFO") == 0) {
      printf("\033[1;34m%s\033[0m", DateStr);
    } else {
      return;
    }
    vprintf(msg, args);
    fflush(stdout);
    funlockfile(stdout);
  }
  if(LogFile) {
    flockfile(LogFile);
    fprintf(LogFile, "%s", DateStr);
    vfprintf(LogFile, msg, args);
    fflush(LogFile);
    funlockfile(LogFile);
  }
  va_end(args);
}
bool Tools::SetLogFile(const char *filename) {
  if(LogFile == NULL) fclose(Tools::LogFile);
  LogFile = fopen(filename, "at");
  if(!LogFile) LogFile = fopen(filename, "wt");
  if(!LogFile) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  return true;
}
bool Tools::CreatePidFile() {
  char PID_Number[64];
  int Len = sprintf(PID_Number, "%d", getpid());
  remove(PidFileName);
  int File = open(PidFileName, O_CREAT, O_WRONLY);
  if(File < 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  write(File, PID_Number, Len);
  close(File);
  return true;
}
bool Tools::RemovePidFile() {
 return remove(PidFileName);
}
void Tools::Daemonize() {
  pid_t ProcID = fork();
  if(ProcID < 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  chdir("/");
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}




#include <ctype.h>



#include "Main.h"
Config_t Config;


#include <stdlib.h>

#define MAX_SECTION 50
#define MAX_NAME 50

char *rstrip(char* s) {
  char *p = s + strlen(s);
  while(p > s && isspace((unsigned char)(*--p))) *p = '\0';
  return s;
}
char *lskip(const char* s) {
  while (*s && isspace((unsigned char)(*s))) s++;
  return (char*)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* See documentation in header file. */



bool Config_Handler(char *section, char *param, char *value) {
  if(strcmp(section, "database") == 0) {
    if(strcmp(param, "port") == 0) {
      Config.DB.Port = atoi(value);
    } else if(strcmp(param, "database") == 0) {
      Config.DB.Database = strdup(value);
    } else if(strcmp(param, "host") == 0) {
      Config.DB.Host = strdup(value);
    } else if(strcmp(param, "user") == 0) {
      Config.DB.User = strdup(value);
    } else if(strcmp(param, "pass") == 0) {
      Config.DB.Password = strdup(value);
    } else if(strcmp(param, "queue") == 0) {
      Config.DB.QueueLen = atoi(value);
    } else {
      LogMsg("ERROR", "Unknown param '%s' in section '%s'\n", param, section);
      return false;
    }
  } else if(strcmp(section, "electrical") == 0) {
    if(strcmp(param, "nominal_frequency") == 0) {
      Config.Electrical.nominal_frequency = atoi(value);
      if((Config.Electrical.nominal_frequency != 50) && (Config.Electrical.nominal_frequency != 60)) Config.Electrical.nominal_frequency = 50;
    } else if(strcmp(param, "nominal_voltage") == 0) {
      //pconfig->electrical.nominal_voltage = atof(value);
    } else if(strcmp(param, "voltage_gain") == 0) {
      Config.Electrical.VoltageGain = atof(value);
    } else if(strcmp(param, "voltage_offset") == 0) {
      Config.Electrical.VoltageOffset = atoi(value);
    } else if(strcmp(param, "current_gain") == 0) {
      Config.Electrical.CurrentGain = atof(value);
    } else if(strcmp(param, "current_offset") == 0) {
      Config.Electrical.CurrentOffset = atoi(value);
    }
  } else if(strcmp(section, "budget")) {
    
    if(strcmp(param, "fixed_price") == 0) {
    
    if(strcmp(value, "false") == 0) {
//      pconfig->budget.fixed_price = -1;
    } else {
//      pconfig->budget.fixed_price = atof(value);
    }
//  } else if(MATCH("budget", "country")) {
//    pconfig->budget.country = strdup(value);
//    pconfig->budget.country_config = NULL;
    }
  } else {
//    if(pconfig->budget.country != NULL && strcmp(pconfig->budget.country, "spain") == 0) {
//      budget_spain_configuration * countryConfig;
//      if(pconfig->budget.country_config == NULL) {
//        pconfig->budget.country_config = malloc(sizeof(budget_spain_configuration));
//      }
//      countryConfig = (budget_spain_configuration*) (pconfig->budget.country_config);
//      if(MATCH("budget-spain", "contracted_power")) {
//        countryConfig->contracted_power = atoi(value);
//      } else if(MATCH("budget-spain", "cpc")) {
//        countryConfig->cpc = atof(value);
//      } else if(MATCH("budget-spain", "rate_type")) {
//        countryConfig->rate_type = strdup(value);
//      } else if(MATCH("budget-spain", "vat")) {
//        countryConfig->vat = atof(value);
//      } else if(MATCH("budget-spain", "discount_percentage")) {
//        countryConfig->discount_percentage = atof(value);
//      } else if(MATCH("budget-spain", "electricity_tax_percentage")) {
//        countryConfig->electricity_tax_percentage = atof(value);
//      } else if(MATCH("budget-spain", "meter_rating")) {
//        countryConfig->meter_rating = atof(value);
//      } else {
//        return 0;
//      }
//    } else {
//      return 0; /* unknown section/name, error */
//    }
  }
  return true;
}
bool Config_Init(char *filename) {
//  Config.Global.Listen = "/var/run/openZmeter.sock";
//  Config.Global.Log    = "/var/log/openZmeter.log";
  
  Config.Electrical.VoltageGain   =     0.01200000;
  Config.Electrical.VoltageOffset =   292.00000000;
  Config.Electrical.CurrentGain   =    -0.00078125;
  Config.Electrical.CurrentOffset =     0.00000000;
  if(filename == NULL) return true;
  LogMsg("INFO", "Loading config file...\n");
  FILE *File = fopen(filename, "r");
  if(!File) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  char Line[256];
  char Section[256] = "";
  int  LineNumber   = 0;
  bool Error        = false;
  while(fgets(Line, sizeof(Line), File) != NULL) {
    LineNumber++;
    char *Start = lskip(rstrip(Line));
    if((Start[0] == ';') || (Start[0] == '#') || (strlen(Start) == 0)) {
      continue;
    } else if(Start[0] == '[') {
      char *End = strchr(&Start[1], ']');
      if((End == NULL) || (&Start[1] == End)) {
        LogMsg("ERROR", "Reading config file (unmached ']' at line %d\n", LineNumber);
        Error = true;
        break;
      }
      End[0] = 0;
      strcpy(Section, &Start[1]);
    } else {
      char *End = strchr(&Start[1], '=');
      if(End == NULL) {
        LogMsg("ERROR", "Expected '=' at line %d\n", LineNumber);
        Error = true;
        break;
      }
      End[0] = 0;
      char *Param = rstrip(Start);
      char *Value = lskip(&End[1]);
      if(Config_Handler(Section, Param, Value) == false) {
        Error = true;
        break;
      }
    }
  }
  fclose(File);
  return !Error;
}