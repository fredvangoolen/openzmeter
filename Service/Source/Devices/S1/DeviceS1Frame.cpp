// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include <string.h>
#include "DeviceS1Frame.h"
// ---------------------------------------------------------------------------------------------------------------------
// CRC-16/X-25 (HDLC FCS, ISO/IEC 13239): reflected poly 0x1021, init 0xFFFF, final xor 0xFFFF
uint16_t DeviceS1Frame::CRC16(const uint8_t *data, const uint16_t len) {
  uint16_t CRC = 0xFFFF;
  for(uint16_t n = 0; n < len; n++) {
    CRC ^= data[n];
    for(uint8_t b = 0; b < 8; b++) CRC = (CRC & 0x0001) ? ((CRC >> 1) ^ 0x8408) : (CRC >> 1);
  }
  return CRC ^ 0xFFFF;
}
bool DeviceS1Frame::Decode(const uint8_t *buff, Frame_t &frame) {
  if((buff[0] != FRAME_FLAG) || (buff[1] != FRAME_FORMAT1) || (buff[2] != FRAME_FORMAT2) || (buff[3] != FRAME_ADDRESS) || (buff[4] != FRAME_CONTROL) || (buff[44] != FRAME_FLAG)) return false;
  uint16_t FCS = buff[42] | (buff[43] << 8);                  //FCS sent LSB first
  if(CRC16(&buff[1], 41) != FCS) return false;
  const uint8_t *Data = &buff[5];
  uint8_t Pos = 0;
  for(uint8_t n = 0; n < 14; n++) {
    char C = (char)Data[n];
    if((C >= 0x20) && (C < 0x7F)) frame.MeterID[Pos++] = C;
  }
  frame.MeterID[Pos] = 0;
  uint8_t Info    = Data[14];
  frame.PolyPhase = (Info & 0x01) != 0;
  frame.PerPeriod = (Info & 0x02) != 0;
  frame.FourWires = (Info & 0x04) != 0;
  frame.Valid     = (Info & 0x08) != 0;
  frame.Neutral   = (Info & 0x10) != 0;
  frame.Version   = Info >> 5;
  frame.Sampling  = (int8_t)Data[15];
  frame.Frequency = ((Data[16] << 8) | Data[17]) / 1000.0;
  frame.Sequence  = Data[18];
  auto Voltage = [](const uint8_t *p) { return (int16_t)((p[0] << 8) | p[1]) * 0.025f; };
  auto Current = [](const uint8_t *p) {
    int32_t Val = (p[0] << 16) | (p[1] << 8) | p[2];
    if(Val & 0x800000) Val -= 0x1000000;
    return Val * 0.001f;
  };
  frame.Voltage[0] = Voltage(&Data[19]);
  frame.Current[0] = Current(&Data[21]);
  frame.Voltage[1] = Voltage(&Data[24]);
  frame.Current[1] = Current(&Data[26]);
  frame.Voltage[2] = Voltage(&Data[29]);
  frame.Current[2] = Current(&Data[31]);
  frame.Current[3] = Current(&Data[34]);
  return true;
}
DeviceS1Parser::DeviceS1Parser() {
  pLen       = 0;
  pFrames    = 0;
  pCRCErrors = 0;
  pDiscarded = 0;
}
void DeviceS1Parser::Reset() {
  pLen = 0;
}
uint32_t DeviceS1Parser::Push(const uint8_t *data, size_t len, const std::function<void(const DeviceS1Frame::Frame_t &frame)> &onFrame) {
  uint32_t Found = 0;
  while(len > 0) {
    size_t Chunk = sizeof(pBuff) - pLen;
    if(Chunk > len) Chunk = len;
    memcpy(&pBuff[pLen], data, Chunk);
    pLen += Chunk;
    data += Chunk;
    len  -= Chunk;
    uint16_t Pos = 0;
    while((pLen - Pos) >= DeviceS1Frame::FRAME_LEN) {
      if((pBuff[Pos] != DeviceS1Frame::FRAME_FLAG) || (pBuff[Pos + 1] != DeviceS1Frame::FRAME_FORMAT1)) {
        if(!((pBuff[Pos] == DeviceS1Frame::FRAME_FLAG) && (pBuff[Pos + 1] == DeviceS1Frame::FRAME_FLAG))) pDiscarded++;  //Closing flag followed by next opening flag is not an error
        Pos++;
        continue;
      }
      DeviceS1Frame::Frame_t Frame;
      if(DeviceS1Frame::Decode(&pBuff[Pos], Frame) == false) {
        pCRCErrors++;
        Pos++;
        continue;
      }
      pFrames++;
      Found++;
      onFrame(Frame);
      Pos += DeviceS1Frame::FRAME_LEN - 1;             //Closing flag may also be the next opening flag
    }
    if(Pos > 0) {
      memmove(pBuff, &pBuff[Pos], pLen - Pos);
      pLen -= Pos;
    }
  }
  return Found;
}
uint64_t DeviceS1Parser::Frames() const {
  return pFrames;
}
uint64_t DeviceS1Parser::CRCErrors() const {
  return pCRCErrors;
}
uint64_t DeviceS1Parser::Discarded() const {
  return pDiscarded;
}
// ---------------------------------------------------------------------------------------------------------------------
