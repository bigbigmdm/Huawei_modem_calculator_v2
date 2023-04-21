#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString txtUSB = "USB";
    ui->setupUi(this);
         //Find available serial ports
         foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
          {
             QSerialPort serial;
             serial.setPort(info);
             if(serial.open(QIODevice::ReadWrite))
             {

                 if (serial.portName().indexOf("USB") > 0)
                 //if (1) // показать все порты
                    {
                      ui->PortBox->addItem(serial.portName());
                    }
                 serial.close();
             }
         }


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QPixmap pixGrey(":/res/img/grey_point.png");
    QIcon ButtonGrey(pixGrey);
    QPixmap pixRed(":/res/img/red_point.png");
    QIcon ButtonRed(pixRed);
    int txt_poz = 0;
    int end_poz = 0;
    QString answer;
    QString status;
    QString attempt;
    ui->pushButton->setIcon(pixRed);
    ui->pushButton->setIconSize(pixRed.rect().size());
    ui->pushButton->repaint();
    QString currPort = ui->PortBox->currentText(); //QString("ttyUSB1");
    init9600(&currPort);
    ui->textBrowser->setText("");
    ui->label_imei->setText("-");
    ui->label_version->setText("-");
    ui->label_model->setText("-");
    ui->label_manuf->setText("-");
    QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
    //serial->waitForReadyRead(800);
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("AT+CGSN\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("AT+CGMI\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("AT+CGMM\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("AT+CGMR\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("AT^CARDLOCK?\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("ATZ\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);
    serial->write("ATZ\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(800);

    serial->write("ATZ\r");
    serial->waitForBytesWritten(800);
    serial->waitForReadyRead(4000);
    close9600();
    ui->pushButton->setIcon(pixGrey);
    ui->pushButton->setIconSize(pixGrey.rect().size());
    ui->pushButton->repaint();
    //Analyse answer
    QString txtBuf = ui->textBrowser->toPlainText();
    qDebug(txtBuf.toLatin1());
    txt_poz = txtBuf.indexOf("AT+CGSN", 0);

    if ( txt_poz >= 0)
       {
       answer = txtBuf.mid(txt_poz + 9, 15);
       ui->label_imei->setText(answer);
       ui->imei->setText(answer);
       }
     txt_poz = txtBuf.indexOf("AT+CGMI", 0);
     end_poz = txtBuf.indexOf("OK", txt_poz);
     if (( txt_poz >= 0) || (end_poz >0))
       {
       answer = txtBuf.mid(txt_poz + 9, end_poz - txt_poz - 11);
       ui->label_manuf->setText(answer);
       }
     txt_poz = txtBuf.indexOf("AT+CGMM", 0);
     end_poz = txtBuf.indexOf("OK", txt_poz);
     if (( txt_poz >= 0) || (end_poz >0))
       {
       answer = txtBuf.mid(txt_poz + 9, end_poz - txt_poz - 11);
       ui->label_model->setText(answer);
       }
     txt_poz = txtBuf.indexOf("AT+CGMR", 0);
     end_poz = txtBuf.indexOf("OK", txt_poz);
     if (( txt_poz >= 0) || (end_poz >0))
       {
       answer = txtBuf.mid(txt_poz + 9, end_poz - txt_poz - 11);
       ui->label_version->setText(answer);
       }
     txt_poz = txtBuf.indexOf("^CARDLOCK:", 0);
     end_poz = txtBuf.indexOf("OK", txt_poz);
     if (( txt_poz >= 0) || (end_poz >0))
       {
       answer = txtBuf.mid(txt_poz + 11, end_poz - txt_poz - 4);
       //ui->label_attempt->setText(answer);
       end_poz = answer.indexOf(",", 0);
       status = answer.mid(0, end_poz);
       if (status == "2") status = "No";
       else status = "yes";
       ui->label_locked->setText(status);
       txt_poz = end_poz;
       txt_poz++;
       end_poz = answer.indexOf(",", txt_poz);
       attempt = answer.mid(txt_poz, end_poz - txt_poz);
       ui->label_attempt->setText(attempt);
        qDebug() << txt_poz << endl << end_poz;
       }

}

void MainWindow::Read_Data()
 {
     QByteArray buf;
     buf = serial->readAll();
     if(!buf.isEmpty())
     {
         QString str = ui->textBrowser->toPlainText();
         str+=tr(buf);
         ui->textBrowser->clear();
         ui->textBrowser->append(str);
     }
     buf.clear();
 }

void MainWindow::on_MainWindow_destroyed()
{
    serial->clear();
    serial->close();
    serial->deleteLater();
}
void MainWindow::init9600(QString *portName)
{
    serial = new QSerialPort;
    //Set the serial port name
    serial->setPortName(*portName);
    //Open the serial port
    serial->open(QIODevice::ReadWrite);
    //Set the baud rate
    serial->setBaudRate(9600);
    //Set the number of data bits
    serial->setDataBits(QSerialPort::Data8);
     //Set parity
     serial->setParity(QSerialPort::NoParity);
    //Set stop bit
    serial->setStopBits(QSerialPort::OneStop);
    //Set up flow control
    serial->setFlowControl(QSerialPort::NoFlowControl);

}
void MainWindow::close9600()
{
    sleep(2);
    serial->clear();
    serial->close();
    serial->deleteLater();
}

void MainWindow::on_ReverseButton_clicked()
{
    char imeibuf[30];
    int i;
    char c;

    strcpy(imeibuf,ui->imei->text().toLatin1());
    for (i=0;i<7;i++) {
      c=imeibuf[i];
      imeibuf[i]=imeibuf[14-i];
      imeibuf[14-i]=c;
    }
    ui->imei->setText(imeibuf);
}

void MainWindow::on_calcbutton_clicked()
{
    char imeibuf[30];
    char codebuf[30];
    QMessageBox errBox;


    strcpy(imeibuf,ui->imei->text().toLatin1());
    if (strlen(imeibuf) != 15) {
      errBox.setText("Incorrect IMEI");
      errBox.exec();
      return;
    }

    encrypt_v1(imeibuf,codebuf,"e630upgrade");
    ui->flashcode->setText(codebuf);

    encrypt_v1(imeibuf,codebuf,"hwe620datacard");
    ui->v1code->setText(codebuf);

    calc2(imeibuf,codebuf);
    ui->v2code->setText(codebuf);

    calc201(imeibuf,codebuf);
    ui->v201code->setText(codebuf);
}

// Вычисление индекса обработчика по хешу IMEI, и вызов его
//
// На входе - буфер с imei и буфер для записи результата
// Возвращает индекс обработчика
//************************************************************************
// Вычисление индекса обработчика по хешу IMEI для алгоритма v201
//
// version=2 для v2 и 201 для v201

int proc_index(char* imeibuf,int version) {

int i;
int csum=0; // хеш IMEI
int c1,ch;
long long cx;
int index;


for(i=0;i<strlen(imeibuf);i++)  {
   ch=imeibuf[i];
   if (version==201) csum+=((ch+i+1)*ch)*(ch+0x139);
   else              csum+=((ch+i+1)*(i+1));
}

cx=((long long)-0x6db6db6d*(long long)csum)>>32;
c1=((cx+csum)>>2)-(csum>>31);
index=csum-((c1<<3)-c1);
return index;
}


//**************************
// циклический сдвиг вправо
//**************************

unsigned int  rotr32(unsigned int val, int n) {
   return ((val>>n)&0x7fffffff)|(val<<(32-n));
}

//****************************************
// Вычисление кода по алгоритму 201
//****************************************

int calc201(char* imeibuf, char* resbuf) {


int index;

//
// Вычисляем хеш IMEI
//
index=proc_index(imeibuf,201);

switch (index) {
  case 0:
    encrypt_1(imeibuf,resbuf,201);
    break;
  case 1:
    encrypt_2(imeibuf,resbuf,201);
    break;
  case 2:
    encrypt_3(imeibuf,resbuf,201);
    break;
  case 3:
    encrypt_4(imeibuf,resbuf,201);
    break;
  case 4:
    encrypt_6(imeibuf,resbuf,5);
    break;
  case 5:
    encrypt_6(imeibuf,resbuf,6);
    break;
  case 6:
    encrypt_7(imeibuf,resbuf,201);
    break;
  default:
    strcpy (resbuf," - N/A -");
    break;
}

return index;
}
//****************************************
// Вычисление кода по алгоритму 2
//****************************************

int calc2(char* imeibuf, char* resbuf) {


int index;

//
// Вычисляем хеш IMEI
//
index=proc_index(imeibuf,2);

switch (index) {
  case 0:
    encrypt_1(imeibuf,resbuf,2);
    break;
  case 1:
    encrypt_2(imeibuf,resbuf,2);
    break;
  case 2:
    encrypt_3(imeibuf,resbuf,2);
    break;
  case 3:
    encrypt_4(imeibuf,resbuf,2);
    break;
  case 4:
    encrypt_5_v2(imeibuf,resbuf);
    break;
  case 5:
    encrypt_6(imeibuf,resbuf,2);
    break;
  case 6:
    encrypt_7(imeibuf,resbuf,2);
    break;
  default:
    strcpy (resbuf," - N/A -");
    break;
}



return index;
}

void MainWindow::on_pushButton_v2send_clicked()
{
    QString command;
    QString currPort = ui->PortBox->currentText();
    if (ui->v2code->text() != "")
    {
        command = "AT^CARDLOCK=\"" + ui->v2code->text() +"\"\r";
        QByteArray ba = command.toLocal8Bit();
        const char *c_str2 = ba.data();
        init9600(&currPort);
        QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(800);
        serial->write(c_str2);
        qDebug(command.toLatin1());
        serial->write("ATZ\r");
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(4000);
        serial->write("ATZ\r");
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(4000);
        close9600();
    }
}

void MainWindow::on_pushButton_v1send_clicked()
{
    QString command;
    QString currPort = ui->PortBox->currentText();
    if (ui->v2code->text() != "")
    {
        command = "AT^CARDLOCK=\"" + ui->v1code->text() +"\"\r";
        QByteArray ba = command.toLocal8Bit();
        const char *c_str2 = ba.data();
        init9600(&currPort);
        QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(800);
        serial->write(c_str2);
        qDebug(command.toLatin1());
        serial->write("ATZ\r");
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(4000);
        serial->write("ATZ\r");
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(4000);
        close9600();
    }
}

void MainWindow::on_pushButton_v201send_clicked()
{
    QString command;
    QString currPort = ui->PortBox->currentText();
    if (ui->v2code->text() != "")
    {
        command = "AT^CARDLOCK=\"" + ui->v201code->text() +"\"\r";
        QByteArray ba = command.toLocal8Bit();
        const char *c_str2 = ba.data();
        init9600(&currPort);
        QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(800);
        serial->write(c_str2);
        qDebug(command.toLatin1());
        serial->write("ATZ\r");
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(4000);
        serial->write("ATZ\r");
        serial->waitForBytesWritten(800);
        serial->waitForReadyRead(4000);
        close9600();
    }
}