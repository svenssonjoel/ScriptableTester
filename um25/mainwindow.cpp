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

    unsigned char cmd[1] = {0xf0};
    mSerial->write((char *)cmd,1);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::serialReadyRead()
{
    qDebug() << "ready read";
    QByteArray data = mSerial->readAll();
    QString str = QString(data);
    qDebug() << str;
}

