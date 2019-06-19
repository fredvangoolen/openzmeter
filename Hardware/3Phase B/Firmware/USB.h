#pragma once
#include <stdbool.h>
#include <stdint.h>


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
  USB_FAIL,                  /**<\brief Function has an error, STALLPID will be issued.*/
  USB_ACK,                   /**<\brief Function completes request accepted ZLP or data will be send.*/
  USB_NACK,                   /**<\brief Function is busy. NAK handshake.*/
} USB_Response;

// Functions -----------------------------------------------------------------------------------------------------------
uint8_t USB_Connect(bool connect);
void    USB_Enable(bool enable);
//void    USB_SetAddress(uint8_t addr);
bool    USB_ConfigEP(uint8_t ep, uint8_t eptype, uint16_t epsize);
void    USB_DeconfigEP(uint8_t ep);
void    USB_StallEP(uint8_t ep, bool stall);
bool    USB_StalledEP(uint8_t ep);
int32_t ep_read(uint8_t ep, void *buf, uint16_t blen);
int32_t ep_write(uint8_t ep, void *buf, uint16_t blen);




//void evt_poll(usbd_device *dev, usbd_evt_callback callback);


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

#ifndef _USB_CDC_H_
#define _USB_CDC_H_

/**\addtogroup USB_CDC USB CDC class
 * \brief Generic USB CDC class definitions
 * \details This module based on
 * + Universal Serial Bus Class Definitions for Communications Devices Revision 1.2 (Errata 1)
 * @{ */

/**\name USB CDC Class codes
 * @{ */
#define USB_CLASS_CDC                   0x02    /**<\brief Communicational Device class */
#define USB_CLASS_CDC_DATA              0x0A    /**<\brief Data Interface class */
/** @} */

/**\name USB CDC subclass codes
 * @{ */
#define USB_CDC_SUBCLASS_ACM            0x02    /**<\brief Abstract Control Model */
 /** @} */

/**\name Communications Class Protocol Codes
 * @{ */
#define USB_CDC_PROTO_NONE              0x00    /**<\brief No class specific protocol required */
#define USB_CDC_PROTO_V25TER            0x01    /**<\brief AT Commands: V.250 etc.*/
 /** @} */


/** \name Data Interface Class Protocol Codes
 * @{ */
#define USB_CDC_PROTO_NTB               0x01    /**<\brief Network Transfer Block.*/
#define USB_CDC_PROTO_HOST              0xFD    /**<\brief Host based driver.
                                                 * \details This protocol code should only be  used
                                                 * in messages between host and device to identify
                                                 * the host driver portion of a protocol stack.*/
#define USB_CDC_PROTO_CDCSPEC           0xFE    /**<\brief CDC specified.
                                                 * \details The protocol(s) are described using a
                                                 * Protocol Unit Functional Descriptors on
                                                 *Communication Class Interface.*/
/** @} */

/**\name USB CDC class-specified functional descriptors
 * @{ */
#define USB_DTYPE_CDC_HEADER            0x00    /**<\brief Header Functional Descriptor.*/
#define USB_DTYPE_CDC_CALL_MANAGEMENT   0x01    /**<\brief Call Management Functional Descriptor.*/
#define USB_DTYPE_CDC_ACM               0x02    /**<\brief Abstract Control Management Functional
                                                 * Descriptor.*/
#define USB_DTYPE_CDC_UNION             0x06    /**<\brief Union Functional Descriptor.*/
#define USB_DTYPE_CDC_COUNTRY           0x07    /**<\brief Country Selection Functional Descriptor.*/
/** @} */



/** \name USB CDC class-specific requests
 * @{ */
#define USB_CDC_SEND_ENCAPSULATED_CMD   0x00    /**<\brief Used to issue a command in the format of
                                                 * the supported control protocol of the Communication
                                                 * Class interface.*/
#define USB_CDC_GET_ENCAPSULATED_RESP   0x01    /**<\brief Used to request a response in the format
                                                 * of the supported control protocol of the
                                                 * Communication Class interface.*/
#define USB_CDC_SET_COMM_FEATURE        0x02    /**<\brief Controls the settings for a particular
                                                 * communication feature  of a particular target.*/
#define USB_CDC_GET_COMM_FEATURE        0x03    /**<\brief Returns the current settings for the
                                                 * communication feature as selected.*/
#define USB_CDC_CLEAR_COMM_FEATURE      0x04    /**<\brief Controls the settings for a particular
                                                 * communication feature of a particular target,
                                                 * setting the selected feature to its default state.*/
#define USB_CDC_SET_LINE_CODING         0x20    /**<\brief Allows the host to specify typical
                                                 * asynchronous line-character  formatting properties.*/
#define USB_CDC_GET_LINE_CODING         0x21    /**<\brief Allows the host to find out the currently
                                                 * configured line coding.*/
#define USB_CDC_SET_CONTROL_LINE_STATE  0x22    /**<\brief Generates RS-232/V.24 style control signals.*/
#define USB_CDC_SEND_BREAK              0x23    /**<\brief Sends special carrier modulation that
                                                 * generates an RS-232 style break.*/
/** @} */

/**\name Generic CDC specific notifications
 * @{ */
#define USB_CDC_NTF_NETWORK_CONNECTION  0x00    /**<\brief Allows the device to notify the host about
                                                 * network connection status.*/
#define USB_CDC_NTF_RESPONSE_AVAILABLE  0x01    /**<\brief Allows the device to notify the host that
                                                 * a response is available.*/
#define USB_CDC_NTF_SERIAL_STATE        0x20    /**<\brief Sends asynchronous notification of UART status.*/
#define USB_CDC_NTF_SPEED_CHANGE        0x2A    /**<\brief Allows the device to inform the host-networking
                                                 * driver that a change in either the uplink or the
                                                 * downlink bit rate of the connection has occurred.*/
/** @} */


/**\anchor USB_CDC_ACMGMNTCAP
 * \name USB CDC Abstract Control Management capabilities
 * @{ */
#define USB_CDC_COMM_FEATURE            0x01    /**<\brief Supports the request combination of
                                                 * Set_Comm_Feature, Clear_Comm_Feature, Get_Comm_Feature.*/
#define USB_CDC_CAP_LINE                0x02    /**<\brief Supports the request combination of
                                                 * Set_Line_Coding, Set_Control_Line_State,
                                                 * Get_Line_Coding, and the notification Serial_State.*/
#define USB_CDC_CAP_BRK                 0x04    /**<\brief Supports the request Send_Break.*/
#define USB_CDC_CAP_NOTIFY              0x08    /**<\brief Supports notification Network_Connection.*/
/** @} */

/**\anchor USB_CDC_CALLMGMTCAP
 * \name USB CDC Call Management capabilities
 * @{ */
#define USB_CDC_CALL_MGMT_CAP_CALL_MGMT 0x01    /**<\brief Device handles call management itself.*/
#define USB_CDC_CALL_MGMT_CAP_DATA_INTF 0x02    /**<\brief Device can send/receive call management
                                                 * information over a Data Class interface.*/
/** @} */

/**\anchor USB_CDC_LINECODE
 * \name Line coding structire bit fields
 * @{ */
#define USB_CDC_1_STOP_BITS             0x00    /**<\brief 1 stop bit.*/
#define USB_CDC_1_5_STOP_BITS           0x01    /**<\brief 1.5 stop bits.*/
#define USB_CDC_2_STOP_BITS             0x02    /**<\brief 2 stop bits.*/
#define USB_CDC_NO_PARITY               0x00    /**<\brief NO parity bit.*/
#define USB_CDC_ODD_PARITY              0x01    /**<\brief ODD parity bit.*/
#define USB_CDC_EVEN_PARITY             0x02    /**<\brief EVEN parity bit.*/
#define USB_CDC_MARK_PARITY             0x03    /**<\brief patity is MARK.*/
#define USB_CDC_SPACE_PARITY            0x04    /**<\brief patity is SPACE.*/
/** @} */

/**\name SERIAL_STATE notification data values
 * @{ */
#define USB_CDC_STATE_RX_CARRIER        0x0001 /**<\brief State of receiver carrier detection mechanism.
                                                * \details This signal corresponds to V.24 signal 109
                                                * and RS-232 DCD.*/
#define USB_CDC_STATE_TX_CARRIER        0x0002 /**<\brief State of transmission carrier.
                                                * \details This signal corresponds to V.24 signal 106
                                                * and RS-232 DSR.*/
#define USB_CDC_STATE_BREAK             0x0004 /**<\brief State of break detection mechanism of the device.*/
#define USB_CDC_STATE_RING              0x0008 /**<\brief State of ring signal detection of the device.*/
#define USB_CDC_STATE_FRAMING           0x0010 /**<\brief A framing error has occurred.*/
#define USB_CDC_STATE_PARITY            0x0020 /**<\brief A parity error has occurred.*/
#define USB_CDC_STATE_OVERRUN           0x0040 /**<\brief Received data has been discarded due to
                                                * overrun in the device.*/



/**\brief Line Coding Structure */
struct usb_cdc_line_coding {
    uint32_t    dwDTERate;          /**<\brief Data terminal rate, in bits per second.*/
    uint8_t     bCharFormat;        /**<\brief Stop bits.*/
    uint8_t     bParityType;        /**<\brief Parity.*/
    uint8_t     bDataBits;          /**<\brief Data bits (5,6,7,8 or 16).*/
} __attribute__ ((packed));


#endif /* _USB_CDC_H_ */



/**\addtogroup USBD_HW USB hardware driver
 * \brief Contains HW driver API
 * @{ */
/**\anchor USB_EVENTS
 * \name USB device events
 * @{ */
#define USB_EVENT_RESET      0   /* Reset.*/
#define usbd_evt_sof        1   /**<\brief Start of frame.*/
#define usbd_evt_susp       2   /**<\brief Suspend.*/
#define usbd_evt_wkup       3   /**<\brief Wakeup.*/
#define usbd_evt_eptx       4   /**<\brief Data packet transmitted*/
#define usbd_evt_eprx       5   /**<\brief Data packet received.*/
#define usbd_evt_epsetup    6   /**<\brief Setup packet received.*/
#define usbd_evt_error      7   /**<\brief Data error.*/
#define usbd_evt_count      8
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
/** @} */


//#if !defined(__ASSEMBLER__)
//#include <stdbool.h>



typedef struct _usbd_device usbd_device;

/**\brief Represents generic USB control request.*/
typedef struct {
    uint8_t     bmRequestType;  /**<\brief This bitmapped field identifies the characteristics of
                                 * the specific request.*/
    uint8_t     bRequest;       /**<\brief This field specifies the particular request.*/
    uint16_t    wValue;         /**<\brief It is used to pass a parameter to the device, specific to
                                 * the request.*/
    uint16_t    wIndex;         /**<\brief It is used to pass a parameter to the device, specific to
                                 * the request.*/
    uint16_t    wLength;        /**<\brief This field specifies the length of the data transferred
                                 * during the second phase of the control transfer.*/
    uint8_t     data[];         /**<\brief Data payload.*/
} usbd_ctlreq;

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
} usbd_status;

/**\brief Generic USB device event callback for events and endpoints processing
  * \param[in] dev pointer to USB device
  * \param event \ref USB_EVENTS "USB event"
  * \param ep active endpoint number
  * \note endpoints with same indexes i.e. 0x01 and 0x81 shares same callback.
  */
typedef void (*usbd_evt_callback)(usbd_device *dev, uint8_t event, uint8_t ep);

/**\brief USB control transfer completed callback function.
 * \param[in] dev pointer to USB device
 * \param[in] req pointer to usb request structure
 * \note usbd_device->complete_callback will be set to NULL after this callback completion.
 */
typedef void (*usbd_rqc_callback)(usbd_device *dev, usbd_ctlreq *req);

/**\brief Represents a USB device data.*/
struct _usbd_device {
//    const struct usbd_driver    *driver;                /**<\copybrief usbd_driver */
//    usbd_ctl_callback           control_callback;       /**<\copybrief usbd_ctl_callback */
    usbd_rqc_callback           complete_callback;      /**<\copybrief usbd_rqc_callback */
//    usbd_cfg_callback           config_callback;        /**<\copybrief usbd_cfg_callback */
//    usbd_dsc_callback           descriptor_callback;    /**<\copybrief usbd_dsc_callback */
    usbd_evt_callback           events[usbd_evt_count]; /**<\brief array of the event callbacks.*/
    usbd_evt_callback           endpoint[8];            /**<\brief array of the endpoint callbacks.*/
    usbd_status                 status;                 /**<\copybrief usbd_status */
};

/**\brief Initializes device structure
 * \param dev USB device that will be initialized
 * \param ep0size Control endpoint 0 size
 * \param buffer Pointer to control request data buffer (32-bit aligned)
 * \param bsize Size of the data buffer
 */
inline static void usbd_init(usbd_device *dev,
                             const uint8_t ep0size, uint32_t *buffer, const uint16_t bsize) {
//    dev->driver = drv;
    dev->status.ep0size = ep0size;
    dev->status.data_ptr = buffer;
    dev->status.data_buf = buffer;
    dev->status.data_maxsize = bsize - __builtin_offsetof(usbd_ctlreq, data);
}


/**\brief Register endpoint callback
 * \param dev dev usb device \ref _usbd_device
 * \param ep endpoint index
 * \param callback pointer to user \ref usbd_evt_callback callback for endpoint events
 */
inline static void usbd_reg_endpoint(usbd_device *dev, uint8_t ep, usbd_evt_callback callback) {
    dev->endpoint[ep & 0x07] = callback;
}






/**\addtogroup USB_STD USB Standard
 * \brief This module contains generic USB device framework definitions
 * \details This module based on
 * + Chapter 9 of the [Universal Serial Bus Specification Revision 2.0]
 * (https://www.usb.org/sites/default/files/usb_20_20181221.zip)
 * + [LUFA - the Lightweight USB Framework for AVRs.](https://github.com/abcminiuser/lufa)
 * @{ */



/** Macro to create \ref usb_string_descriptor from array */
#define USB_ARRAY_DESC(...)        {.bLength = 2 + sizeof((uint16_t[]){__VA_ARGS__}),\
                                    .bDescriptorType = USB_DTYPE_STRING,\
                                    .wString = {__VA_ARGS__}}
/** Macro to create \ref usb_string_descriptor from string */
//#define USB_STRING_DESC(s)         {.bLength = sizeof(CAT(u,s)), .bDescriptorType = USB_DTYPE_STRING, .wString = {CAT(u,s)}}

/**\brief Macro to set мaximum power consumption field for the \ref usb_config_descriptor */
//#define USB_CFG_POWER_MA(mA)        ((mA) >> 1)
/** @} */

/**\name USB device configuration definitions
 * @{ */
//#define USB_CFG_ATTR_RESERVED       0x80
//#define USB_CFG_ATTR_SELFPOWERED    0x40
/** @} */

/** \anchor USB_ENDPOINT_DEF
 *  \name USB endpoint attributes definitions
 * @{ */
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
#define NO_DESCRIPTOR               0x00    /**<\brief String descriptor doesn't exists in the device.*/
#define INTSERIALNO_DESCRIPTOR      0xFE    /**<\brief String descriptor is an internal serial number
                                             * provided by hardware driver.*/
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
/** @} */

/**\name USB Feature selector
 * @{ */
#define USB_FEAT_ENDPOINT_HALT      0x00    /**<\brief Halt endpoint.*/
#define USB_FEAT_REMOTE_WKUP        0x01
#define USB_FEAT_TEST_MODE          0x02
#define USB_FEAT_DEBUG_MODE         0x06
/** @} */

/**\name USB Test mode Selectors
 * @{ */
#define USB_TEST_J                  0x01    /**<\brief Test J.*/
#define USB_TEST_K                  0x02    /**<\brief Test K.*/
#define USB_TEST_SE0_NAK            0x03    /**<\brief Test SE0 NAK.*/
#define USB_TEST_PACKET             0x04    /**<\brief Test Pcaket.*/
#define USB_TEST_FORCE_ENABLE       0x05    /**<\brief Test Force Enable.*/
/** @} */




/**\brief USB string descriptor
 * \details String descriptors are referenced by their one-based index number. A string descriptor
 * contains one or more not NULL-terminated Unicode strings.
 * \note String descriptors are optional. if a device does not support string descriptors, all
 * references to string descriptors within device, configuration, and interface descriptors must be
 * reset to zero.*/
struct usb_string_descriptor {
    uint8_t  bLength;               /**<\brief Size of the descriptor, in bytes.*/
    uint8_t  bDescriptorType;       /**<\brief String descriptor type.*/
    uint16_t wString[];             /**<\brief String data, as unicode characters or array of
                                     * \ref USB_STD_LANGID codes. */
} __attribute__((packed));



/** @} */

#if defined (__cplusplus)
}
#endif
extern usbd_device udev;
void usbd_process_ep0 (usbd_device *dev, uint8_t event, uint8_t ep);
void cdc_txonly(usbd_device *dev, uint8_t event, uint8_t ep);

extern const uint8_t StringLangID[];
extern const uint8_t StringVendor[];
extern const uint8_t StringProduct[];
extern const uint8_t USB_Descriptor[];
extern const uint8_t USB_ConfigDescriptor[];
