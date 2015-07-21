/**
* LICENSE PLACEHOLDER
*
* @file hdlc_serial.cpp
* @class HdlcSerial
* @package OpenPST
* @brief HDLC serial port implementation
*
* @author Gassan Idriss <ghassani@gmail.com>
*/

#include "hdlc_serial.h"

using namespace openpst;

/**
 * @brief HdlcSerial::HdlcSerial
 * @param port
 * @param baudrate
 */
HdlcSerial::HdlcSerial(std::string port, int baudrate) :
    serial::Serial (port, baudrate, serial::Timeout::simpleTimeout(1000))
{

}

/**
 * @brief QcdmSerial::~QcdmSerial
 */
HdlcSerial::~HdlcSerial()
{

}

/**
 * Escapes the data and creates a CRC'ed HDLC packet
 * then writes the data
 *
 * @super Serial::write (uint8_t *data, size_t size);
 *
 * @brief write
 * @param data
 * @param size
 * @param bool encapsulate
 * @return
 */
size_t HdlcSerial::write (uint8_t *data, size_t size, bool encapsulate)
{
    if (!encapsulate) {
        size_t bytesWritten = Serial::write(data, size);
		if (bytesWritten) hexdump_tx(&data[0], bytesWritten);
		return bytesWritten;
    }

    size_t packetSize = 0;
    uint8_t* packet   = NULL;

    hdlc_request(data, size, &packet, packetSize);

    size_t bytesWritten = Serial::write(packet, packetSize);
    
    hexdump_tx(packet, bytesWritten);

    if (packet != NULL) {
        free(packet);
    }

    return bytesWritten;    
}

/**
 * Reads and unescpaes the CRC'ed HDLC packet
 * read from the device
 *
 * @super Serial::read (uint8_t *buffer, size_t size);
 *
 * @brief read
 * @param uint8_t* buf
 * @param size_t size
 * @param bool unescape
 * @return
 */
size_t HdlcSerial::read (uint8_t *buf, size_t size, bool unescape )
{
    size_t bytesRead = Serial::read(buf, size);

    if (!unescape || !bytesRead) {
		if (bytesRead) hexdump_rx(&buf[0], bytesRead);
        return bytesRead;
    }

    size_t dataSize = 0;
    uint8_t* data = nullptr;

	hdlc_response(buf, bytesRead, &data, dataSize);

	memcpy(buf, data, dataSize);

	hexdump_rx(buf, dataSize);

	if (data != nullptr) {
		free(data);
	}

	return dataSize;
}

size_t HdlcSerial::write(std::vector<uint8_t> &data, bool encapsulate)
{
	if (!encapsulate) {
		size_t bytesWritten = Serial::write(data);
		if (bytesWritten) hexdump_tx(&data[0], bytesWritten);
		return bytesWritten;
	}

	hdlc_request(data);

	size_t bytesWritten = Serial::write(data);

	hexdump_rx(&data[0], bytesWritten);

	return bytesWritten;
}

size_t HdlcSerial::read(std::vector<uint8_t> &buffer, size_t size, bool unescape)
{

	buffer.reserve(buffer.size() + (size + HDLC_OVERHEAD_LENGTH));
	
	size_t bytesRead = Serial::read(buffer, size + HDLC_OVERHEAD_LENGTH);

	if (!unescape || !bytesRead) {
		if (bytesRead) hexdump_rx(&buffer[0], bytesRead);
		return bytesRead;
	}

	hdlc_response(buffer);

	hexdump_rx(&buffer[0], buffer.size());

	return buffer.size();
}
