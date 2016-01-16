/**
* LICENSE PLACEHOLDER
*
* @file laf_serial.h
* @class LafSerial
* @package OpenPST
* @brief LAF serial port implementation
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/

#ifndef _SERIAL_LAF_SERIAL_H
#define _SERIAL_LAF_SERIAL_H

#include "include/definitions.h"
#include "../../extra/lafshell/laf.h"
#include "serial/serial.h"
#include "util/hexdump.h"

namespace OpenPST {
	class LafSerial : public serial::Serial {
	public:
		/**
		* @brief LafSerial
		*
		* @param std::string port
		* @param int baudrate
		* @param serial::Timeout - Timeout, defaults to 1000ms
		*/
		LafSerial(std::string port, int baudrate, serial::Timeout timeout = serial::Timeout::simpleTimeout(1000));

		/**
		* @brief ~LafSerial
		*/
		~LafSerial();

		/**
		* @brief sendCommand
		*
		* @return response - Command Response String
		*/
		std::string sendCommand(std::string command);

	private:

	};
}

#endif /* _SERIAL_LAF_SERIAL_H */