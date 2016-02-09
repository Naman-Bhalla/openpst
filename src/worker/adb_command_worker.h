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
#ifndef _WORKER_ADB_COMMAND_WORKER_H
#define _WORKER_ADB_COMMAND_WORKER_H

#include <QThread>
#include "adb/adb.h"
#include <iostream>
#include <fstream>

namespace OpenPST
{
	struct AdbCommandWorkerRequest
	{
		int task;
		QString serial;
		vector<AdbCommand> commands;
	};

	struct AdbCommandWorkerResponse
	{
		int task;
		QString serial;
		QString command;
		QString output;
	};

	class AdbCommandWorker : public QThread
	{
		Q_OBJECT

	public:
		AdbCommandWorker(AdbDevice& device, AdbCommandWorkerRequest request, QObject *parent = 0);
		~AdbCommandWorker();
	protected:
		AdbDevice&  device;
		AdbCommandWorkerRequest request;

		void run() Q_DECL_OVERRIDE;
	private:
		void adbCommand(AdbCommand command, QString serial);
	signals:
		void update(AdbCommandWorkerResponse request);
		void complete(AdbCommandWorkerRequest request);
		void error(QString msg);
	};
}

#endif // _WORKER_QCDM_NV_ITEM_READ_WORKER_H