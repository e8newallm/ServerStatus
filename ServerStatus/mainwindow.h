#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QtConcurrent/QtConcurrent>
#include <QUdpSocket>
#include <QNetworkDatagram>

#include "../status.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void NetworkCheck();

    Status serverData;
private:
    QUdpSocket* sock;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
