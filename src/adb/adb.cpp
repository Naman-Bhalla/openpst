/**
* LICENSE PLACEHOLDER
*
* @file hdlc.cpp
* @package OpenPST
* @brief HDLC helper functions
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/
#include "adb.h"

namespace OpenPST
{

	vector<AdbDevice> adbListDevices()
	{
		list<string> output;
		vector<AdbDevice> result;

		AdbCommand adbCommand = {};
		adbCommand.command = "devices";
		adbCommand.type = kAdbCommandTypeDefault;

		output = adbSendCommand(adbCommand);

		for (string line : output) {
			if (line.length() > 2) {
				if (line.find("List of devices attached") != string::npos) {

				}
				else {
					AdbDevice device = {};
					string delimiter = "\t";
					size_t position = 0;

					while ((position = line.find(delimiter)) != string::npos) {
						if (!device.serial.length()) {
							device.serial = line.substr(0, position);
							line.erase(0, position + delimiter.length());
						}
					}

					device.state = line;

					AdbCommand adbCommand = {};
					adbCommand.command = "getprop ro.product.model";
					adbCommand.type = kAdbCommandTypeShell;

					list<string> output = adbSendCommand(adbCommand, device.serial);

					for (string line : output) {
						if (line.length()) {
							device.model = line;
						}
					}

					result.push_back(device);
				}
			}
		}

		return result;
	}

	void adbStartServer()
	{
		adbSendCommandNoReturn("start-server");
	}

	void adbKillServer()
	{
		adbSendCommandNoReturn("kill-server");
	}

	void adbSendCommandNoReturn(string command)
	{
		string exec = getCurrentWorkingDir() + "/files/adb " + command;

		FILE *adbCommand_p = _popen(exec.c_str(), "r");

		if (!adbCommand_p)
		{
			return;
		}

		_pclose(adbCommand_p);
	}

	list<string> adbSendCommand(AdbCommand command)
	{
		return adbSendCommand(command, "");
	}

	list<string> adbSendCommand(AdbCommand command, string serial)
	{
		string cmd, exec;

		switch (command.type) {
		case kAdbCommandTypeDefault:
			cmd = command.command;
			break;
		case kAdbCommandTypeShell:
			cmd = "shell \"" + command.command + "\"";
			break;
		case kAdbCommandTypeRootShell:
			cmd = "shell \"su -c '" + command.command + "'\"";
			break;
		}

		if (serial.length()) {
			exec = getCurrentWorkingDir() + "/files/adb -s " + serial + " " + cmd;
		}
		else {
			exec = getCurrentWorkingDir() + "/files/adb " + cmd;
		}

		printf("adb %s\n", command.command.c_str());

		list<string> result;

		FILE *adbCommand_p = _popen(exec.c_str(), "r");

		if (!adbCommand_p)
		{
			return result;
		}

		char buffer[2048];

		while (fgets(buffer, sizeof(buffer), adbCommand_p)) {
			printf(buffer);
			result.push_back(buffer);
		}

		_pclose(adbCommand_p);
		return result;
	}

	string getCurrentWorkingDir()
	{
		char cCurrentPath[256];

		if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		{
			return "";
		}

		return cCurrentPath;
	}

}