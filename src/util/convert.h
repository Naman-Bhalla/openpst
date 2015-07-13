/**
* LICENSE PLACEHOLDER
*
* @file convert.h
* @package OpenPST
* @brief hexdump helper functions
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/

#ifndef _UTIL_CONVERT_H
#define _UTIL_CONVERT_H

#include "include/definitions.h"
#include "util/hexdump.h"
#include <iostream>
#include <stdio.h>

std::string bytesToHex(unsigned char* input, int size, bool byteswap);
std::string hexToString(char *input, int length);

#endif
