/* ================================ openZmeter =========================================================================
 * File:     Buffer.h
 * Author:   Eduardo Viciana (2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include "Main.h"
//----------------------------------------------------------------------------------------------------------------------
static uint8_t  Sequence;
static uint8_t  Channels;
static uint16_t INBuff[INBUFFER_LEN];
static uint16_t INReadPos;
static uint16_t INWritePos;
static uint16_t INBuffUsed;
//----------------------------------------------------------------------------------------------------------------------

/**
 * Reinicia los contadores del buffer a sus valores por defecto
 */
void Buffers_Reset() {
  Sequence   = 0;
  INReadPos  = 0;
  INWritePos = 0;
  INBuffUsed = 0;
  Channels   = Channels & 0x0F;
}

/**
 * Inicia el buffer, con todos los contadores asociados
 * @param channels numero de canales usados
 */
void Buffers_Init(uint8_t channels) {
  Channels = channels & 0x0F;
  Buffers_Reset();
}

/**
 * Añade muestras al buffer de salida si hay espacio, en caso contrario reinicia el buffer previamente
 * @param samples muestras a añadir, segun la cantidad de canales
 */
void Buffer_AppendSamples(float *samples) {
  if(INBuffUsed >= (INBUFFER_LEN - Channels)) Buffers_Reset();
  for(uint8_t i = 0; i < Channels; i++) {
    INBuff[INWritePos++] = (uint16_t)samples[i];
    INWritePos = INWritePos % INBUFFER_LEN;
  }
  INBuffUsed = INBuffUsed + Channels;
}

/**
 * Genera un paquete de datos para ser enviado por USB. El tamaño del paquete
 * es de 64 bytes. El primer byte contiene un numero de sequencia que se incrementa
 * en cada paquete enviado. El bits [7-5] del segundo byte indican el primer canal
 * que contiene el paquete, los bits [4-0] indican el numero de muestras del paquete.
 * @param dst posicion de escritura del buffer (debe de tener al menos 64 bytes)
 * @return TRUE si hay datos para enviar, FALSE en otro caso
 */
bool Buffers_ToPMA(uint16_t dst) {
  uint32_t *Ptr = (uint32_t *)(dst * 2 + PMAAddr);
  uint8_t   AvaliableSamples = (INBuffUsed > 31) ? 31 : INBuffUsed;
  INBuffUsed = INBuffUsed - AvaliableSamples;
  *Ptr++ = Sequence++ | (AvaliableSamples << 8);
  while(AvaliableSamples--) {
    *Ptr++ = INBuff[INReadPos++];
    INReadPos = INReadPos % INBUFFER_LEN;
  }
  return TRUE;
}