#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "Main.h"
Config_t Config;


#include "ini.h"

#if !INI_USE_STACK
#include <stdlib.h>
#endif

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
    } else {
      return false;
    }
  } else if(strcmp(section, "electrical") == 0) {
    if(strcmp(param, "nominal_frequency")) {
      Config.Electrical.nominal_frequency = atoi(value);
      //Comprobar frecuencia
    } else if(strcmp(param, "nominal_voltage")) {
      //pconfig->electrical.nominal_voltage = atof(value);
    } else if(strcmp(param, "voltage_scale")) {
//      pconfig->electrical.voltage_scale = atof(value);
    } else if(strcmp(param, "voltage_offset")) {
//      pconfig->electrical.voltage_offset = atoi(value);
    } else if(strcmp(param, "current_scale")) {
//      pconfig->electrical.current_scale = atof(value);
    } else if(strcmp(param, "current_offset")) {
//      pconfig->electrical.current_offset = atoi(value);
    } else if(strcmp(param, "num_stored_harmonics")) {
//      pconfig->electrical.num_stored_harmonics = atoi(value);
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
  //Inicio valores por defecto
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
        Error = true;
        break;
      }
      End[0] = 0;
      strcpy(Section, &Start[1]);
    } else {
      char *End = strchr(&Start[1], '=');
      if(End == NULL) {
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