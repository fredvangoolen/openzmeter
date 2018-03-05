/* ================================== zMeter ===========================================================================
 * File:     Resources.cpp
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Resources.h"
extern char _binary_build_Resources_bin_start[];
extern char _binary_build_Resources_bin_end[];
Resources::File_t *Resources::Files = NULL;
uint32_t Resources::FileCount = 0;
Resources::Resources() {
  if(Files != NULL) return;
  char *Ptr   = _binary_build_Resources_bin_start;
  FileCount = atoi(Ptr);
  Files = (File_t*)malloc(sizeof(File_t*) * FileCount);
  Ptr = &Ptr[strlen(Ptr) + 1];
  for(uint32_t i = 0; i < FileCount; i++) {
    Files[i].Name = strdup(Ptr);
    Ptr = &Ptr[strlen(Ptr) + 1];
    Files[i].Size = atoi(Ptr);
    Ptr = &Ptr[strlen(Ptr) + 1];
    Files[i].Ptr = (uint8_t*)Ptr;
    Ptr = &Ptr[Files[i].Size];
  }
  fflush(stdout);
}
uint8_t *Resources::GetFile(const char* name, uint32_t* size) {
  for(uint32_t i = 0; i < FileCount; i++) {
    if(strcmp(name, Files[i].Name) == 0) {
      *size = Files[i].Size;
      return Files[i].Ptr;
    }
  }
  return NULL;
}