/**
* LICENSE PLACEHOLDER
*
* @file adb.h
* @package OpenPST
* @brief ADB helper functions definitions
*
* @author Gassan Idriss <ghassani@gmail.com>
*/

#ifndef _ADB_H
#define _ADB_H

#include "include/definitions.h"
#include <iostream>
#include <list>
#include <stdio.h>
#include <string.h>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#define DEBUG

using namespace std;

namespace OpenPST
{

	struct AdbDevice
	{
		string serial;
		string model;
		string state;
	};

	struct AdbCommand
	{
		string command;
		int type;
	};

	enum AdbCommandType
	{
		kAdbCommandTypeDefault = 0,
		kAdbCommandTypeShell = 1,
		kAdbCommandTypeRootShell = 2
	};

	void adbStartServer();
	void adbKillServer();

	vector<AdbDevice> adbListDevices();

	void adbSendCommandNoReturn(string command);
	list<string> adbSendCommand(AdbCommand command);
	list<string> adbSendCommand(AdbCommand command, string serial);

	bool adbInstallApk(string path);
	bool adbPushFile(string path, string dest);
	bool adbPullFile(string path, string dest);

	string getCurrentWorkingDir();

}

#endif // _ADB_H
