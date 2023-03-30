#include "mainwindow.h"
using namespace QtCharts;
#include "./ui_mainwindow.h"

bool AppRunning = true;
bool dataReceived = false;



void MainWindow::NetworkCheck()
{
    qDebug() << "Packet received\r\n";
    QByteArray rawData = sock->receiveDatagram().data();
    serverData.deserialise(std::vector<uint8_t>(rawData.begin(), rawData.end()));

    if(!dataReceived)
    {
        dataReceived = true;
        QValueAxis* axisX = new QValueAxis();
        axisX->hide();
        axisX->setRange(-0.5, serverData.CoreCount+0.5f);
        ui->cpuUsage->chart()->addAxis(axisX, Qt::AlignBottom);
        cpuUsage->attachAxis(axisX);
    }

    QBarSet* cpuData = cpuUsage->barSets()[0];
    cpuData->remove(0, cpuData->count());
    for(uint64_t cpuUsage : serverData.CoreUsage)
    {
        *cpuData << (double)cpuUsage/1000.0f;
        qDebug() << (double)cpuUsage/1000.0f << "%";
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    sock = new QUdpSocket();
    sock->bind((quint16) port);
    connect(sock, &QUdpSocket::readyRead, this, &MainWindow::NetworkCheck);
    ui->setupUi(this);

    QBarSet* cpuData = new QBarSet("");
    cpuUsage = new QBarSeries();
    cpuUsage->append(cpuData);
    QChart* chart = new QChart();
    chart->addSeries(cpuUsage);
    chart->setTitle("CPU usage");
    QValueAxis* axisY = new QValueAxis();
    axisY->hide();
    axisY->setRange(0,100);
    chart->addAxis(axisY, Qt::AlignLeft);
    cpuUsage->attachAxis(axisY);
    ui->cpuUsage->setChart(chart);

}

MainWindow::~MainWindow()
{
    AppRunning = false;
    delete ui;
}

