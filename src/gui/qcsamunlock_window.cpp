 /**
* LICENSE PLACEHOLDER
*
* @file qcsamunlock_window.cpp
* @class QcSamUnlockWindow
* @package OpenPST
* @brief QCSamUnlock GUI interface class
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/

#include "qcsamunlock_window.h"

using namespace std;
using namespace OpenPST;

QcSamUnlockWindow::QcSamUnlockWindow(QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::QcSamUnlockWindow),
	lafPort("", 115200, serial::Timeout::simpleTimeout(500)),
	qcdmPort("", 115200, serial::Timeout::simpleTimeout(500)),
	efsManager(qcdmPort)
{
    ui->setupUi(this);

    updatePortList();

	ui->subscriptionValue->addItem("", -1);
	ui->subscriptionValue->addItem("RUIM ONLY", kQcdmRuimConfigTypeRuim);
	ui->subscriptionValue->addItem("NV ONLY", kQcdmRuimConfigTypeNv);
	ui->subscriptionValue->addItem("RUIM PREFERRED", kQcdmRuimConfigTypePref);
	ui->subscriptionValue->addItem("GSM 1X", kQcdmRuimConfigTypeGsm1x);

    QObject::connect(ui->updatePortListButton, SIGNAL(clicked()), this, SLOT(updatePortList()));

	// Qualcomm
	QObject::connect(ui->qualcommReadAllButton, SIGNAL(clicked()), this, SLOT(qualcommReadAll()));
	QObject::connect(ui->readSpcButton, SIGNAL(clicked()), this, SLOT(qualcommReadSpc()));
	QObject::connect(ui->writeSpcButton, SIGNAL(clicked()), this, SLOT(writeSpc()));
	QObject::connect(ui->readMeidButton, SIGNAL(clicked()), this, SLOT(qualcommReadMeid()));
	QObject::connect(ui->writeMeidButton, SIGNAL(clicked()), this, SLOT(writeMeid()));
	QObject::connect(ui->readImeiButton, SIGNAL(clicked()), this, SLOT(qualcommReadImei()));
	QObject::connect(ui->writeImeiButton, SIGNAL(clicked()), this, SLOT(writeImei()));
	QObject::connect(ui->readSubscriptionButton, SIGNAL(clicked()), this, SLOT(qualcommReadSubscription()));
	QObject::connect(ui->writeSubscriptionButton, SIGNAL(clicked()), this, SLOT(writeSubscription()));

	// LG
	QObject::connect(ui->lgSprintSimUnlockButton, SIGNAL(clicked()), this, SLOT(lgSprintSimUnlock()));
	QObject::connect(ui->lgRemovePasswordButton, SIGNAL(clicked()), this, SLOT(lgRemovePasscode()));
	QObject::connect(ui->lgRemoveFrpButton, SIGNAL(clicked()), this, SLOT(lgRemoveFrp()));

	// Samsung
	QObject::connect(ui->samsungSimUnlockButton, SIGNAL(clicked()), this, SLOT(samsungSimUnlock()));
}

/**
* @brief QcSamUnlockWindow::~QcSamUnlockWindow
*/
QcSamUnlockWindow::~QcSamUnlockWindow()
{
    delete ui;
}

/**
* @brief QcSamUnlockWindow::updatePortList
*/
void QcSamUnlockWindow::updatePortList()
{
	std::vector<serial::PortInfo> devices = serial::list_ports();
	std::vector<serial::PortInfo>::iterator iter = devices.begin();

	ui->portListComboBox->clear();
	ui->portListComboBox->addItem("- Select a Port -", 0);

	while (iter != devices.end()) {
		serial::PortInfo device = *iter++;

		QString item = device.port.c_str();
		item.append(" - ").append(device.description.c_str());

		ui->portListComboBox->addItem(item, device.port.c_str());

		QString logMsg = "Found ";

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
		logMsg.append(device.hardware_id.c_str()).append(" on ").append(device.port.c_str());
#endif
		if (device.description.length()) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
			logMsg.append(" - ")
#endif
			logMsg.append(device.description.c_str());
		}

		log(kLogTypeDebug, logMsg);
	}

	logAddEmptyLine();
}

/**
* @brief QcSamUnlockWindow::connectPort
*/
void QcSamUnlockWindow::connectPort(int portType)
{
	QString selected = ui->portListComboBox->currentData().toString();

    if (selected.compare("0") == 0) {
        log(kLogTypeWarning, "Select a Port First");
        return;
    }

    std::vector<serial::PortInfo> devices = serial::list_ports();
    std::vector<serial::PortInfo>::iterator iter = devices.begin();

    while (iter != devices.end()) {
        serial::PortInfo device = *iter++;
        if (selected.compare(device.port.c_str(), Qt::CaseInsensitive) == 0) {
            currentPort = device;
            break;
        }
    }

    if (!currentPort.port.length()) {
        log(kLogTypeError, "Invalid Port Type");
        return;
    }

	if (lafPort.isOpen() || qcdmPort.isOpen()) { return; }

	switch (portType) {
	case kPortTypeLaf:
		try {
			

			lafPort.setPort(currentPort.port);
			lafPort.open();
		}
		catch (serial::IOException e) {
			log(kLogTypeError, "Error Connecting To Serial Port");
			log(kLogTypeError, e.getErrorNumber() == 13 ? "Permission Denied. Try Running With Elevated Privledges." : e.what());
			return;
		}
		catch (std::exception e) {
			log(kLogTypeError, e.what());
			return;
		}
		break;
	case kPortTypeQcdm:
		try {
			

			qcdmPort.setPort(currentPort.port);
			qcdmPort.open();
		}
		catch (serial::IOException e) {
			log(kLogTypeError, "Error Connecting To Serial Port");
			log(kLogTypeError, e.getErrorNumber() == 13 ? "Permission Denied. Try Running With Elevated Privledges." : e.what());
			return;
		}
		catch (std::exception e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	}

	clearLog();

	ui->updatePortListButton->setEnabled(false);
	ui->portListComboBox->setEnabled(false);

	QString connectedText = "Connected to ";
	connectedText.append(currentPort.port.c_str());
	log(kLogTypeInfo, connectedText);
}

/**
* @brief QcSamUnlockWindow::disconnectPort
*/
void QcSamUnlockWindow::disconnectPort(int portType)
{
	switch (portType) {
	case kPortTypeLaf:
		if (lafPort.isOpen()) {
			lafPort.close();
		}

		break;
	case kPortTypeQcdm:
		if (qcdmPort.isOpen()) {
			qcdmPort.close();
		}

		break;
	}

	logAddEmptyLine();

	QString closeText = "Disconnected from ";
	closeText.append(currentPort.port.c_str());
	log(kLogTypeInfo, closeText);

	ui->updatePortListButton->setEnabled(true);
	ui->portListComboBox->setEnabled(true);
}

/**
* @brief QcSamUnlockWindow::qualcommReadAll
*/
void QcSamUnlockWindow::qualcommReadAll()
{
	connectPort(kPortTypeQcdm);

	if (!qcdmPort.isOpen()) { return; }

	readSpc();
	readMeid();
	readImei();
	readSubscription();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadAll
*/
void QcSamUnlockWindow::qualcommReadSpc()
{
	connectPort(kPortTypeQcdm);

	if (!qcdmPort.isOpen()) { return; }

	readSpc();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadMeid
*/
void QcSamUnlockWindow::qualcommReadMeid()
{
	connectPort(kPortTypeQcdm);

	if (!qcdmPort.isOpen()) { return; }

	readMeid();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadImei
*/
void QcSamUnlockWindow::qualcommReadImei()
{
	connectPort(kPortTypeQcdm);

	if (!qcdmPort.isOpen()) { return; }

	readImei();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadSubscription
*/
void QcSamUnlockWindow::qualcommReadSubscription()
{
	connectPort(kPortTypeQcdm);

	if (!qcdmPort.isOpen()) { return; }

	readSubscription();

	disconnectPort(kPortTypeQcdm);
}


/**
* @brief QcSamUnlockWindow::readSpc
*/
void QcSamUnlockWindow::readSpc()
{
	logAddEmptyLine();
	log(kLogTypeInfo, "TASK:   SPC READ");
	log(kLogTypeInfo, "METHOD: " + ui->readSpcMethod->currentText());
	logAddEmptyLine();

	QcdmNvResponse nvItem;
	QString spc;

	switch (ui->readSpcMethod->currentIndex()) {
	case kSpcReadTypeNv:
		try {
			nvItem = qcdmPort.readNV(NV_SEC_CODE_I);

			spc = QString::fromStdString(hexToString((char *)nvItem.data, 6));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
		}

		break;
	case kSpcReadTypeHtc:
		try {
			qcdmPort.sendHtcNvUnlock();

			nvItem = qcdmPort.readNV(NV_SEC_CODE_I);

			spc = QString::fromStdString(hexToString((char *)nvItem.data, 6));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
		}

		break;
	case kSpcReadTypeLg:
		try {
			qcdmPort.sendLgNvUnlock();

			QcdmLgSpcResponse lgSpc = qcdmPort.getLgSpc();

			spc = QString::fromStdString(hexToString((char *)lgSpc.spc, 6));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
		}

		break;
	case kSpcReadTypeEfs:
		if (efsManager.statfs("/nvm/num/85", statResponse) == efsManager.kDmEfsSuccess) {
			// TODO:
		}

		break;
	case kSpcReadTypeEfsForceWrite:
		if (efsManager.statfs("/nvm/num/85", statResponse) == efsManager.kDmEfsSuccess) {
			// TODO:
		}

		break;
	case kSpcReadTypeSubsys:
		QcdmNvSubsysResponse subsys;

		try {
			subsys = qcdmPort.readNvSubsys(NV_SEC_CODE_I);

			spc = QString::fromStdString(hexToString((char *)subsys.data, 6));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
		}

		break;
	
	}

	if (spc.length() == 6) {
		ui->decSpcValue->setText(spc);
		log(kLogTypeInfo, "RESULT: " + spc);
	}
	else {
		log(kLogTypeError, "RESULT: FAIL");
	}
}

/**
* @brief QcSamUnlockWindow::writeSpc
*/
void QcSamUnlockWindow::writeSpc()
{
	connectPort(kPortTypeQcdm);

	QcdmNvRequest packet = {};

	if (ui->decSpcValue->text().length() != 6) {
		log(kLogTypeWarning, "Enter a Valid 6 Digit SPC");
		return;
	}

	switch (ui->readSpcMethod->currentIndex()) {
	case kSpcReadTypeNv:
		try {
			uint8_t* data = (uint8_t*)ui->decSpcValue->text().toStdString().c_str();

			std::memcpy(&packet.data, &data, sizeof(data));

			qcdmPort.writeNV(NV_SEC_CODE_I, (uint8_t*)&packet.data, sizeof(packet.data));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	case kSpcReadTypeHtc:
		try {
			// qcdmPort.sendHtcNvUnlock(); // Broken? No idea..

			uint8_t* data = (uint8_t*)ui->decSpcValue->text().toStdString().c_str();

			std::memcpy(&packet.data, &data, sizeof(data));

			qcdmPort.writeNV(NV_SEC_CODE_I, (uint8_t*)&packet.data, sizeof(packet.data));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	case kSpcReadTypeLg:
		try {
			qcdmPort.sendLgNvUnlock(); // LG Method

			uint8_t* data = (uint8_t*)ui->decSpcValue->text().toStdString().c_str();

			std::memcpy(&packet.data, &data, sizeof(data));

			qcdmPort.writeNV(NV_SEC_CODE_I, (uint8_t*)&packet.data, sizeof(packet.data));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	case kSpcReadTypeSubsys:
		try {
			uint8_t* data = (uint8_t*)ui->decSpcValue->text().toStdString().c_str();

			std::memcpy(&packet.data, &data, sizeof(data));

			qcdmPort.writeNvSubsys(NV_SEC_CODE_I, (uint8_t*)&packet.data, sizeof(packet.data));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	case kSpcReadTypeEfs:
		if (efsManager.statfs("/nvm/num/85", statResponse) == efsManager.kDmEfsSuccess) {
			// TODO:
		}

		break;
	}

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::readMeid
*/
void QcSamUnlockWindow::readMeid()
{
	logAddEmptyLine();
	log(kLogTypeInfo, "TASK:   MEID READ");
	log(kLogTypeInfo, "METHOD: " + ui->meidMethod->currentText());
	logAddEmptyLine();

	QString meid, tmp;

	switch (ui->meidMethod->currentIndex()) {
	case kImeiMeidMethodNv:
		QcdmNvResponse nvItem;

		try {
			nvItem = qcdmPort.readNV(NV_MEID_I);
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		std::reverse((uint8_t*)&nvItem.data, ((uint8_t*)&nvItem.data) + 7);

		for (int i = 0; i < 7; i++) {
			meid.append(tmp.sprintf("%02X", nvItem.data[i]));
		}

		break;
	case kImeiMeidMethodEfs:

		break;
	case kImeiMeidMethodSubsys:
		QcdmNvSubsysResponse subsys;

		try {
			subsys = qcdmPort.readNvSubsys(NV_MEID_I);
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		std::reverse((uint8_t*)&subsys.data, ((uint8_t*)&subsys.data) + 7);

		for (int i = 0; i < 7; i++) {
			meid.append(tmp.sprintf("%02X", subsys.data[i]));
		}
	}

	if (meid.length() >= 14) {
		ui->hexMeidValue->setText(meid);
		log(kLogTypeInfo, "RESULT: " + meid);
	}
	else {
		log(kLogTypeError, "RESULT: FAIL");
	}
}

/**
* @brief QcSamUnlockWindow::writeMeid
*/
void QcSamUnlockWindow::writeMeid()
{
	connectPort(kPortTypeQcdm);

	if (ui->hexMeidValue->text().length() != 14) {
		log(kLogTypeWarning, "Enter a Valid 14 Character MEID");
		return;
	}

	QcdmNvRequest packet = {};

	long long data = HexToBytes(ui->hexMeidValue->text().toStdString());

	switch (ui->meidMethod->currentIndex()) {
	case kImeiMeidMethodNv:
		std::memcpy(&packet.data, &data, sizeof(data));

		try {
			qcdmPort.writeNV(NV_MEID_I, (uint8_t*)&packet.data, sizeof(packet.data));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	case kImeiMeidMethodEfs:

		break;
	case kImeiMeidMethodEfsForceWrite:

		break;
	case kImeiMeidMethodSubsys:
		std::memcpy(&packet.data, &data, sizeof(data));

		try {
			qcdmPort.writeNvSubsys(NV_MEID_I, (uint8_t*)&packet.data, sizeof(packet.data));
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
			return;
		}

		break;
	}

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::readImei
*/
void QcSamUnlockWindow::readImei()
{
	logAddEmptyLine();
	log(kLogTypeInfo, "TASK:   IMEI READ");
	log(kLogTypeInfo, "METHOD: " + ui->meidMethod->currentText());
	logAddEmptyLine();

	QcdmNvResponse nvItem;
	QString imei, tmp;

	try {
		nvItem = qcdmPort.readNV(NV_UE_IMEI_I);
	}
	catch (std::exception &e) {
		log(kLogTypeError, e.what());
		return;
	}

	for (int i = 1; i < 9; i++) {
		if (i == 1) {
			imei.append(tmp.sprintf("%X", (nvItem.data[i] & 0xF0) >> 4));
		}
		else {
			imei.append(tmp.sprintf("%02X", (nvItem.data[i] & 0xF0) >> 4 | (nvItem.data[i] & 0x0F) << 4));
		}
	}

	if (imei.length() == 15) {
		ui->imeiValue->setText(imei);
		log(kLogTypeInfo, "RESULT: " + imei);
	}
	else {
		log(kLogTypeError, "RESULT: FAIL");
	}
}

/**
* @brief QcSamUnlockWindow::writeImei
*/
void QcSamUnlockWindow::writeImei()
{
	connectPort(kPortTypeQcdm);

	QcdmNvRequest packet = {};

	if (ui->imeiValue->text().length() != 15) {
		log(kLogTypeWarning, "Enter a Valid 15 Character IMEI");
	}

	QString imei = ui->imeiValue->text();

	// std::reverse(imei.constBegin(), imei.constEnd());

	long long data = HexToBytes(imei.toStdString());

	// for (int i = 1; i < 9; i++) {
	// 	 if (i == 1) {
	// 	   imei.append(tmp.sprintf("%X", (nvItem.data[i] & 0xF0) >> 4));
	// 	 }
	// 	 else {
	// 	   imei.append(tmp.sprintf("%02X", (nvItem.data[i] & 0xF0) >> 4 | (nvItem.data[i] & 0x0F) << 4));
	// 	 }
	// }

	// std::reverse((uint8_t*)data, (uint8_t*)data);

	std::memcpy(&packet.data, &data, sizeof(data));

	hexdump((uint8_t*)packet.data, sizeof(packet.data));

	// try {
	// 	port.writeNV(NV_UE_IMEI_I, (uint8_t*)&packet.data, sizeof(packet.data));
	// }
	// catch (std::exception &e) {
	// 	log(kLogTypeError, e.what());
	// 	return;
	// }

	// for (int i = 1; i < 9; i++) {
	// 	if (i == 1) {
	// 		imei.append(tmp.sprintf("%X", (nvItem.data[i] & 0xF0) >> 4));
	// 	}
	// 	else {
	// 		imei.append(tmp.sprintf("%02X", (nvItem.data[i] & 0xF0) >> 4 | (nvItem.data[i] & 0x0F) << 4));
	// 	}
	// }

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::readSubscription
*/
void QcSamUnlockWindow::readSubscription()
{
	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: SUBSCRIPTION MODE READ");
	logAddEmptyLine();

	QcdmNvResponse nvItem;

	try {
		nvItem = qcdmPort.readNV(NV_RTRE_CONFIG_I);
	}
	catch (std::exception &e) {
		log(kLogTypeError, e.what());
		return;
	}

	ui->subscriptionValue->setCurrentIndex(ui->subscriptionValue->findData(nvItem.data[0]));

	log(kLogTypeInfo, "RESULT: " + ui->subscriptionValue->currentText());
}

/**
* @brief QcSamUnlockWindow::writeSubscription
*/
void QcSamUnlockWindow::writeSubscription()
{
	connectPort(kPortTypeQcdm);

	int mode = ui->subscriptionValue->currentData().toUInt();

	if (mode < 0) {
		log(kLogTypeError, "Select a Subsciption Mode to Write");
		return;
	}

	QcdmNvRequest packet = {};

	packet.data[0] = mode;

	try {
		qcdmPort.writeNV(NV_RTRE_CONFIG_I, &packet.data[0], sizeof(packet.data));
	}
	catch (std::exception &e) {
		log(kLogTypeError, e.what());
		return;
	}

	ui->subscriptionValue->setCurrentIndex(ui->subscriptionValue->findData(packet.data[0]));

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::processItem
* Delete an NV item by deltree traversal exploit
* @param int item - The NV item number to delete
* @return bool
*/
bool QcSamUnlockWindow::processItem(int item, int sequence)
{
	ostringstream path;
	path << "/public/../nvm/num/" << item; // deltree only works in /public

	if (efsManager.mkdir("public", 0x01FF) == efsManager.kDmEfsSuccess) {
		if (efsManager.deltree(path.str(), sequence) == efsManager.kDmEfsSuccess) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}

/**
* @brief QcSamUnlockWindow::lgRemoveFrp
*/
void QcSamUnlockWindow::lgRemoveFrp()
{
	connectPort(kPortTypeLaf);

	if (!lafPort.isOpen()) { return; }

	log(kLogTypeInfo, "TASK: LG FRP REMOVE");
	logAddEmptyLine();

	if (lafPort.sendCommand("id").find("root") != string::npos) {
	// // G4 -- All Variants
	// dd if = / dev / block / mmcblk0 bs = 8192 seek = 23808 count = 10 of = <output>  // Backup
	// 
	// dd if = / dev / zero bs = 8192 seek = 23808 count = 10 of = / dev / block / mmcblk0 // Remove FRP
	// 
	// // G Flex / G Flex 2 -- All Variants
	// dd if = / dev / block / mmcblk0 bs = 8192 seek = 21697 count = 256 of = <output>  // Backup
	// dd if = / dev / block / mmcblk0p34 of = / dev / zero                           // Backup
	// 
	// dd if = / dev / zero bs = 8192 seek = 21697 count = 256 of = / dev / block / mmcblk0 // Remove FRP
	// dd if = / dev / zero of = / dev / block / mmcblk0p34                           // Remove FRP
	// 
	// // G Stylo - Leon - Risio -- All Variants
	// dd if = / dev / block / mmcblk0p34 of = <output>  // Backup
	// 
	// dd if = / dev / zero of = / dev / block / mmcblk0p34 // Remove FRP
	// 
	// // G4s Beat
	// 
	// dd if = / dev / block / mmcblk0p27 of = <output>  // Backup
	// dd if = / dev / block / mmcblk0p28 of = <output>  // Backup
	// 
	// dd if = / dev / zero of = / dev / block / mmcblk0p27 // Remove FRP
	// dd if = / dev / zero of = / dev / block / mmcblk0p28 // Remove FRP
	// 
	// // Leon LTE MetroPCS
	// dd if = / dev / block / mmcblk0 bs = 8192 seek = 37184 count = 10 of = <output>  // Backup
	// 
	// dd if = / dev / zero bs = 8192 seek = 37184 count = 10 of = / dev / block / mmcblk0 // Remove FRP

		log(kLogTypeInfo, "RESULT: SUCCESS");
	}
	else {
		log(kLogTypeError, "Phone Detection Failed");
		log(kLogTypeError, "RESULT: FAIL");
	}

	disconnectPort(kPortTypeLaf);
}

/**
* @brief QcSamUnlockWindow::lgRemovePasscode
*/
void QcSamUnlockWindow::lgRemovePasscode()
{
	connectPort(kPortTypeLaf);

	if (!lafPort.isOpen()) { return; }

	log(kLogTypeInfo, "TASK: LG PASSCODE REMOVE");
	logAddEmptyLine();

	if (lafPort.sendCommand("id").find("root") != string::npos) {
		lafPort.sendCommand("rm /data/system/locksettings.db");
		lafPort.sendCommand("rm /data/system/locksettings.db-shm");
		lafPort.sendCommand("rm /data/system/locksettings.db-wal");
		lafPort.sendCommand("rm /data/system/gesture.key");
		lafPort.sendCommand("rm /data/system/password.key");

		log(kLogTypeInfo, "RESULT: SUCCESS");
	}
	else {
		log(kLogTypeError, "Phone Detection Failed");
		log(kLogTypeError, "RESULT: FAIL");
	}

	disconnectPort(kPortTypeLaf);
}

/**
* @brief QcSamUnlockWindow::lgSprintSimUnlock
*/
void QcSamUnlockWindow::lgSprintSimUnlock()
{
	connectPort(kPortTypeLaf);

	if (!lafPort.isOpen()) { return; }

	log(kLogTypeInfo, "TASK: LG SPRINT SIM UNLOCK");
	logAddEmptyLine();

	if (lafPort.sendCommand("id").find("root") != string::npos) {
		lafPort.sendCommand("mkdir carrier");
		lafPort.sendCommand("mount -t ext4 /dev/block/bootdevice/by-name/carrier /carrier");
		lafPort.sendCommand("mount -t ext4 /dev/block/msm_sdcc.1/by-name/carrier /carrier");
		lafPort.sendCommand("chown -R root:root /carrier");
		lafPort.sendCommand("umount /carrier");
		lafPort.sendCommand("sync");

		log(kLogTypeInfo, "RESULT: SUCCESS");
	}
	else {
		log(kLogTypeError, "Phone Detection Failed");
		log(kLogTypeError, "RESULT: FAIL");
	}

	disconnectPort(kPortTypeLaf);
}

/**
* @brief QcSamUnlockWindow::unlockSim
*/
void QcSamUnlockWindow::samsungSimUnlock()
{
	switch (ui->samsungSimUnlockComboBox->currentIndex()) {
	case kSamsungSimUnlockTypeEfs:
		samsungEfsSimUnlock();
		break;
	case kSamsungSimUnlockTypeNvData:

		break;
	}
}

/**
* @brief QcSamUnlockWindow::SamsungEfsSimUnlock
*/

void QcSamUnlockWindow::samsungEfsSimUnlock() {
	connectPort(kPortTypeQcdm);

	log(kLogTypeInfo, "TASK: SAMSUNG EFS SIM UNLOCK");
	logAddEmptyLine();

	int removeFailCount = 0;

	if (!testSecurity()) {
		goto finish;
	}

	if (!processItem(10080, ++sequence)) {
		log(kLogTypeWarning, "Error removing NV item 10080");
		removeFailCount++;
	}

	if (!processItem(10074, ++sequence)) {
		log(kLogTypeWarning, "Error removing NV item 10074");
		removeFailCount++;
	}

	if (!processItem(10073, ++sequence)) {
		log(kLogTypeWarning, "Error removing NV item 10073");
		removeFailCount++;
	}

	try {
		syncResponse = efsManager.syncNoWait("/");

		QString msg = "EFS Sync Initiated. Received token: ";
		log(kLogTypeInfo, msg.append(syncResponse.token));
	}
	catch (std::exception e) {
		log(kLogTypeError, "EFS Sync Error: " + QString::fromStdString(e.what()));
		goto finish;
	}

	try {
		sequence = 0;

		while (syncMaxRetries > syncRetries) {
			syncStatusResponse = efsManager.getSyncStatus("/", syncResponse.token, ++sequence);
			if (syncStatusResponse.status) {
				log(kLogTypeInfo, "EFS Sync Complete");
				log(kLogTypeInfo, "Finished");
				logAddEmptyLine();

				if (removeFailCount > 2) {
					log(kLogTypeDebug, "Phone is either already unlocked or not supported");
				}
				else {
					log(kLogTypeInfo, "Finished. Unlock successful");
				}

				goto finish;
			}
			else {
				sleep(1000); // wait and check again
				syncRetries++;
			}
		}

		log(kLogTypeInfo, "EFS Sync Check Error");
		log(kLogTypeInfo, "Phone may still have been unlocked");
		log(kLogTypeInfo, "Reboot and insert a different carriers SIM");
	}
	catch (std::exception e) {
		log(kLogTypeError, "EFS Sync Check Error: " + QString::fromStdString(e.what()));
		goto finish;
	}

finish:
	logAddEmptyLine();
	log(kLogTypeDebug, "Select MTP to restore phone to default USB mode");
	disconnectPort(kPortTypeQcdm);
	saveLog("SAMSUNG EFS SIM UNLOCK");
}

/**
* @brief QcSamUnlockWindow::testSecurity
*/
bool QcSamUnlockWindow::testSecurity()
{
	bool success = false;

	if (efsManager.statfs("/", statResponse) == efsManager.kDmEfsError) {
		log(kLogTypeWarning, "Bad Response. Attempting Security Bypass");

		string passwords[] = { "01F2030F5F678FF9", "2010031619780721", "2013051320130909", "2009031920090615", "2012112120131219" };
		for (const string &password : passwords) {
			if (qcdmPort.sendPassword(password)) {
				success = true;
				break;
			}
		}

		if (success) {
			log(kLogTypeInfo, "Security Bypass Successful");
		} else {
			log(kLogTypeError, "Security Bypass Failure: Device Not Supported");
		}
	} else {
		success = true;
		log(kLogTypeInfo, "EFS Access Available");
	}

	return success;
}

/**
* @brief QcSamUnlockWindow::clearLog
*/
void QcSamUnlockWindow::clearLog() 
{
    ui->log->clear();
}

/**
* @brief QcSamUnlockWindow::saveLog
*/
void QcSamUnlockWindow::saveLog(QString task) 
{
	QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

	log(kLogTypeInfo, QDir::currentPath() + "/log/" + now + "_" + task + ".pst");

	QFile file(QDir::currentPath() + "/log/" + now + "_" + task + ".pst");
	if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
		QTextStream stream(&file);
		stream << now << endl;
		stream << ui->log->toPlainText() << endl;
	}
}

/**
* @brief QcSamUnlockWindow::log
* @param type
* @param message
*/
void QcSamUnlockWindow::log(int type, QString message)
{
	QString color, prefix, tmp;

	switch (type) {
	case kLogTypeDebug:
		color = "gray";
		prefix = "[*]";
		break;
	case kLogTypeError:
		color = "red";
		prefix = "[*]";
		break;
	case kLogTypeWarning:
		color = "orange";
		prefix = "[*]";
		break;
	case kLogTypeInfo:
		color = "green";
		prefix = "[*]";
		break;
	}
	
	if (message.length()) {
		ui->log->appendHtml(tmp.sprintf("<font color=%s><pre>%s %s</pre></font>", tmp.toStdString().c_str(), prefix.toStdString().c_str(), message.toStdString().c_str()));
	}
}

/**
* @brief QcSamUnlockWindow::logAddEmptyLine
*/
void QcSamUnlockWindow::logAddEmptyLine()
{
	ui->log->appendHtml("<span>&nbsp;</span");
}
