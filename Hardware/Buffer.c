/* ================================ openZmeter =========================================================================
 * File:     Buffer.h
 * Author:   Eduardo Viciana (2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include "Main.h"
//----------------------------------------------------------------------------------------------------------------------
static uint8_t  Status;               //[7] Stream reset, [2-0]   Channel count
static uint16_t INBuff[INBUFFER_LEN]; //Buffer de muestras
static uint16_t INReadPos;
static uint16_t INWritePos;
static uint16_t INBuffUsed;
//----------------------------------------------------------------------------------------------------------------------

/**
 * Reinicia los contadores del buffer a sus valores por defecto
 */
void Buffers_Reset() {
  INReadPos  = 0;
  INWritePos = 0;
  INBuffUsed = 0;
  Status     = Status | 0x80;
}

/**
 * Inicia el buffer, con todos los contadores asociados
 * @param channels numero de canales usados
 */
void Buffers_Init(uint8_t channels) {
  Status = channels & 0x07;
  Buffers_Reset();
}

/**
 * Añade muestras al buffer de salida si hay espacio, en caso contrario reinicia el buffer previamente
 * @param samples muestras a añadir, segun la cantidad de canales
 */
void Buffer_AppendSamples(float *samples) {
  uint8_t Channels = (Status & 0x07);
  if(INBuffUsed >= (INBUFFER_LEN - Channels)) Buffers_Reset();
  for(uint8_t i = 0; i < Channels; i++) {
    float    Value = samples[i] * 8.0 + 0.5;
    if(Value < 0) 
      INBuff[INWritePos] = 0;
    else if(Value > 65534)
      INBuff[INWritePos] = 65534;
    else
       INBuff[INWritePos] = (uint16_t)Value;
    INWritePos = (INWritePos + 1) % INBUFFER_LEN;
  }
  INBuffUsed = INBuffUsed + Channels;
}

/**
 * Genera un paquete de datos para ser enviado por USB. El tamaño del paquete
 * es de 64 bytes donde entra un maximo de 32 muestras.
 *   Si no hay datos para enviar se devuelve FALSE
 *   Si el buffer esta en estado de reset la primera muestra es 0xFFFF, y se limpiara el estado de reset
 *   Si una muestra no se usa (el buffer tiene menos de 32 muestras) se usara el valor 0xFFFF para indicarlo
 * @param dst posicion de escritura del buffer (debe de tener 64 bytes)
 * @return TRUE si hay datos para enviar, FALSE en otro caso
 */
bool Buffers_ToPMA(uint16_t dst) {
  if(INBuffUsed == 0) return FALSE;
  uint32_t *Ptr  = (uint32_t *)(dst * 2 + PMAAddr);
  uint8_t   Pos = 0;
  if(Status & 0x80) {
    Ptr[Pos++] = 0xFFFF;
    Status = Status & 0x7F;
  }
  while(Pos < 32) {
    if(INBuffUsed) {
      Ptr[Pos++] = INBuff[INReadPos];
      INReadPos = (INReadPos + 1) % INBUFFER_LEN;
      INBuffUsed--;
    } else {
      Ptr[Pos++] = 0xFFFF;
    }
  }
  return TRUE;
}