// Modified by Eduardo Viciana (zRed)

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
#pragma once
#include <stdbool.h>
#include <stdint.h>
//Defines ----------------------------------------------------------------------
#define USB_EVENT_RESET     0   /* Reset.*/
#define usbd_evt_sof        1   /**<\brief Start of frame.*/
#define usbd_evt_susp       2   /**<\brief Suspend.*/
#define usbd_evt_wkup       3   /**<\brief Wakeup.*/
#define USB_EVENT_TX        4   /**<\brief Data packet transmitted*/
#define USB_EVENT_RX        5   /**<\brief Data packet received.*/
#define usbd_evt_epsetup    6   /**<\brief Setup packet received.*/
#define usbd_evt_error      7   /**<\brief Data error.*/
#define usbd_evt_count      8

/* USB device machine states */
typedef enum {
  USB_STATE_DEFAULT,         /* Default */
  USB_STATE_ADDRESSED,       /* Addressed */
  USB_STATE_CONFIGURED,      /* Configured */
} USB_State;

/* USB device control endpoint machine state */
typedef enum {
  USB_CONTROL_IDLE,              /* Idle stage. Awaiting for SETUP packet */
  USB_CONTROL_RXDATA,            /* RX stage. Receiving DATA-OUT payload */
  USB_CONTROL_TXDATA,            /* TX stage. Transmitting DATA-IN payload */
  USB_CONTROL_TXPAYLOAD,         /* TX stage. Transmitting DATA-IN payload. Zero length packet maybe required */
  USB_CONTROL_LASTDATA,          /* TX stage. Last DATA-IN packed passed to buffer. Awaiting for the TX completion */
  USB_CONTROL_STATUSIN,          /* STATUS-IN stage */
  USB_CONTROL_STATUSOUT,         /* STATUS-OUT stage */
} USB_Control_State;

/* Reporting status results */
typedef enum {
  USB_FAIL,                      /* Function has an error, STALLPID will be issued. */
  USB_ACK,                       /* Function completes request accepted ZLP or data will be send.*/
  USB_NACK,                      /* Function is busy. NAK handshake.*/
} USB_Response;

/* Represents generic USB control request */
typedef struct {
  uint8_t     bmRequestType;  /**<\brief This bitmapped field identifies the characteristics of the specific request.*/
  uint8_t     bRequest;       /**<\brief This field specifies the particular request.*/
  uint16_t    wValue;         /**<\brief It is used to pass a parameter to the device, specific to the request.*/
  uint16_t    wIndex;         /**<\brief It is used to pass a parameter to the device, specific to the request.*/
  uint16_t    wLength;        /**<\brief This field specifies the length of the data transferred during the second phase of the control transfer.*/
  uint8_t     data[];         /**<\brief Data payload.*/
} USB_ControlRequest_t;

/** USB device status data.*/
typedef struct {
  void        *data_buf;      /**<\brief Pointer to data buffer used for control requests.*/
  void        *data_ptr;      /**<\brief Pointer to current data for control request.*/
  uint16_t    data_count;     /**<\brief Count remained data for control request.*/
  uint16_t    data_maxsize;   /**<\brief Size of the data buffer for control requests.*/
  uint8_t     ep0size;        /**<\brief Size of the control endpoint.*/
  uint8_t     device_cfg;     /**<\brief Current device configuration number.*/
  uint8_t     device_state;   /**<\brief Current \ref usbd_machine_state.*/
  uint8_t     control_state;  /**<\brief Current \ref usbd_ctl_state.*/
} USB_Status_t;

typedef void (*USB_Event_Callback_t)(uint8_t event, uint8_t ep);
typedef void (*USB_Request_Callback_t)(USB_ControlRequest_t *req);

/* Represents a USB device data */
typedef struct {
  USB_Request_Callback_t Complete_Callback;
  USB_Event_Callback_t   Endpoint[8];
  volatile USB_Status_t  Status;
} USB_Device_t;

typedef struct {
  uint16_t addr;
  uint16_t cnt;
} PMA_Record_t;

typedef union {
  struct {
    PMA_Record_t tx;
    PMA_Record_t rx;
  };
  struct {
    PMA_Record_t tx0;
    PMA_Record_t tx1;
  };
  struct {
    PMA_Record_t rx0;
    PMA_Record_t rx1;
  };
} PMA_Table_t;


 // Functions -----------------------------------------------------------------------------------------------------------
void         USB_Init();
uint8_t      USB_Connect(bool connect);
void         USB_Enable(bool enable);
void         USB_RegisterEP(uint8_t ep, USB_Event_Callback_t callback);
bool         USB_ConfigEP(uint8_t ep, uint8_t eptype, uint16_t epsize);
void         USB_DeconfigEP(uint8_t ep);
int32_t      USB_ReadEP(uint8_t ep, void *buf, uint16_t blen);
int32_t      USB_WriteEP(uint8_t ep, void *buf, uint16_t blen);
USB_Response USB_HandleEP0_Class(USB_ControlRequest_t *req, USB_Request_Callback_t *callback);
USB_Response USB_SetConfiguration (uint8_t cfg);



/**\addtogroup USBD_HW USB hardware driver
 * \brief Contains HW driver API
 * @{ */
/**\anchor USB_EVENTS
 * \name USB device events
 * @{ */

/** @} */

/**\anchor USB_LANES_STATUS
 * \name USB lanes connection states
 * @{ */
#define usbd_lane_unk       0   /**<\brief Unknown or proprietary charger.*/
#define usbd_lane_dsc       1   /**<\brief Lanes disconnected.*/
#define usbd_lane_sdp       2   /**<\brief Lanes connected to standard downstream port.*/
#define usbd_lane_cdp       3   /**<\brief Lanes connected to charging downstream port.*/
#define usbd_lane_dcp       4   /**<\brief Lanes connected to dedicated charging port.*/
/** @} */

/**\anchor USBD_HW_CAPS
 * \name USB HW capabilities and status
 * @{ */
#define USBD_HW_ADDRFST     (1 << 0)    /**<\brief Set address before STATUS_OUT.*/
#define USBD_HW_BC          (1 << 1)    /**<\brief Battery charging detection supported.*/
//#define USND_HW_HS          (1 << 2)    /**<\brief High speed supported.*/
#define USBD_HW_ENABLED     (1 << 3)    /**<\brief USB device enabled. */
//#define USBD_HW_ENUMSPEED   (2 << 4)    /**<\brief USB device enumeration speed mask.*/
//#define USBD_HW_SPEED_NC    (0 << 4)    /**<\brief Not connected */
//#define USBD_HW_SPEED_LS    (1 << 4)    /**<\brief Low speed */
#define USBD_HW_SPEED_FS    (2 << 4)    /**<\brief Full speed */
//#define USBD_HW_SPEED_HS    (3 << 4)    /**<\brief High speed */

/** @} */
/** @} */

/**\addtogroup USBD_CORE USB device core
 * \brief Contains core API
 * @{ */
#define USB_EPTYPE_DBLBUF   0x04    /**<\brief Doublebuffered endpoint (bulk endpoint only).*/

/**\name bmRequestType bitmapped field
 * @{ */
#define USB_REQ_DIRECTION   (1 << 7)    /**<\brief Request direction mask.*/
#define USB_REQ_HOSTTODEV   (0 << 7)    /**<\brief Request direction is HOST to DEVICE.*/
#define USB_REQ_DEVTOHOST   (1 << 7)    /**<\brief Request direction is DEVICE to HOST.*/
#define USB_REQ_TYPE        (3 << 5)    /**<\brief Request type mask.*/
#define USB_REQ_STANDARD    (0 << 5)    /**<\brief Standard request.*/
#define USB_REQ_CLASS       (1 << 5)    /**<\brief Class specified request.*/
#define USB_REQ_VENDOR      (2 << 5)    /**<\brief Vendor specified request.*/
#define USB_REQ_RECIPIENT   (3 << 0)    /**<\brief Request recipient mask.*/
#define USB_REQ_DEVICE      (0 << 0)    /**<\brief Request to device.*/
#define USB_REQ_INTERFACE   (1 << 0)    /**<\brief Request to interface.*/
#define USB_REQ_ENDPOINT    (2 << 0)    /**<\brief Request to endpoint.*/
#define USB_REQ_OTHER       (3 << 0)    /**<\brief Other request.*/

#define USB_EP_SWBUF_TX     USB_EP_DTOG_RX
#define USB_EP_SWBUF_RX     USB_EP_DTOG_TX
#define EP_TOGGLE_SET(epr, bits, mask) *(epr) = (*(epr) ^ (bits)) & (USB_EPREG_MASK | (mask))
#define EP_TX_STALL(epr)    EP_TOGGLE_SET((epr), USB_EP_TX_STALL,                   USB_EPTX_STAT)
#define EP_RX_STALL(epr)    EP_TOGGLE_SET((epr), USB_EP_RX_STALL,                   USB_EPRX_STAT)
#define EP_TX_UNSTALL(epr)  EP_TOGGLE_SET((epr), USB_EP_TX_NAK,                     USB_EPTX_STAT | USB_EP_DTOG_TX)
#define EP_RX_UNSTALL(epr)  EP_TOGGLE_SET((epr), USB_EP_RX_VALID,                   USB_EPRX_STAT | USB_EP_DTOG_RX)
#define EP_DTX_UNSTALL(epr) EP_TOGGLE_SET((epr), USB_EP_TX_VALID,                   USB_EPTX_STAT | USB_EP_DTOG_TX | USB_EP_SWBUF_TX)
#define EP_DRX_UNSTALL(epr) EP_TOGGLE_SET((epr), USB_EP_RX_VALID | USB_EP_SWBUF_RX, USB_EPRX_STAT | USB_EP_DTOG_RX | USB_EP_SWBUF_RX)
#define EP_TX_VALID(epr)    EP_TOGGLE_SET((epr), USB_EP_TX_VALID,                   USB_EPTX_STAT)
#define EP_RX_VALID(epr)    EP_TOGGLE_SET((epr), USB_EP_RX_VALID,                   USB_EPRX_STAT)
#define STATUS_VAL(x)   (USBD_HW_BC | (x))

#define USB_EPDIR_IN                0x00    /**<\brief Host-to-device endpoint direction.*/
#define USB_EPDIR_OUT               0x80    /**<\brief Device-to-host endpoint direction.*/
#define USB_EPTYPE_CONTROL          0x00    /**<\brief Control endpoint.*/
#define USB_EPTYPE_ISOCHRONUS       0x01    /**<\brief Isochronous endpoint.*/
#define USB_EPTYPE_BULK             0x02    /**<\brief Bbulk endpoint.*/
#define USB_EPTYPE_INTERRUPT        0x03    /**<\brief Interrupt endpoint.*/
#define USB_EPATTR_NO_SYNC          0x00    /**<\brief No synchronization.*/
#define USB_EPATTR_ASYNC            0x04    /**<\brief Asynchronous endpoint.*/
#define USB_EPATTR_ADAPTIVE         0x08    /**<\brief Adaptive endpoint.*/
#define USB_EPATTR_SYNC             0x0C    /**<\brief Synchronous endpoint.*/
#define USB_EPUSAGE_DATA            0x00    /**<\brief Data endpoint.*/
#define USB_EPUSAGE_FEEDBACK        0x10    /**<\brief Feedback endpoint.*/
#define USB_EPUSAGE_IMP_FEEDBACK    0x20    /**<\brief Implicit feedback Data endpoint.*/
/** @} */

/**\name Special string descriptor indexes
 * @{ */
//#define NO_DESCRIPTOR               0x00    /**<\brief String descriptor doesn't exists in the device.*/
//#define INTSERIALNO_DESCRIPTOR      0xFE    /**<\brief String descriptor is an internal serial number
//                                             * provided by hardware driver.*/
/** @} */

/**\name USB class definitions
 * @{ */
#define USB_CLASS_PER_INTERFACE     0x00    /**<\brief Class defined on interface level.*/
#define USB_SUBCLASS_NONE           0x00    /**<\brief No subclass defined.*/
#define USB_PROTO_NONE              0x00    /**<\brief No protocol defined.*/
#define USB_CLASS_AUDIO             0x01    /**<\brief Audio device class.*/
#define USB_CLASS_PHYSICAL          0x05    /**<\brief Physical device class.*/
#define USB_CLASS_STILL_IMAGE       0x06    /**<\brief Still Imaging device class.*/
#define USB_CLASS_PRINTER           0x07    /**<\brief Printer device class.*/
#define USB_CLASS_MASS_STORAGE      0x08    /**<\brief Mass Storage device class.*/
#define USB_CLASS_HUB               0x09    /**<\brief HUB device class.*/
#define USB_CLASS_CSCID             0x0B    /**<\brief Smart Card device class.*/
#define USB_CLASS_CONTENT_SEC       0x0D    /**<\brief Content Security device class.*/
#define USB_CLASS_VIDEO             0x0E    /**<\brief Video device class.*/
#define USB_CLASS_HEALTHCARE        0x0F    /**<\brief Personal Healthcare device class.*/
#define USB_CLASS_AV                0x10    /**<\brief Audio/Video device class.*/
#define USB_CLASS_BILLBOARD         0x11    /**<\brief Billboard device class.*/
#define USB_CLASS_CBRIDGE           0x12    /**<\brief USB Type-C Bridge device class.*/
#define USB_CLASS_DIAGNOSTIC        0xDC    /**<\brief Diagnostic device class.*/
#define USB_CLASS_WIRELESS          0xE0    /**<\brief Wireless controller class.*/
#define USB_CLASS_MISC              0xEF    /**<\brief Miscellanious device class.*/
#define USB_CLASS_APP_SPEC          0xFE    /**<\brief Application Specific class.*/
#define USB_CLASS_VENDOR            0xFF    /**<\brief Vendor specific class.*/
#define USB_SUBCLASS_VENDOR         0xFF    /**<\brief Vendor specific subclass.*/
#define USB_PROTO_VENDOR            0xFF    /**<\brief Vendor specific protocol.*/
#define USB_CLASS_IAD               0xEF    /**<\brief Class defined on interface association level.*/
#define USB_SUBCLASS_IAD            0x02    /**<\brief Subclass defined on interface association level.*/
#define USB_PROTO_IAD               0x01    /**<\brief Protocol defined on interface association level.*/
/** @} */

/**\name USB Standard descriptor types
 * @{ */
#define USB_DTYPE_DEVICE            0x01    /**<\brief Device descriptor.*/
#define USB_DTYPE_CONFIGURATION     0x02    /**<\brief Configuration descriptor.*/
#define USB_DTYPE_STRING            0x03    /**<\brief String descriptor.*/
#define USB_DTYPE_INTERFACE         0x04    /**<\brief Interface descriptor.*/
#define USB_DTYPE_ENDPOINT          0x05    /**<\brief Endpoint  descriptor.*/
#define USB_DTYPE_QUALIFIER         0x06    /**<\brief Qualifier descriptor.*/
#define USB_DTYPE_OTHER             0x07    /**<\brief Descriptor is of other type. */
#define USB_DTYPE_INTERFACEPOWER    0x08    /**<\brief Interface power descriptor. */
#define USB_DTYPE_OTG               0x09    /**<\brief OTG descriptor.*/
#define USB_DTYPE_DEBUG             0x0A    /**<\brief Debug descriptor.*/
#define USB_DTYPE_INTERFASEASSOC    0x0B    /**<\brief Interface association descriptor.*/
#define USB_DTYPE_CS_INTERFACE      0x24    /**<\brief Class specific interface descriptor.*/
#define USB_DTYPE_CS_ENDPOINT       0x25    /**<\brief Class specific endpoint descriptor.*/
/** @} */

/**\name USB Standard requests
 * @{ */
#define USB_STD_GET_STATUS          0x00    /**<\brief Returns status for the specified recipient.*/
#define USB_STD_CLEAR_FEATURE       0x01    /**<\brief Used to clear or disable a specific feature.*/
#define USB_STD_SET_FEATURE         0x03    /**<\brief Used to set or enable a specific feature.*/
#define USB_STD_SET_ADDRESS         0x05    /**<\brief Sets the device address for all future device
                                             * accesses.*/
#define USB_STD_GET_DESCRIPTOR      0x06    /**<\brief Returns the specified descriptor if the
                                             * descriptor exists.*/
#define USB_STD_SET_DESCRIPTOR      0x07    /**<\brief This request is optional and may be used to
                                             * update existing descriptors or new descriptors may be
                                             * added.*/
#define USB_STD_GET_CONFIG          0x08    /**<\brief Returns the current device configuration value.*/
#define USB_STD_SET_CONFIG          0x09    /**<\brief Sets the device configuration.*/
#define USB_STD_GET_INTERFACE       0x0A    /**<\brief Returns the selected alternate setting for
                                             * the specified interface.*/
#define USB_STD_SET_INTERFACE       0x0B    /**<\brief Allows the host to select an alternate setting
                                             * for the specified interface.*/
#define USB_STD_SYNCH_FRAME         0x0C    /**<\brief Used to set and then report an endpoint's
                                             * synchronization frame.*/



extern USB_Device_t USB_Device;