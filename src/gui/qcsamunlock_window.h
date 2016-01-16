/**
* LICENSE PLACEHOLDER
*
* @file qcdm_window.h
* @class QcdmWindow
* @package OpenPST
* @brief QCDM GUI interface class definitions
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/

#ifndef _GUI_QCSAMUNLOCK_WINDOW_H
#define _GUI_QCSAMUNLOCK_WINDOW_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <QMainWindow>
#include <QVariant>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QClipboard>
#include <QListWidget>
#include <QtXml>
#include "ui_qcsamunlock_window.h"
#include "serial/laf_serial.h"
#include "serial/qcdm_serial.h"
#include "include/definitions.h"
#include "util/sleep.h"
#include "qc/dm.h"
#include "qc/dm_efs_manager.h"
#include "about_dialog.h"
#include "util/convert.h"

namespace Ui {
	class QcSamUnlockWindow;
}

namespace OpenPST {

	class QcSamUnlockWindow : public QMainWindow
	{
		Q_OBJECT

		enum PortType
		{
			kPortTypeLaf = 0,
			kPortTypeQcdm = 1
		};

		enum LogType
		{
			kLogTypeError = 1,
			kLogTypeWarning = 2,
			kLogTypeInfo = 3,
			kLogTypeDebug = 4
		};

		enum SpcReadType
		{
			kSpcReadTypeNv = 0,
			kSpcReadTypeEfs = 1,
			kSpcReadTypeEfsForceWrite = 2,
			kSpcReadTypeHtc = 3,
			kSpcReadTypeLg = 4,
			kSpcReadTypeSubsys = 5
		};

		enum ImeiMeidMethod
		{
			kImeiMeidMethodNv = 0,
			kImeiMeidMethodEfs = 1,
			kImeiMeidMethodEfsForceWrite = 2,
			kImeiMeidMethodSubsys = 3
		};

		enum SamsungSimUnlockType
		{
			kSamsungSimUnlockTypeEfs    = 0,
			kSamsungSimUnlockTypeNvData = 1
		};

	public:
		Ui::QcSamUnlockWindow* ui;
		LafSerial lafPort;
		QcdmSerial qcdmPort;
		serial::PortInfo currentPort;
		DmEfsManager efsManager;

		/**
		* @brief
		*/
		explicit QcSamUnlockWindow(QWidget *parent = 0);

		/**
		* @brief
		*/
		~QcSamUnlockWindow();

	public slots:
		/**
		* @brief
		*/
		void updatePortList();

		/**
		* @brief
		*/
		void connectPort(int portType);

		/**
		* @brief
		*/
		void disconnectPort(int portType);

		/**
		* @brief
		*/
		void clearLog();

		/**
		* @brief
		*/
		void saveLog(QString task);

	private:
		QcdmEfsStatfsResponse        statResponse;
		QcdmEfsSyncResponse          syncResponse;
		QcdmEfsGetSyncStatusResponse syncStatusResponse;
		int syncMaxRetries = 10;
		int syncRetries = 0;
		int sequence = 0;

		/**
		* @brief
		*/
		QString readSpcNv();

		/**
		* @brief
		*/
		QString readSpcEfs();

		/**
		* @brief
		*/
		bool processItem(int item, int sequence);

		/**
		* @brief
		*/
		void samsungEfsSimUnlock();

		/**
		* @brief
		*/
		bool testSecurity();

		/**
		* @brief
		*/
		void log(int type, QString message);

		/**
		* @brief
		*/
		void logAddEmptyLine();

	private slots:

		/**
		* @brief
		*/
		void qualcommReadAll();

		/**
		* @brief
		*/
		void readImei();

		/**
		* @brief
		*/
		void qualcommReadImei();

		/**
		* @brief
		*/
		void writeImei();

		/**
		* @brief
		*/
		void readMeid();

		/**
		* @brief
		*/
		void qualcommReadMeid();

		/**
		* @brief
		*/
		void writeMeid();

		/**
		* @brief
		*/
		void readSpc();

		/**
		* @brief
		*/
		void qualcommReadSpc();

		/**
		* @brief
		*/
		void writeSpc();

		/**
		* @brief
		*/
		void readSubscription();

		/**
		* @brief
		*/
		void qualcommReadSubscription();

		/**
		* @brief
		*/
		void writeSubscription();

		/**
		* @brief
		*/
		void samsungSimUnlock();

		/**
		* @brief
		*/
		void lgSprintSimUnlock();

		/**
		* @brief
		*/
		void lgRemovePasscode();

		/**
		* @brief
		*/
		void lgRemoveFrp();
	};

}
#endif // _GUI_QCSAMUNLOCK_WINDOW_H
