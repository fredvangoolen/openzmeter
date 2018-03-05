/* ================================== zMeter ===========================================================================
 * File:     Resources.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <stdint.h>
#include <stdlib.h>
class Resources {
  private :
    typedef struct {
      char    *Name;
      uint32_t Size;
      uint8_t *Ptr;
    } File_t;
  private :
    static File_t  *Files;
    static uint32_t FileCount;
  public :
    Resources();
  public :
    uint8_t *GetFile(const char *mame, uint32_t *size);
    
};
