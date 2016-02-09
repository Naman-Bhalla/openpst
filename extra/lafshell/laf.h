#ifndef _LAF_H
#define _LAF_H

#include "include/definitions.h"

#define LAF_EXEC_MAGIC 0xBCBAA7BA
#define LAF_CTRL_MAGIC 0xB3ADABBC
#define LAF_INFO_MAGIC 0xB0B9B1B6

#define LAF_INFO_SIZE 0x00000B08

#define LAF_MAX_DATA_SIZE 1048578

#define LAF_ENTER 0x3AA16E7E

#define LAF_CMD_WRITE   0x45545257 // WRITE
#define LAF_CMD_READ    0x44414552 // READ
#define LAF_CMD_IOCT    0x54434F49 // IOCT
#define LAF_CMD_OPEN    0x4E45504F // OPEN
#define LAF_CMD_CLSE    0x45534C43 // CLOSE
#define LAF_CMD_UNLINK  0x4B4C4E55 // UNLINK
#define LAF_CMD_ERASE   0x45535245 // ERASE
#define LAF_CMD_CONTROL 0x4C525443 // CONTROL
#define LAF_CMD_HELO    0x4F4C4548 // HELLO
#define LAF_CMD_EXECUTE 0x43455845 // EXECUTE
#define LAF_CMD_RSVD    0x44565352 // RSVD
#define LAF_CMD_FAIL    0x4C494146 // FAIL
#define LAF_CMD_INFO    0x4F464E49 // INFO
#define LAF_CMD_MISC    0x4353494D // MISC
#define LAF_CMD_KILO    0x4F4C494B // KILO

#define LAF_CMD_INFO_GPRO 0x4F525047 // GPRO
#define LAF_CMD_INFO_SPRO 0x4F525053 // SPRO

#define LAF_CMD_CONTROL_POWER_OFF 0x46464F50 // POWER_OFF
#define LAF_CMD_CONTROL_RESET     0x54455352 // RESET
#define LAF_CMD_CONTROL_ONRS      0x53524E4F // ONRS
#define LAF_CMD_CONTROL_RSON      0x4E4F5352 // RSON

#define LAF_CMD_KILO_METR 0x5254454D // METR
#define LAF_CMD_KILO_CENT 0x544E4543 // CENT

#define LAF_MISC_PARITION_OFFSET(number) \ number << 9

enum LafProperties
{
	LAF_PROP_AP_FACTORY_RESET_STATUS    = 0x4F0C,
	LAF_PROP_BATTERY_LEVEL              = 0x420B,
	LAF_PROP_BOOTLOADER_VERSION         = 0x390F,
	LAF_PROP_CHIPSET_PLATFORM           = 0x430A,
	LAF_PROP_CP_FACTORY_RESET_STATUS    = 0x5000,
	LAF_PROP_CUPSS_SWFV                 = 0x6208, // ?
	LAF_PROP_DEVICE_BUILD_TYPE          = 0x4300,
	LAF_PROP_DEVICE_FACTORY_VERSION     = 0x240F,
	LAF_PROP_DEVICE_FACTORY_OUT_VERSION = 0x5208,
	LAF_PROP_DEVICE_SW_VERSION          = 0x140F,
	LAF_PROP_DOWNLOAD_CABLE             = 0x3F09,
	LAF_PROP_DOWNLOAD_SPEED             = 0x0201,
	LAF_PROP_DOWNLOAD_SW_VERSION        = 0x0209,
	LAF_PROP_DOWNLOAD_TYPE              = 0x0100,
	LAF_PROP_HARDWARE_REVISION          = 0x4107,
	LAF_PROP_IMEI                       = 0x3C7,
	LAF_PROP_IS_DOWNLOAD_NOT_FINISHED   = 0x5004,
	LAF_PROP_IS_ONE_BINARY_DUAL_PLAN    = 0x7208, // ?
	LAF_PROP_LAF_SW_VERSION             = 0x4E08,
	LAF_PROP_MEMORY_ID                  = 0x7300,
	LAF_PROP_MEMORY_SIZE                = 0x720C,
	LAF_PROP_MODEL_NAME                 = 0x1301,
	LAF_PROP_PID                        = 0x3D0B,
	LAF_PROP_QEM                        = 0x5008,
	LAF_PROP_SECURE_DEVICE              = 0x420F,
	LAF_PROP_TARGET_COUNTRY             = 0x4602,
	LAF_PROP_TARGET_OPERATOR            = 0x440E,
	LAF_PROP_USB_VERSION                = 0x4003,
};

/**
* This functionality is part of the laf kernel image in the / sbin / lafd binary file
* 
* [27.355346 / 03 - 31 05:04 : 58.419][LAF] laf_message.command = 0x43455845(EXEC)
* [27.355398 / 03 - 31 05:04 : 58.419][LAF] laf_message.arg0 = 0x0
* [27.355438 / 03 - 31 05:04 : 58.419][LAF] laf_message.arg1 = 0x0
* [27.355477 / 03 - 31 05:04 : 58.419][LAF] laf_message.arg_opt0 = 0x0
* [27.355517 / 03 - 31 05:04 : 58.419][LAF] laf_message.arg_opt1 = 0x0
* [27.355557 / 03 - 31 05:04 : 58.419][LAF] laf_message.data_length = 0x5
* [27.355597 / 03 - 31 05:04 : 58.419][LAF] laf_message.data_check = 0xdd13
* [27.355636 / 03 - 31 05:04 : 58.419][LAF] laf_message.magic = 0xbcbaa7ba
*/

PACKED(typedef struct LafCmdHeader{
	uint32_t command;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg_opt0;
	uint32_t arg_opt1;
	uint32_t size;
	uint32_t crc;
	uint32_t magic;
}) LafCmdHeader;

PACKED(typedef struct LafCmd{
	LafCmdHeader header;
	uint8_t data[2]; // must trail with 2 null bytes and requires at least 2 null bytes if no data
}) LafCmd;
#endif // _LAF_H