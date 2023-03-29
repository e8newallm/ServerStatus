#include "mainwindow.h"
using namespace QtCharts;
#include "./ui_mainwindow.h"

bool AppRunning = true;

void MainWindow::NetworkCheck()
{
    qDebug() << "Packet received\r\n";
    QByteArray rawData = sock->receiveDatagram().data();
    serverData.deserialise(std::vector<uint8_t>(rawData.begin(), rawData.end()));
    qDebug() << serverData.CoreCount << "\r\n";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    sock = new QUdpSocket();
    sock->bind((quint16) port);
    connect(sock, &QUdpSocket::readyRead, this, &MainWindow::NetworkCheck);
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    AppRunning = false;
    delete ui;
}

