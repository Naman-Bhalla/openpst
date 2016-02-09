/**
* LICENSE PLACEHOLDER
*
* @file qcdm_nv_item_read_worker.cpp
* @class QcdmNvItemReadWorker
* @package OpenPST
* @brief Handles background processing of NV item reading
*
* @author Gassan Idriss <ghassani@gmail.com>
*/

#include "adb_command_worker.h"

using namespace OpenPST;

AdbCommandWorker::AdbCommandWorker(AdbDevice& device, AdbCommandWorkerRequest request, QObject *parent) :
device(device),
request(request),
QThread(parent)
{

}

AdbCommandWorker::~AdbCommandWorker()
{

}

void AdbCommandWorker::run()
{
	for (AdbCommand command : request.commands) {
		adbCommand(command, request.serial);
	}

	emit complete(request);
}

void AdbCommandWorker::adbCommand(AdbCommand command, QString serial)
{
	
	list<string> output = OpenPST::adbCommand(command, serial.toStdString());

	AdbCommandWorkerResponse response = {};
	response.task = request.task;
	response.serial = request.serial;
	response.command = QString::fromStdString(command.command);

	for (std::string line : output) {
		if (line.length()) {
			printf(line.c_str());
			response.output.append(QString::fromStdString(line).simplified());
		}
	}

	emit update(response);
}


