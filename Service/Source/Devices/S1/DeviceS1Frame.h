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
// S1 frame format (e-MUCS H ed. 1.3, section 4.2). Self contained (no openZmeter dependencies) so it can be unit tested.
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <stdint.h>
// ---------------------------------------------------------------------------------------------------------------------
class DeviceS1Frame final {
  public :
    static const uint8_t  FRAME_LEN      = 45;       //Total frame length including both flags
    static const uint8_t  FRAME_FLAG     = 0x7E;     //HDLC flag
    static const uint8_t  FRAME_FORMAT1  = 0x80;     //Frame format type 1
    static const uint8_t  FRAME_FORMAT2  = 0x2B;     //Frame length (43) excluding flags
    static const uint8_t  FRAME_ADDRESS  = 0xFF;     //All stations broadcast
    static const uint8_t  FRAME_CONTROL  = 0x03;     //UI frame
  public :
    typedef struct {
      char     MeterID[15];                            //Meter identifier (DIN 43863-5), printable chars, NUL terminated
      bool     PolyPhase;                              //Meter type
      bool     PerPeriod;                              //Sampling type (fixed per period / fixed per second)
      bool     FourWires;                              //3/4 wires mode (poly phase meters)
      bool     Valid;                                  //Samples are valid
      bool     Neutral;                                //Neutral current provided
      uint8_t  Version;                                //Data format version
      int8_t   Sampling;                               //Raw sampling field (samples/period or x100 Hz)
      float    Frequency;                              //Grid frequency (Hz)
      uint8_t  Sequence;                               //Frame sequence number
      float    Voltage[3];                             //Instantaneous voltages (V), L1, L2, L3
      float    Current[4];                             //Instantaneous currents (A), L1, L2, L3, N
    } Frame_t;
  public :
    static uint16_t CRC16(const uint8_t *data, const uint16_t len);
    // Decode a FRAME_LEN bytes frame starting at the opening flag. Returns false if format or FCS are wrong.
    static bool     Decode(const uint8_t *buff, Frame_t &frame);
};
// ---------------------------------------------------------------------------------------------------------------------
