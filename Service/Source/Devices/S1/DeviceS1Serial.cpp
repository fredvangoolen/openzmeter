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
// Serial port setup with arbitrary baudrate (termios2 / BOTHER). Kept in its own translation unit because the kernel
// <asm/termbits.h> header can not be mixed with the libc <termios.h> one.
// ---------------------------------------------------------------------------------------------------------------------
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include "DeviceS1Serial.h"
// ---------------------------------------------------------------------------------------------------------------------
int DeviceS1Serial::Open(const char *path, const uint32_t baudrate, const uint8_t timeoutDs) {
#if !defined(BOTHER) || !defined(TCGETS2)
  errno = ENOTSUP;
  return -1;
#else
  int FD = open(path, O_RDONLY | O_NOCTTY | O_CLOEXEC);
  if(FD < 0) return -1;
  struct termios2 TIO;
  if(ioctl(FD, TCGETS2, &TIO) != 0) {
    int Err = errno;
    close(FD);
    errno = Err;
    return -1;
  }
  TIO.c_iflag  = IGNPAR;                                      //Raw input, no flow control, no translation
  TIO.c_oflag  = 0;
  TIO.c_lflag  = 0;
  TIO.c_cflag  = CS8 | CREAD | CLOCAL | BOTHER;               //8N1, receiver enabled, ignore modem lines
  TIO.c_ispeed = baudrate;
  TIO.c_ospeed = baudrate;
  TIO.c_cc[VMIN]  = 0;                                        //Return with whatever is available...
  TIO.c_cc[VTIME] = timeoutDs;                                //...or after timeout (tenths of second)
  if(ioctl(FD, TCSETS2, &TIO) != 0) {
    int Err = errno;
    close(FD);
    errno = Err;
    return -1;
  }
  if((ioctl(FD, TCGETS2, &TIO) != 0) || (TIO.c_ispeed < baudrate * 0.98) || (TIO.c_ispeed > baudrate * 1.02)) {
    close(FD);
    errno = EINVAL;                                           //Adapter can not run at requested speed
    return -1;
  }
  ioctl(FD, TCFLSH, TCIFLUSH);
  return FD;
#endif
}
// ---------------------------------------------------------------------------------------------------------------------
