#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPort>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mSerial = new QSerialPort(this);

    connect(mSerial, &QSerialPort::readyRead,
                this, &MainWindow::serialReadyRead);

    mSerial->setPortName("/dev/rfcomm2");
    mSerial->setBaudRate(QSerialPort::Baud9600);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setStopBits(QSerialPort::OneStop);
    mSerial->setFlowControl(QSerialPort::NoFlowControl);

    if(mSerial->open(QIODevice::ReadWrite)) {
        qDebug() << "SERIAL: OK!";
    } else {
        qDebug() << "SERIAL: ERROR!";
    }

    mPacket.clear();

    unsigned char cmd[1] = {0xf0};
    mSerial->write((char *)cmd,1);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::serialReadyRead()
{
    QByteArray data = mSerial->readAll();

    mPacket.append(data);

    /* assumes perfect transmissions... (not ok) */

    if(mPacket.size() == 130) {
        QString str = QString(mPacket);
        //qDebug() << str;

        parsePacket();

        mPacket.clear();

        unsigned char cmd[1] = {0xf0};
        mSerial->write((char *)cmd,1);
    }
}

void MainWindow::parsePacket() {

    float v = 0;
    float a = 0;
    float w = 0;

    unsigned int tmp = 0 ;
    tmp = (unsigned char)mPacket[2];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[3];
    v = (float)tmp / 1000.0;

    tmp = 0;
    tmp = (unsigned char)mPacket[4];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[5];
    a = (float)tmp / 10000.0;

    qDebug() << "volts: " << QString::number(v) << " amps: " << QString::number(a);
}


/* from: https://sigrok.org/wiki/RDTech_UM_series#Commands_to_send */
/*
Offset 	Length 	Type 	Meaning
0 	2 	model 	Model ID (see below)
2 	2 	measurement 	Voltage - UM25C: millivolts (divide by 1000 to get V), UM24C/UM34C: centivolts (divide by 100 to get V)
4 	2 	measurement 	Amperage - UM25C tenth-milliamps (divide by 10000 to get A), UM24C/UM34C: milliamps (divide by 1000 to get A)
6 	4 	measurement 	Wattage (in mW, divide by 1000 to get W)
10 	2 	measurement 	Temperature (in Celsius)
12 	2 	measurement 	Temperature (in Fahrenheit)
14 	2 	configuration 	Currently selected data group, zero-indexed
16 	80 	measurement 	Array of 10 main capacity data groups (where the first one, group 0, is the ephemeral one) -- for each data group: 4 bytes mAh, 4 bytes mWh
96 	2 	measurement 	USB data line voltage (positive) in centivolts (divide by 100 to get V)
98 	2 	measurement 	USB data line voltage (negative) in centivolts (divide by 100 to get V)
100 	2 	measurement 	Charging mode index, see below
102 	4 	measurement 	mAh from threshold-based recording
106 	4 	measurement 	mWh from threshold-based recording
110 	2 	configuration 	Currently configured threshold for recording (in centiamps, divide by 100 to get A)
112 	4 	measurement 	Duration of threshold recording, in cumulative seconds
116 	2 	configuration 	Threshold recording active (1 if recording, 0 if not)
118 	2 	configuration 	Current screen timeout setting, in minutes (0-9, 0 is no screen timeout)
120 	2 	configuration 	Current backlight setting (0-5, 0 is dim, 5 is full brightness)
122 	4 	measurement 	Resistance in deci-ohms (divide by 10 to get ohms)
126 	2 	configuration 	Current screen (zero-indexed, same order as on device)
128 	1 	unknown 	See below
129 	1 	checksum/unknown 	Checksum (UM34C) or unknown. See below.
*/
