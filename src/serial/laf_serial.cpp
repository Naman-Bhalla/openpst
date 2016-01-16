/**
* LICENSE PLACEHOLDER
*
* @file laf_serial.cpp
* @class LafSerial
* @package OpenPST
* @brief HDLC serial port implementation
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/

#include "laf_serial.h"
#include "qc/hdlc.h"

#define DEBUG
#define HEXDUMP_PORT_TX
#define HEXDUMP_PORT_RX
#define BUFFER_SIZE (LAF_MAX_DATA_SIZE + sizeof(LafCmdHeader))

using namespace OpenPST;

uint8_t enable_laf[] = { 0x3A, 0xA1, 0x6E, 0x7E };
uint8_t buffer[BUFFER_SIZE] = {};

/**
* @brief LafSerial::LafSerial
*
* @param std::string port
* @param int baudrate
* @param serial::Timeout - Timeout, defaults to 1000ms
*/
LafSerial::LafSerial(std::string port, int baudrate, serial::Timeout timeout) :
    serial::Serial(port, baudrate, timeout)
{

}

/**
* @brief LafSerial::~LafSerial
*/
LafSerial::~LafSerial()
{

}

/**
* @brief LafSerial::sendCommand(std::string command)
*/
std::string LafSerial::sendCommand(std::string command) {
	if (command.compare("ENTER") == 0) {
		write(enable_laf, sizeof(enable_laf));
		return "";
	} else if (command.compare("LEAVE") == 0) {
		// TODO: 
	} else if (command.compare("SPECIAL") == 0) {
		LafCmdHeader header = {};
		header.command = LAF_CMD_INFO;
		header.magic = 0x000;
		header.arg1 = 0xB08;
		header.arg_opt0 = LAF_CMD_INFO_SPRO;
		write((uint8_t*)&header, sizeof(enable_laf));
		return "";
	}

	std::stringstream lafCmdResponse;

	size_t responseSize;

	LafCmdHeader header = {};
	header.command = LAF_CMD_EXECUTE;
	header.magic = LAF_EXEC_MAGIC;

	LafCmd* packet = (LafCmd*)buffer;
	packet->header = header;

	if (command.size()) {
		memcpy(packet->data, command.c_str(), command.size());
		memset(packet->data + command.size(), 0x00, 1);
		packet->header.size = command.size() + 1;
		packet->header.crc = 0x000000;

		uint16_t crc = crc16((char*)packet, (sizeof(header) + command.size() + 1));
		packet->header.crc = crc;

		write((uint8_t*)packet, (sizeof(header) + command.size() + 1));

		if (!(responseSize = read(buffer, BUFFER_SIZE))) {
			return "No Response";
		}

		LafCmd* response = (LafCmd*)buffer;

		if (response->header.command == LAF_CMD_FAIL) {
			return "Command Failed";
		} else {
			for (int i = 0; i < response->header.size; i++) {
				lafCmdResponse << response->data[i];
			}
		}
	}

	return lafCmdResponse.str();
}
