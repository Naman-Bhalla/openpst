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

#include "serial_at_command_worker.h"

using namespace OpenPST;

SerialAtCommandWorker::SerialAtCommandWorker(Serial& port, SerialAtCommandWorkerRequest request, QObject *parent) :
	port(port),
    request(request),
    QThread(parent)
{

}

SerialAtCommandWorker::~SerialAtCommandWorker()
{

}

void SerialAtCommandWorker::run()
{
	for (QString command : request.commands) {
		sendAtCommand(command);
	}

	emit complete(request);
}

void SerialAtCommandWorker::sendAtCommand(QString command)
{
	port.write(command.toStdString() + "\r\n");

	if (command.contains("VERSNAME")) {
		msleep(500);
	}
	
	SerialAtCommandWorkerResponse response = {};
	response.task = request.task;
	response.command = command;

	for (std::string line : port.readlines()) {
		if (line.length() && QString::fromStdString(line) != command + "\r\n") {
			if (command.contains("DEVROOTK")) {
				printf(line.c_str());
				response.output.append(QString::fromStdString(line).simplified());
			}
			else if (line != "OK\r\n") {
				printf(line.c_str());
				response.output.append(QString::fromStdString(line).simplified());
			}
		}
	}

	emit update(response);
}
