/**
* LICENSE PLACEHOLDER
*
* @file hdlc.cpp
* @package OpenPST
* @brief HDLC helper functions
*
* @author Gassan Idriss <ghassani@gmail.com>
*/
#include "hdlc.h"

int hdlc_request(uint8_t* in, size_t inSize, uint8_t** out, size_t &outSize) {

	uint16_t crc = crc = crc16((const char*)in, inSize); // perform the crc or the original data

	outSize = inSize + HDLC_OVERHEAD_LENGTH;
	for (int i = 0; i < inSize; i++) {
		if (in[i] == HDLC_CONTROL_CHAR || in[i] == HDLC_ESC_CHAR) {
			outSize += 1;
		}
	}

	uint8_t* buffer = new uint8_t[outSize];

	buffer[0] = HDLC_CONTROL_CHAR;

	int o = 1;
	for (int i = 0; i < inSize; i++) {
		if (in[i] == HDLC_CONTROL_CHAR || in[i] == HDLC_ESC_CHAR) {
			buffer[o] = HDLC_ESC_CHAR;
			buffer[++o] = (in[i] == HDLC_ESC_CHAR ? HDLC_ESC_CHAR : HDLC_CONTROL_CHAR) ^ HDLC_ESC_MASK;
		}
		else {
			buffer[o] = in[i];
		}
		o++;
	}


	buffer[o] = crc & 0xFF; // add first byte of crc
	buffer[++o] = (crc >> 8) & 0xFF; // add second byte of crc
	buffer[++o] = HDLC_CONTROL_CHAR; // Add out ending control character

	*out = buffer;

	return 1;
}

int  hdlc_response(uint8_t* in, size_t inSize, uint8_t** out, size_t &outSize) {
	uint16_t crc = crc16((const char*)&in[1], inSize - HDLC_OVERHEAD_LENGTH);
	uint16_t chk = *((uint16_t*)&in[inSize - HDLC_TRAILER_LENGTH]);
	if (crc != chk) {
		printf("Invalid Response CRC Expected: %04X - Received: %04X\n", crc, chk);
	}

	outSize = inSize;
	uint8_t* buffer = new uint8_t[outSize];

	int o = 0;
	for (int i = 0; i < inSize; i++) {
		if (in[i] == HDLC_ESC_CHAR) {
			buffer[o] = in[i + 1] ^ HDLC_ESC_MASK;
			i++;
		}
		else if (in[i] == HDLC_CONTROL_CHAR) {
			continue;
		}
		else {
			buffer[o] = in[i];
		}

		o++;
	}

	outSize = o - 2; // less crc
	*out = buffer;
	return 1;
}



int hdlc_request(std::vector<uint8_t> &data) {

	uint16_t crc = crc16((const char*)&data[0], data.size()); // perform the crc or the original data
	data.push_back(crc & 0xFF);
	data.push_back((crc >> 8) & 0xFF);

	int count = data.size();
	for (int i = 0; i < count; i++) {
		if (data[i] == HDLC_CONTROL_CHAR) {
			data[i] = HDLC_ESC_CHAR;
			data.insert(data.begin() + i + 1, HDLC_CONTROL_CHAR ^ HDLC_ESC_MASK);
			count++;
		}
		else if (data[i] == HDLC_ESC_CHAR) {
			data[i] = HDLC_ESC_CHAR;
			data.insert(data.begin() + i + 1, HDLC_ESC_CHAR ^ HDLC_ESC_MASK);
			count++;
		}
	}

	data.insert(data.begin(), HDLC_CONTROL_CHAR);
	data.push_back(HDLC_CONTROL_CHAR);

	return 0;
}

int hdlc_response(std::vector<uint8_t> &data) {
	// unescape the data first
	int count = data.size();
	for (int i = 0; i < count; i++) {
		if (data[i] == HDLC_ESC_CHAR) {
			data[i] = data[i + 1] ^ HDLC_ESC_MASK;
			data.erase(data.begin() + i + 1);
			count--;
		}
	}

	// check the crc
	if (data[0] == HDLC_CONTROL_CHAR) {
		data.erase(data.begin());
	}

	uint16_t crc = crc16((const char*)&data[0], data.size() - HDLC_TRAILER_LENGTH);
	uint16_t chk = *((uint16_t*)&data[data.size() - HDLC_TRAILER_LENGTH]);

	if (crc != chk) {
		printf("Invalid Response CRC Expected: %04X - Received: %04X\n", crc, chk);
	}

	data.erase(data.end() - 3, data.end());

	return 0;
}