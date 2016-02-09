/**
* LICENSE PLACEHOLDER
*
* @file qcdm_nv_item_read_worker.h
* @class QcdmNvItemReadWorker
* @package OpenPST
* @brief Handles background processing of NV item reading
*
* @author Gassan Idriss <ghassani@gmail.com>
*/
#ifndef _WORKER_SERIAL_AT_COMMAND_WORKER_H
#define _WORKER_SERIAL_AT_COMMAND_WORKER_H

#include <QThread>
#include "serial/serial.h"
#include <iostream>
#include <fstream>

using namespace serial;

namespace OpenPST {

	struct SerialAtCommandWorkerRequest
	{
		int task;
		QStringList commands;
	};

	struct SerialAtCommandWorkerResponse
	{
		int task;
		QString command;
		QString output;
	};

    class SerialAtCommandWorker : public QThread
    {
		Q_OBJECT

		public:
			SerialAtCommandWorker(Serial& port, SerialAtCommandWorkerRequest request, QObject *parent = 0);
			~SerialAtCommandWorker();
		protected:
			Serial&  port;
			SerialAtCommandWorkerRequest request;

			void run() Q_DECL_OVERRIDE;
		private:
			void sendAtCommand(QString command);
		signals:
			void update(SerialAtCommandWorkerResponse request);
			void complete(SerialAtCommandWorkerRequest request);
			void error(QString msg);
    };
}

#endif // _WORKER_QCDM_NV_ITEM_READ_WORKER_H