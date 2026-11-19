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
#pragma once
#include <stdint.h>
// ---------------------------------------------------------------------------------------------------------------------
class DeviceS1Serial final {
  public :
    // Open a serial port read only, raw 8N1 at an arbitrary baudrate. read() returns 0 after timeoutDs tenths of second
    // without data. Returns file descriptor or -1 (errno set). Fails with EINVAL if the adapter can not reach the speed.
    static int Open(const char *path, const uint32_t baudrate, const uint8_t timeoutDs);
};
// ---------------------------------------------------------------------------------------------------------------------
