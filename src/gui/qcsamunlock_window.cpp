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
	serialPort("", 115200, serial::Timeout::simpleTimeout(500)),
	efsManager(qcdmPort)
{
    ui->setupUi(this);

    updatePortList();

	ui->subscriptionValue->addItem("", -1);
	ui->subscriptionValue->addItem("RUIM ONLY", kQcdmRuimConfigTypeRuim);
	ui->subscriptionValue->addItem("NV ONLY", kQcdmRuimConfigTypeNv);
	ui->subscriptionValue->addItem("RUIM PREFERRED", kQcdmRuimConfigTypePref);
	ui->subscriptionValue->addItem("GSM 1X", kQcdmRuimConfigTypeGsm1x);

	qRegisterMetaType<SerialAtCommandWorkerRequest>("SerialAtCommandWorkerRequest");
	qRegisterMetaType<SerialAtCommandWorkerResponse>("SerialAtCommandWorkerResponse");

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
	QObject::connect(ui->samsungUartTaskButton, SIGNAL(clicked()), this, SLOT(samsungUartPerformTask()));
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
bool QcSamUnlockWindow::connectPort(int portType)
{
	QString selected = ui->portListComboBox->currentData().toString();

    if (selected.compare("0") == 0) {
        log(kLogTypeWarning, "Select a Port First");
        return false;
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
        return false;
    }

	if (lafPort.isOpen() || qcdmPort.isOpen()) { return true; }

	switch (portType) {
	case kPortTypeLaf:
		try {
			lafPort.setPort(currentPort.port);
			lafPort.open();
		}
		catch (serial::IOException e) {
			log(kLogTypeError, "Error Connecting To Serial Port");
			log(kLogTypeError, e.getErrorNumber() == 13 ? "Permission Denied. Try Running With Elevated Privledges." : e.what());
			return false;
		}
		catch (std::exception e) {
			log(kLogTypeError, e.what());
			return false;
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
			return false;
		}
		catch (std::exception e) {
			log(kLogTypeError, e.what());
			return false;
		}

		break;
	case kPortTypeSerial:
		try {
			serialPort.setPort(currentPort.port);
			serialPort.open();
		}
		catch (serial::IOException e) {
			log(kLogTypeError, "Error Connecting To Serial Port");
			log(kLogTypeError, e.getErrorNumber() == 13 ? "Permission Denied. Try Running With Elevated Privledges." : e.what());
			return false;
		}
		catch (std::exception e) {
			log(kLogTypeError, e.what());
			return false;
		}

		break;
	}

	clearLog();

	ui->updatePortListButton->setEnabled(false);
	ui->portListComboBox->setEnabled(false);

	QString connectedText = "Connected to ";
	connectedText.append(currentPort.port.c_str());
	log(kLogTypeInfo, connectedText);

	return true;
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
	case kPortTypeSerial:
		if (serialPort.isOpen()) {
			serialPort.close();
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
* @brief QcSamUnlockWindow::sendAtCommand
*/
bool QcSamUnlockWindow::samsungUartTestConnection()
{
	serialPort.write("AT\r\n");

	for (string line : serialPort.readlines()) {
		if (line.length() && QString::fromStdString(line) != "AT\r\n") {
			printf(line.c_str());
			return true;
		}
	}

	sleep(300);

	return false;
}

/**
* @brief QcSamUnlockWindow::qualcommReadAll
*/
void QcSamUnlockWindow::qualcommReadAll()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

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
	if (!connectPort(kPortTypeQcdm)) { return; }

	readSpc();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadMeid
*/
void QcSamUnlockWindow::qualcommReadMeid()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

	readMeid();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadImei
*/
void QcSamUnlockWindow::qualcommReadImei()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

	readImei();

	disconnectPort(kPortTypeQcdm);
}

/**
* @brief QcSamUnlockWindow::qualcommReadSubscription
*/
void QcSamUnlockWindow::qualcommReadSubscription()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

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
	if (!connectPort(kPortTypeQcdm)) { return; }

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

	QString meid, tmp = "";

	switch (ui->meidMethod->currentIndex()) {
	case kImeiMeidMethodNv:
		QcdmNvResponse nvItem;

		try {
			nvItem = qcdmPort.readNV(NV_MEID_I);
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
		}

		std::reverse((uint8_t*)&nvItem.data, ((uint8_t*)&nvItem.data) + 7);

		for (int i = 0; i < 7; i++) {
			meid.append(tmp.sprintf("%02X", nvItem.data[i]));
		}

		break;
	case kImeiMeidMethodEfs || kImeiMeidMethodEfsForceWrite:
		if (efsManager.statfs("/nvm/num/1943", statResponse) == efsManager.kDmEfsSuccess) {
			// TODO:
		}

		break;
	case kImeiMeidMethodSubsys:
		QcdmNvSubsysResponse subsys;

		try {
			subsys = qcdmPort.readNvSubsys(NV_MEID_I);
		}
		catch (std::exception &e) {
			log(kLogTypeError, e.what());
		}

		std::reverse((uint8_t*)&subsys.data, ((uint8_t*)&subsys.data) + 7);

		for (int i = 0; i < 7; i++) {
			meid.append(tmp.sprintf("%02X", subsys.data[i]));
		}
	}

	if (meid.length() != 14 || meid.toStdString() == "CCCCCCCCCCCCCC") {
		log(kLogTypeError, "RESULT: FAIL");
	}
	else {
		ui->hexMeidValue->setText(meid);
		log(kLogTypeInfo, "RESULT: " + meid);
	}
}

/**
* @brief QcSamUnlockWindow::writeMeid
*/
void QcSamUnlockWindow::writeMeid()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

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
		if (efsManager.statfs("/nvm/num/1943", statResponse) == efsManager.kDmEfsSuccess) {
			// TODO:
		}

		break;
	case kImeiMeidMethodEfsForceWrite:
		if (efsManager.statfs("/nvm/num/1943", statResponse) == efsManager.kDmEfsSuccess) {
			// TODO:
		}

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
	}

	for (int i = 1; i < 9; i++) {
		if (i == 1) {
			imei.append(tmp.sprintf("%X", (nvItem.data[i] & 0xF0) >> 4));
		}
		else {
			imei.append(tmp.sprintf("%02X", (nvItem.data[i] & 0xF0) >> 4 | (nvItem.data[i] & 0x0F) << 4));
		}
	}

	if (imei.length() == 15 || imei.toStdString() == "CCCCCCCCCCCCCCC") {
		log(kLogTypeError, "RESULT: FAIL");
	}
	else {
		ui->imeiValue->setText(imei);
		log(kLogTypeInfo, "RESULT: " + imei);
	}
}

/**
* @brief QcSamUnlockWindow::writeImei
*/
void QcSamUnlockWindow::writeImei()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

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
	log(kLogTypeInfo, "TASK: QUALCOMM SUBSCRIPTION MODE READ");
	logAddEmptyLine();

	QcdmNvResponse nvItem;

	try {
		nvItem = qcdmPort.readNV(NV_RTRE_CONFIG_I);
	}
	catch (std::exception &e) {
		log(kLogTypeError, e.what());
	}

	if (nvItem.data[0] = 0) {
		log(kLogTypeError, "RESULT: FAIL");
	}
	else {
		ui->subscriptionValue->setCurrentIndex(nvItem.data[0]);

		log(kLogTypeInfo, "RESULT: " + ui->subscriptionValue->currentText());
	}
}

/**
* @brief QcSamUnlockWindow::writeSubscription
*/
void QcSamUnlockWindow::writeSubscription()
{
	if (!connectPort(kPortTypeQcdm)) { return; }

	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: QUALCOMM SUBSCRIPTION MODE WRITE");
	logAddEmptyLine();

	int mode = ui->subscriptionValue->currentData().toUInt();

	if (mode < 0) {
		log(kLogTypeError, "SELECT A SUBSCRIPTION MODE TO WRITE");
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
	
	if (packet.data[0] = 0) {
		log(kLogTypeError, "RESULT: FAIL");
	}
	else {
		ui->subscriptionValue->setCurrentIndex(packet.data[0]);

		log(kLogTypeInfo, "RESULT: " + ui->subscriptionValue->currentText());
	}

	disconnectPort(kPortTypeQcdm);
	saveLog("QUALCOMM SUBSCRIPTION MODE WRITE");
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
void QcSamUnlockWindow::lgGetDeviceInfo()
{
	QString serial = QString::fromStdString(lafPort.sendCommand("getprop ro.serialno"));

	log(kLogTypeInfo, "SERIAL:   " + serial);

	QString value;

	QString buildprop = QString::fromStdString(lafPort.sendCommand("cat build.prop"));

	// log(kLogTypeDebug, "BUILD.PROP:<br>" + buildprop);
	// logAddEmptyLine();

	QStringList lines = buildprop.split("\n");

	for (QString line : lines) {
		if (line.contains("ro.product.model=")) {
			QStringList prop = line.split("=");
			value = prop[1];
			log(kLogTypeDebug, "MODEL:    " + value);
		}

		if (line.contains("ro.lge.swversion_short=")) {
			QStringList prop = line.split("=");
			value = prop[1];
			log(kLogTypeDebug, "BUILD:    " + value);
		}

		if (line.contains("ro.build.target_operator=")) {
			QStringList prop = line.split("=");
			value = prop[1];
			log(kLogTypeDebug, "OPERATOR: " + value);
		}
	}

	logAddEmptyLine();
}

/**
* @brief QcSamUnlockWindow::lgRemoveFrp
*/
void QcSamUnlockWindow::lgRemoveFrp()
{
	if (!connectPort(kPortTypeLaf)) { return; }

	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: LG FRP REMOVE");
	logAddEmptyLine();

	lafPort.sendHello();
	lafPort.enterLaf();

	// QString test = QString::fromStdString(lafPort.getProperty(LAF_PROP_IMEI));
	// log(kLogTypeDebug, "PROPERTY: " + test);

	if (lafPort.sendCommand("id").find("root") != string::npos) {

		

		// lgGetDeviceInfo();

		// QString mounts = QString::fromStdString(lafPort.sendCommand("cat /proc/mounts"));
		// 
		// log(kLogTypeDebug, "MOUNTS:<br>" + mounts);
		// 
		// QString partitions = QString::fromStdString(lafPort.sendCommand("cat /proc/partitions"));
		// 
		// log(kLogTypeDebug, "PARTITIONS:<br>" + partitions);
		// 
		// logAddEmptyLine();

		// QString test = QString::fromStdString(lafPort.sendCommand("SPECIAL"));
		// log(kLogTypeDebug, "SPECIAL: " + test);

		// QString model;
		// 
		// QString buildprop = QString::fromStdString(lafPort.sendCommand("cat build.prop"));
		// 
		// QStringList lines = buildprop.split("\n");
		// 
		// for (QString line : lines) {
		// 	if (line.contains("ro.product.model=")) {
		// 		QStringList prop = line.split("=");
		// 		model = prop[1];
		// 	}
		// }
		// 
		// log(kLogTypeDebug, "CHECKING DEVICE COMPATIBILITY");
		// 
		// QStringList lgG4Variants = { "VS986", "LS991", "US991", "H810", "H811", "H812", "H815", "H818", "H819", "F500" };
		// 
		// for (QString variant : lgG4Variants) {
		// 	if (model.contains(variant)) {
		// 		log(kLogTypeDebug, "DEVICE IS COMPATIBLE");
		// 		logAddEmptyLine();
		// 
		// 		log(kLogTypeDebug, "CREATING PARTITION BACKUP");
		// 
		// 		// FIXME
		// 		// lafPort.sendCommand("dd if=/dev/block/mmcblk0 bs=8192 seek=23808 count=10 of=");
		// 
		// 		log(kLogTypeDebug, "REMOVING GOOGLE FRP");
		// 		
		// 		// lafPort.sendCommand("dd if=/dev/zero bs=8192 seek=23808 count=10 of=/dev/block/mmcblk0");
		// 	}
		// }
		// 
		// QStringList lgGFlexVariants = { "D950", "D955", "D958", "D959", "LS995", "F340", "H950", "H955", "LS996", "US995" };
		// 
		// for (QString variant : lgGFlexVariants) {
		// 	if (model.contains(variant)) {
		// 		log(kLogTypeDebug, variant + " DETECTED");
		// 		logAddEmptyLine();
		// 
		// 		log(kLogTypeDebug, "CREATING PARTITION BACKUP");
		// 
		// 		// FIXME
		// 		lafPort.sendCommand("dd if=/dev/block/mmcblk0 bs=8192 seek=21697 count=256 of=");
		// 		lafPort.sendCommand("dd if=/dev/block/mmcblk0p34 of=");
		// 
		// 		log(kLogTypeDebug, "REMOVING GOOGLE FRP");
		// 
		// 		lafPort.sendCommand("dd if=/dev/zero bs=8192 seek=21697 count=256 of=/dev/block/mmcblk0");
		// 		lafPort.sendCommand("dd if=/dev/zero of=/dev/block/mmcblk0p34");
		// 	}
		// }
		// 
		// QStringList lgLeonRisioStyloVariants = { "H631", "MS631", "LS770", "H340", "MS345", "H343" };
		// 
		// for (QString variant : lgLeonRisioStyloVariants) {
		// 	if (model.contains(variant)) {
		// 		log(kLogTypeDebug, variant + " DETECTED");
		// 		logAddEmptyLine();
		// 
		// 		log(kLogTypeDebug, "CREATING PARTITION BACKUP");
		// 
		// 		// FIXME
		// 		lafPort.sendCommand("dd if=/dev/block/mmcblk0p34 of=");
		// 
		// 		log(kLogTypeDebug, "REMOVING GOOGLE FRP");
		// 
		// 		lafPort.sendCommand("dd if=/dev/zero of=/dev/block/mmcblk0p34");
		// 	}
		// }
		// 
		// logAddEmptyLine();

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

		
		
		lafPort.sendPowerOff();
	}
	else {
		log(kLogTypeError, "Phone Detection Failed");
		log(kLogTypeError, "RESULT: FAIL");
	}

	disconnectPort(kPortTypeLaf);
	saveLog("LG FRP REMOVE");
}

/**
* @brief QcSamUnlockWindow::lgRemovePasscode
*/
void QcSamUnlockWindow::lgRemovePasscode()
{
	if (!connectPort(kPortTypeLaf)) { return; }

	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: LG PASSCODE REMOVE");
	logAddEmptyLine();

	lafPort.sendHello();
	lafPort.enterLaf();

	if (lafPort.sendCommand("id").find("root") != string::npos) {
		lgGetDeviceInfo();

		lafPort.sendCommand("rm /data/system/locksettings.db");
		lafPort.sendCommand("rm /data/system/locksettings.db-shm");
		lafPort.sendCommand("rm /data/system/locksettings.db-wal");
		lafPort.sendCommand("rm /data/system/gesture.key");
		lafPort.sendCommand("rm /data/system/password.key");

		log(kLogTypeInfo, "REBOOTING DEVICE");
		logAddEmptyLine();

		lafPort.sendReset();

		log(kLogTypeInfo, "RESULT: SUCCESS");
	}
	else {
		log(kLogTypeError, "Phone Detection Failed");
		log(kLogTypeError, "RESULT: FAIL");
	}

	disconnectPort(kPortTypeLaf);
	saveLog("LG PASSCODE REMOVE");
}

/**
* @brief QcSamUnlockWindow::lgSprintSimUnlock
*/
void QcSamUnlockWindow::lgSprintSimUnlock()
{
	if (!connectPort(kPortTypeLaf)) { return; }

	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: LG SPRINT SIM UNLOCK");
	logAddEmptyLine();

	lafPort.sendHello();
	lafPort.enterLaf();

	sleep(5000);

	if (lafPort.sendCommand("id").find("root") != string::npos) {
		lgGetDeviceInfo();

		lafPort.sendCommand("mkdir carrier");
		lafPort.sendCommand("mount -t ext4 /dev/block/bootdevice/by-name/carrier /carrier");
		lafPort.sendCommand("mount -t ext4 /dev/block/platform/msm_sdcc.1/by-name/carrier /carrier");
		lafPort.sendCommand("chown -R root:root /carrier");
		lafPort.sendCommand("umount /carrier");
		lafPort.sendCommand("sync");

		log(kLogTypeInfo, "REBOOTING DEVICE");
		logAddEmptyLine();

		lafPort.sendReset();

		log(kLogTypeInfo, "RESULT: SUCCESS");
	}
	else {
		log(kLogTypeError, "Phone Detection Failed");
		log(kLogTypeError, "RESULT: FAIL");
	}

	disconnectPort(kPortTypeLaf);
	saveLog("LG SPRINT SIM UNLOCK");
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
	if (!connectPort(kPortTypeQcdm)) { return; }

	logAddEmptyLine();
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
* @brief QcSamUnlockWindow::samsungUartReadInfo
*/
void QcSamUnlockWindow::samsungUartReadInfo()
{
	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: SAMSUNG UART READ INFO");
	logAddEmptyLine();

	QStringList commands = {
		"AT+FACTOLOG=0,7,1,2",
		"AT+VERSNAME=3,2,0",
		"AT+SERIALNO=1,1",
		"AT+WIFIIDRW=1,0",
		"AT+IMEITEST=1,0",
		"AT+IMEITEST=1,1",
		"AT+MSLSECUR=1,0",
		"AT+DEVROOTK=0,0,0",
		"AT+IMEICERT=1,0",
		"AT+IMEICERT=1,1",
		"AT+IMEISIGN=1,1,0",
		"AT+IMEISIGN=0,1,0",
		"AT+REACTIVE=1,0,0",
		"AT+SECUREBT=1,2,0",
		"AT+LOCKREAD=1,0",
		"AT+LOCKINFO=1,0,0",
		"AT+LVFOLOCK=1,0",
		"AT+LVFOLOCK=1,1",
		"AT+SECUNBLOCK=1,0"
	};

	SerialAtCommandWorkerRequest request = {};
	request.task = kSamsungUartTaskReadInfo;
	request.commands = commands;

	AtCommandRequest(request);
}

/**
* @brief QcSamUnlockWindow::samsungUartReadSpc
*/
void QcSamUnlockWindow::samsungUartReadSpc()
{
	logAddEmptyLine();
	log(kLogTypeInfo, "TASK: SAMSUNG UART READ MSL / SPC");
	logAddEmptyLine();

	QStringList commands = {
		"AT+MEIDAUTH=0,0,2012112120131219",
		"AT+MEIDAUTH=1,0,0"
	};

	SerialAtCommandWorkerRequest request = {};
	request.task = kSamsungUartTaskReadSpc;
	request.commands = commands;

	AtCommandRequest(request);
}

/**
* @brief QcSamUnlockWindow::samsungUartParseReadInfo
*/
void QcSamUnlockWindow::samsungUartParseReadInfo(QString command, QString output)
{
	if (output.contains("+VERSNAME:3")) {
		QStringList lines = output.split(",");

		for (int i = 1; i < 6; i++) {
			switch (i) {
			case 1:
				log(kLogTypeDebug, "MODEL:         " + lines[i]);
				break;
			case 2:
				log(kLogTypeDebug, "HW VERSION:    " + lines[i]);
				break;
			case 3:
				log(kLogTypeDebug, "BUILD / SW:    " + lines[i]);
				break;
			case 4:
				log(kLogTypeDebug, "BUILD / SW:    " + lines[i]);
				break;
			case 5:
				log(kLogTypeDebug, "BASEBAND:      " + lines[i]);
				break;
			}
		}

		logAddEmptyLine();
	}

	if (output.contains("+SERIALNO:1")) {
		log(kLogTypeDebug, "SERIAL NO:     " + output.split(",")[1]);
	}

	if (output.contains("+WIFIIDRW:")) {
		log(kLogTypeDebug, "WIFI ID:       " + output.split(",")[1]);
	}

	if (command == "AT+IMEITEST=1,0"  && output.contains("+IMEITEST:1,")) {
		log(kLogTypeDebug, output.split(",")[1].contains("NONE") ? "IMEI:          EMPTY" : "IMEI:          " + output.split(",")[1]);
	}

	if (command == "AT+IMEITEST=1,1"  && output.contains("+IMEITEST:1,")) {
		log(kLogTypeDebug, output.split(",")[1].contains("NONE") ? "MEID:          EMPTY" : "MEID:          " + output.split(",")[1]);
	}

	if (output.contains("+IMEISIGN:1,")) {
		log(kLogTypeDebug, "SKEY:          " + output.split(",")[1]);
	}

	if (output.contains("+IMEISIGN:0,")) {
		log(kLogTypeDebug, "IMEI SIGN:     " + output.split(",")[1]);
	}

	if (output.contains("+MSLSECUR:1,")) {
		log(kLogTypeDebug, "MSL ADDRESS:   " + output.split(",")[1]);
	}

	if (output.contains("+DEVROOTK:0,")) {
		if (output.contains("NG") && output.contains("OK")) {
			log(kLogTypeDebug, "DEVROOTKEY:    BAD - NEEDS REPAIR");
		}
		else if (output.contains("NG") && output.contains("ERROR")) {
			log(kLogTypeDebug, "DEVROOTKEY:    NOT SUPPORTED");
		}
		else {
			log(kLogTypeDebug, "DEVROOTKEY:    " + output.split(",")[1]);
		}
	}

	if (command == "AT+IMEICERT=1,0"  && output.contains("+IMEICERT:1,")) {
		log(kLogTypeDebug, output.split(",")[1].contains("NA") ? "IMEI CERT 1:   NOT SUPPORTED" : "IMEI CERT 1:   " + output.split(",")[1]);
	}

	if (command == "AT+IMEICERT=1,1"  && output.contains("+IMEICERT:1,")) {
		log(kLogTypeDebug, output.split(",")[1].contains("NA") ? "IMEI CERT 2:   NOT SUPPORTED" : "IMEI CERT 1:   " + output.split(",")[1]);
	}

	if (output.contains("+REACTIVE:1,")) {
		logAddEmptyLine();
		log(kLogTypeDebug, output.split(",")[1].contains("NG") ? "REACTIVATION:  NOT SUPPORTED" : "REACTIVATION:  " + output.split(",")[1]);
	}

	if (output.contains("+SECUREBT:1,")) {
		log(kLogTypeDebug, output.split(",")[1].contains("1") ? "KNOX WARRANTY: VOID" : "KNOX WARRANTY: INTACT");
	}

	if (output.contains("+LOCKREAD:1,")) {
		if (output.contains("NA")) {
			logAddEmptyLine();
			log(kLogTypeDebug, "LOCK READ:     NOTHING FOUND");
		}
	}
}



/**
* @brief QcSamUnlockWindow::~samsungUartPerformTask
*/
void QcSamUnlockWindow::samsungUartPerformTask()
{
	if (!connectPort(kPortTypeSerial)) { return; }

	if (!samsungUartTestConnection()) {
		logAddEmptyLine();
		log(kLogTypeDebug, "UART CONNECTION FAIL");
		disconnectPort(kPortTypeSerial);
		return;
	}

	switch (ui->samsungUartTaskComboBox->currentIndex()) {
		case kSamsungUartTaskReadInfo:
			samsungUartReadInfo();
			break;
		case kSamsungUartTaskReadSpc:
			samsungUartReadSpc();
			break;
	}
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

void QcSamUnlockWindow::AtCommandRequest(SerialAtCommandWorkerRequest &request)
{
	serialAtCommandWorker = new SerialAtCommandWorker(serialPort, request, this);

	connect(serialAtCommandWorker, &SerialAtCommandWorker::update, this, &QcSamUnlockWindow::AtCommandUpdate, Qt::QueuedConnection);
	connect(serialAtCommandWorker, &SerialAtCommandWorker::complete, this, &QcSamUnlockWindow::AtCommandComplete);
	connect(serialAtCommandWorker, &SerialAtCommandWorker::error, this, &QcSamUnlockWindow::AtCommandError);
	connect(serialAtCommandWorker, &SerialAtCommandWorker::finished, serialAtCommandWorker, &QObject::deleteLater);

	serialAtCommandWorker->start();
}


void QcSamUnlockWindow::AtCommandUpdate(SerialAtCommandWorkerResponse response)
{
	switch (response.task) {
	case kSamsungUartTaskReadInfo:
		samsungUartParseReadInfo(response.command, response.output);
		break;
	case kSamsungUartTaskReadSpc:
		if (response.output.contains("+MEIDAUTH:1,")) {
			log(kLogTypeDebug, "MSL / SPC: " + response.output.split(",")[1]);
		}
		break;
	}
}

void QcSamUnlockWindow::AtCommandComplete(SerialAtCommandWorkerRequest request)
{
	serialAtCommandWorker = nullptr;

	disconnectPort(kPortTypeSerial);

	switch (request.task) {
	case kSamsungUartTaskReadInfo:
		saveLog("SAMSUNG UART READ INFO");
		break;
	case kSamsungUartTaskReadSpc:
		saveLog("SAMSUNG UART READ MSL / SPC");
		break;
	}
	
}

void QcSamUnlockWindow::AtCommandError(QString message)
{
	log(kLogTypeDebug, "ERROR");
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
	QString now = QDateTime::currentDateTime().toString("MM.dd.yy-hh.mm.ss ap");

	QDir dir = QDir::currentPath() + QDir::separator() + "log" + QDir::separator() + task;

	if (!dir.exists()) {
		dir.mkpath(".");
	}

	QFile file(dir.absolutePath() + QDir::separator() + now + "-openpst.log");

	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream << ui->log->toPlainText() << endl;
	}
	else {
		logAddEmptyLine();
		log(kLogTypeError, "LOG SAVE FAIL");
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
