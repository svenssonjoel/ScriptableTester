#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPort>
#include <QDebug>
#include <QDateTime>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mLogging = false;
    mSample = 0;
    mSampling = false;
    mScriptRunning = false;

    mSerial = new QSerialPort(this);
    mTesterSerial = new QSerialPort(this);

    connect(mSerial, &QSerialPort::readyRead,
                this, &MainWindow::serialReadyRead);

    connect(&mScriptTimer, &QTimer::timeout,
            this, &MainWindow::scriptTimerTimeout);

    mPacket.clear();

    initPlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initPlots()
{
    mVoltData = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
    mAmpData = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
    mWattData = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);

    /* Setup Volt plot */
    ui->voltPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->voltPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->voltPlot->legend->setFont(legendFont);
    ui->voltPlot->legend->setSelectedFont(legendFont);
    ui->voltPlot->legend->setSelectableParts(QCPLegend::spItems);
    ui->voltPlot->yAxis->setLabel("Volt");
    ui->voltPlot->xAxis->setLabel("Sample");
    ui->voltPlot->clearGraphs();
    ui->voltPlot->addGraph();

    ui->voltPlot->graph()->setPen(QPen(Qt::black));
    ui->voltPlot->graph()->setData(mVoltData);
    ui->voltPlot->graph()->setName("Volt");

    /* Setup Amp plot */
    ui->ampPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->ampPlot->legend->setVisible(true);
    //QFont legendFont = font();
    //legendFont.setPointSize(10);
    ui->ampPlot->legend->setFont(legendFont);
    ui->ampPlot->legend->setSelectedFont(legendFont);
    ui->ampPlot->legend->setSelectableParts(QCPLegend::spItems);
    ui->ampPlot->yAxis->setLabel("A");
    ui->ampPlot->xAxis->setLabel("Sample");
    ui->ampPlot->clearGraphs();
    ui->ampPlot->addGraph();

    ui->ampPlot->graph()->setPen(QPen(Qt::black));
    ui->ampPlot->graph()->setData(mAmpData);
    ui->ampPlot->graph()->setName("Ampere");

    /* Setup Watt plot */
    ui->wattPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->wattPlot->legend->setVisible(true);
    //QFont legendFont = font();
    //legendFont.setPointSize(10);
    ui->wattPlot->legend->setFont(legendFont);
    ui->wattPlot->legend->setSelectedFont(legendFont);
    ui->wattPlot->legend->setSelectableParts(QCPLegend::spItems);
    ui->wattPlot->yAxis->setLabel("W");
    ui->wattPlot->xAxis->setLabel("Sample");
    ui->wattPlot->clearGraphs();
    ui->wattPlot->addGraph();

    ui->wattPlot->graph()->setPen(QPen(Qt::black));
    ui->wattPlot->graph()->setData(mWattData);
    ui->wattPlot->graph()->setName("Watts");


}


void MainWindow::updateSerialPorts()
{
    mSerialPorts = QSerialPortInfo::availablePorts();

    ui->devSelectComboBox->clear();
    for (QSerialPortInfo port : mSerialPorts) {
        ui->devSelectComboBox->addItem(port.portName(), port.systemLocation());
    }

    ui->testerSelectComboBox->clear();
    for (QSerialPortInfo port : mSerialPorts) {
        ui->testerSelectComboBox->addItem(port.portName(), port.systemLocation());
    }
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

        if (mSampling || mScriptRunning) {
            unsigned char cmd[1] = {0xf0};
            mSerial->write((char *)cmd,1);
        }
    }
}

void MainWindow::parsePacket() {

    //qDebug() << "parsePacket mSample: " << mSample;

    float v = 0;
    float a = 0;
    float w = 0;

    unsigned int tmp = 0 ;
    tmp = (unsigned char)mPacket[2];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[3];
    v = (float)tmp / 1000.0;

    mVoltData->add(QCPGraphData(mSample, v));
    ui->voltPlot->rescaleAxes();
    //ui->voltPlot->yAxis->rescale();
    //ui->voltPlot->yAxis->setRange(0,10);
    ui->voltPlot->replot();

    tmp = 0;
    tmp = (unsigned char)mPacket[4];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[5];
    a = (float)tmp / 10000.0;

    mAmpData->add(QCPGraphData(mSample, a));
    ui->ampPlot->rescaleAxes();
    //ui->ampPlot->yAxis->rescale();
    //ui->voltPlot->yAxis->setRange(0,10);
    ui->ampPlot->replot();

    tmp = 0;
    tmp = (unsigned char)mPacket[6];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[7];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[8];
    tmp = tmp << 8;
    tmp += (unsigned char)mPacket[9];
    w = (float)tmp / 1000.0;

    mWattData->add(QCPGraphData(mSample, w));
    ui->wattPlot->rescaleAxes();
    ui->wattPlot->yAxis->rescale();
    //ui->wattPlot->xAxis->tra
    //ui->voltPlot->yAxis->setRange(0,10);
    ui->wattPlot->replot();

    mSample +=1;
    if (mLogging) {
        QString str = QString("volts: " + QString::number(v) + " amps: " + QString::number(a) + " Watts: " + QString::number(w) + "\n");

        ui->logTextBrowser->insertPlainText(str);
        QScrollBar *sb = ui->logTextBrowser->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
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

void MainWindow::on_devRefreshPushButton_clicked()
{
     updateSerialPorts();
}

void MainWindow::on_devConnectPushButton_clicked()
{
    ui->devConnectPushButton->setEnabled(false);
    //QString serialName =  ui->serialComboBox->currentText();
    QString serialLoc  =  ui->devSelectComboBox->currentData().toString();

    if (serialLoc.isEmpty()) {
        ui->devConnectPushButton->setEnabled(true);
        return;
    }
    if (mSerial->isOpen()) {
        qDebug() << "Serial already connected, disconnecting!";
        mSerial->close();
    }

    mSerial->setPortName(serialLoc);
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

    unsigned char cmd[1] = {0xf0};
    mSerial->write((char *)cmd,1);

    ui->devConnectPushButton->setEnabled(true);
}

void MainWindow::testerSerialReadyRead()
{
    QByteArray data = mTesterSerial->readAll();
    QString str = QString(data);

    //ui->logTextBrowser->insertPlainText(str);
    //QScrollBar *sb = ui->logTextBrowser->verticalScrollBar();
    //sb->setValue(sb->maximum());
    qDebug() << QString(data);
}

void MainWindow::on_testerConnectPushButton_clicked()
{
    ui->testerConnectPushButton->setEnabled(false);
    //QString serialName =  ui->serialComboBox->currentText();
    QString serialLoc  =  ui->testerSelectComboBox->currentData().toString();

    if (serialLoc.isEmpty()) {
        ui->testerConnectPushButton->setEnabled(true);
        return;
    }
    if (mTesterSerial->isOpen()) {
        qDebug() << "Serial already connected, disconnecting!";

        disconnect(mTesterSerial, SIGNAL(readyRead()), 0, 0);
        mTesterSerial->close();
    }

    mTesterSerial->setPortName(serialLoc);
    mTesterSerial->setBaudRate(QSerialPort::Baud115200);
    mTesterSerial->setDataBits(QSerialPort::Data8);
    mTesterSerial->setParity(QSerialPort::NoParity);
    mTesterSerial->setStopBits(QSerialPort::OneStop);
    mTesterSerial->setFlowControl(QSerialPort::NoFlowControl);

    if(mTesterSerial->open(QIODevice::ReadWrite)) {
        qDebug() << "SERIAL: OK!";
    } else {
        qDebug() << "SERIAL: ERROR!";
    }

    ui->testerStatusLabel->setText("Sending: init");
    mTesterSerial->write("init\n");

    mTesterSerial->waitForReadyRead();
    QByteArray response = mTesterSerial->readAll();
    QString rstr = QString(response);
    if (rstr == "OK!\n") {
        ui->testerStatusLabel->setText("Tester OK!");

        connect(mTesterSerial, &QSerialPort::readyRead,
                    this, &MainWindow::testerSerialReadyRead);

    } else {
        ui->testerStatusLabel->setText("Error: " + QString(rstr));
    }
    ui->testerConnectPushButton->setEnabled(true);
}

void MainWindow::on_testerRefreshPushButton_clicked()
{
    updateSerialPorts();
}

void MainWindow::on_runScriptPushButton_clicked()
{
    QString script_txt = ui->scriptPlainTextEdit->toPlainText();
    mScript = script_txt.split("\n");

    mScriptTimer.setTimerType(Qt::PreciseTimer);
    mScriptTimer.setSingleShot(true);
    mScriptTimer.setInterval(0);
    mScriptTimer.start();

    ui->sampleRadioButton->toggled(true); // maybe control with an instr in the script?
    ui->sampleRadioButton->setChecked(true);
    ui->sampleRadioButton->setEnabled(false);
}

void MainWindow::finishScript()
{
    /* restore gui state */
    ui->sampleRadioButton->toggled(false);
    ui->sampleRadioButton->setChecked(false);
    ui->sampleRadioButton->setEnabled(true);

    return;
}

void MainWindow::scriptTimerTimeout()
{
    if (mScript.isEmpty()) {
        finishScript();
        return; /* maybe set script done? */
    }

    QString instr = mScript.takeFirst();

    if (instr.startsWith("SLEEP",Qt::CaseInsensitive)) {
        QStringList sl = instr.split(" ", QString::SkipEmptyParts);

        bool ok = false;
        int m = sl.at(1).toInt(&ok);

        if (ok) {
            mScriptTimer.setInterval(m);
            mScriptTimer.start();
        } else {
            qDebug() << "Error in script";
        }
        return;
    } else if (instr.startsWith("CLEARGRAPHS",Qt::CaseInsensitive)) {
        ui->voltPlot->graph(0)->data()->clear();
        ui->ampPlot->graph(0)->data()->clear();
        ui->wattPlot->graph(0)->data()->clear();
        //qDebug() << "Setting mSample";
        mSample = 0;

        //qDebug() << "mSample = " << mSample;
        mScriptTimer.setInterval(0);
        mScriptTimer.start();
        return;
    } else {
        if (mTesterSerial->isOpen()) {

            QString str = instr + QString("\n");
            mTesterSerial->write(str.toUpper().toLocal8Bit());
            mScriptTimer.setInterval(0);
            mScriptTimer.start();
            return;
        } else {
            qDebug () << "Error: Tester connection not open.";
            return;
        }
    }
}

void MainWindow::on_loadScriptPushButton_clicked()
{
    qDebug() << "Load: Not implemented";
}

void MainWindow::on_saveScriptPushButton_clicked()
{
    qDebug() << "Save: Not implemented";
}

void MainWindow::on_logRadioButton_toggled(bool checked)
{
    mLogging = checked;
}

void MainWindow::on_sampleRadioButton_toggled(bool checked)
{
    mSampling = checked;
    unsigned char cmd[1] = {0xf0}; /* start sampling */
    mSerial->write((char *)cmd,1);
}
