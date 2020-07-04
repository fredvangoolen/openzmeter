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
#include <stdlib.h>
#include "USBTransfer.h"
// ---------------------------------------------------------------------------------------------------------------------
USBTransfer::USBTransfer(void *parent, uint32_t size) {
  pTransfer = libusb_alloc_transfer(0);
  pBufferSize = size;
  pParent = parent;
  pBuffer = (uint8_t*)malloc(size);
}
USBTransfer::~USBTransfer() {
  if(pBuffer != NULL) free(pBuffer);
  if(pTransfer != NULL) libusb_free_transfer(pTransfer);
}
int USBTransfer::Submit() {
  return libusb_submit_transfer(pTransfer);
}
int USBTransfer::Submit(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout) {
  Setup(deviceHandle, endpoint, callback, timeout);
  return Submit();
}
void USBTransfer::Setup(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout) {
  libusb_fill_bulk_transfer(pTransfer, deviceHandle, endpoint, pBuffer, pBufferSize, callback, pParent, timeout);
}
int USBTransfer::Submit(void *data, uint32_t size) {
  memcpy(pBuffer, data, size);
  libusb_submit_transfer(pTransfer);
  return Submit();
}
//----------------------------------------------------------------------------------------------------------------------