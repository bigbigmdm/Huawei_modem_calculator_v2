#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>
#include <QMainWindow>
#include <QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <unistd.h>
#include "encrypt.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QPixmap pixGrey;
    QIcon ButtonGrey;
    QPixmap pixRed;
    QIcon ButtonRed;
    bool init;
    bool currentPortStatus;

private slots:
    void on_pushButton_clicked();
    void init9600(QString *portName);
    void close9600();
    void on_MainWindow_destroyed();
    void on_ReverseButton_clicked();
    void on_calcbutton_clicked();
    void on_pushButton_v2send_clicked();
    void on_pushButton_v1send_clicked();
    void on_pushButton_v201send_clicked();
    void on_pushButton_2_clicked();
    void on_lineEdit_editingFinished();
    void on_PortBox_currentIndexChanged(int index);
    void parsingData();
    QString portWriteAndRead(QString *writeData);
    void on_exitbutton_clicked();
    void on_centralWidget_destroyed();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
};

#endif // MAINWINDOW_H
