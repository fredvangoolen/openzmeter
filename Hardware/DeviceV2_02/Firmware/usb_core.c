/* This file is the part of the Lightweight USB device Stack for STM32 microcontrollers
 *
 * Copyright ©2016 Dmitry Filimonchuk <dmitrystu[at]gmail[dot]com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include <stdint.h>
//#include <stdbool.h>
#include "STM32F0xx.h"
#include "USB.h"







/** \brief General event processing callback
 * \param dev usb device
 * \param evt usb event
 * \param ep active endpoint
 */
/*
void usbd_process_evt(usbd_device *dev, uint8_t evt, uint8_t ep) {
  switch(evt) {
    case usbd_evt_eprx:
    case usbd_evt_eptx:
    case usbd_evt_epsetup:
      if(dev->endpoint[ep & 0x07]) dev->endpoint[ep & 0x07](dev, evt, ep);
      break;
    default:
      break;
  }
  if(dev->events[evt]) dev->events[evt](dev, evt, ep);
}
*/
