#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    pixGrey.load(":/res/img/grey_point.png");
    pixRed.load(":/res/img/red_point.png");
    ButtonGrey.addPixmap(pixGrey);
    ButtonRed.addPixmap(pixRed);

    ui->setupUi(this);
    currentPortStatus = false;
    init = true;
         //Find available serial ports
         ui->PortBox->addItem("---");
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
         ui->pushButton->setIcon(pixGrey);
         ui->pushButton->setIconSize(pixGrey.rect().size());
         init = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->pushButton->setIcon(pixRed);
    ui->pushButton->setIconSize(pixRed.rect().size());
    ui->pushButton->repaint();
    QString command;
    //QString currPort = ui->PortBox->currentText(); //QString("ttyUSB1");
    //init9600(&currPort);
    ui->imei->setText("");
    ui->textBrowser->setText("");
    ui->label_imei->setText("-");
    ui->label_version->setText("-");
    ui->label_model->setText("-");
    ui->label_manuf->setText("-");
    ui->label_locked->setText("-");
    ui->label_attempt->setText("-");

    command = "ATE1\r";
    ui->textBrowser->append(portWriteAndRead(&command));
    command = "AT+CGSN\r";
    ui->textBrowser->append(portWriteAndRead(&command));
    command = "AT+CGMI\r";
    ui->textBrowser->append(portWriteAndRead(&command));
    command = "AT+CGMM\r";
    ui->textBrowser->append(portWriteAndRead(&command));
    command = "AT+CGMR\r";
    ui->textBrowser->append(portWriteAndRead(&command));
    command = "AT^CARDLOCK?\r";
    ui->textBrowser->append(portWriteAndRead(&command));
    ui->textBrowser->scroll(0,1000);
    parsingData();

    //close9600();
    ui->pushButton->setIcon(pixGrey);
    ui->pushButton->setIconSize(pixGrey.rect().size());
    ui->pushButton->repaint();
}

QString MainWindow::portWriteAndRead(QString *writeData)
{
    int index = ui->PortBox->currentIndex();
    QString readData ="";
    if (index == 0)
    {
        QMessageBox::about(this, "Error", "Please select correct USB port number!");
                return readData;
    }
    if (currentPortStatus)
       {
       QByteArray ba = writeData->toLocal8Bit();
       const char *str = ba.data();
       serial->write(str);
       serial->waitForBytesWritten(800);
       while (serial->waitForReadyRead(10))
          {
             readData.append(serial->readAll());
          }
       return readData;
       }
    else return QString("");
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
    currentPortStatus = serial->open(QIODevice::ReadWrite);
    //Set the baud rate
    serial->setBaudRate(9600);
    //Set the number of data bits
    serial->setDataBits(QSerialPort::Data8);
     //Set parity
     serial->setParity(QSerialPort::NoParity);
    //Set stop bit
    serial->setStopBits(QSerialPort::OneStop);
    //Set up flow control
    //serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->setFlowControl( QSerialPort::SoftwareControl );
}

void MainWindow::close9600()
{
    if (currentPortStatus)
    {
    serial->clear();
    serial->close();
    serial->deleteLater();
    sleep(1);
    currentPortStatus = false;
    }
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
        //init9600(&currPort);
        ui->textBrowser->append(portWriteAndRead(&command));
        ui->textBrowser->scroll(0,1000);
        //close9600();
    }
}

void MainWindow::on_pushButton_v1send_clicked()
{
    QString command;
    QString currPort = ui->PortBox->currentText();
    if (ui->v2code->text() != "")
    {
        command = "AT^CARDLOCK=\"" + ui->v1code->text() +"\"\r";
        //init9600(&currPort);
        ui->textBrowser->append(portWriteAndRead(&command));
        ui->textBrowser->scroll(0,1000);
        //close9600();
    }
}

void MainWindow::on_pushButton_v201send_clicked()
{
    QString command;
    QString currPort = ui->PortBox->currentText();
    if (ui->v2code->text() != "")
    {
        command = "AT^CARDLOCK=\"" + ui->v201code->text() +"\"\r";
        //init9600(&currPort);
        ui->textBrowser->append(portWriteAndRead(&command));
        ui->textBrowser->scroll(0,1000);
        //close9600();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QString command;
    QMessageBox::StandardButton reply;
    QString currPort = ui->PortBox->currentText();
    command = "at^nvwrex=8268,0,12,1,0,0,0,2,0,0,0,a,0,0,0\r";
    reply = QMessageBox::question(this, "AT^NVWREX commant", "The AT^NVWREX command can destroy your device!\nAre you sure you want to run this command? ",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
      //init9600(&currPort);
      ui->textBrowser->append(portWriteAndRead(&command));
      ui->textBrowser->scroll(0,1000);
      //close9600();
    }
}

void MainWindow::on_lineEdit_editingFinished()
{
    QString currPort = ui->PortBox->currentText();
    QString command = ui->lineEdit->text() + "\r";
    //init9600(&currPort);
    ui->textBrowser->append(portWriteAndRead(&command));
    ui->textBrowser->scroll(0,1000);
    //close9600();
}

void MainWindow::on_PortBox_currentIndexChanged(int index)
{
    if (init == false)
    {

       if (index != 0)
       {
           QString currPort = ui->PortBox->currentText();
           init9600(&currPort);
       }
    }
    ui->textBrowser->clear();
}


void MainWindow::parsingData()
{
    int txt_poz = 0;
    int end_poz = 0;
    QString answer = "";
    QString status;
    QString attempt;
    QString txtV1 = " E122, E150, E155, E156, E156G, E160, E160G, E161, E166, E169, E169G, E170, E171, E171u-1, E172, E176, E180, E182E, E196, E226, E270, E271, E272, E510, E612, E618, E620, E630, E630+, E660, E660A, E800, E870, E880, EG162, EG162G, EG602, EG602G, E1550, E1750, Vodafone K2540, Vodafone K3515, Vodafone K3520, Vodafone K3565, Vodafone K3715,";
    QString txtV2 = " B260, B593, B593s-22, B970, B970b, E173, E173U-1, E303, E303s-1, E325, E352, E352B, E352u-2C, E353, E353s-2, E353Au-2, E355, E357, E367, E369, E392, E586, E586-50e5, E3276, E3121, E3131, E3131S-1, E3131S-2, E3131s-1EW, E3131s-2EW, E3372, E5172, E5331s-2, E5776, E5576S, E5776S-601, 821FT, 822FT, М150-1, B970b, 320D, 320S, 321S, 420S,";
    QString txtV201 = " E320D, E3272, E3370, E3372, E3531, E5330, E5331, E5330Bs-2, E5372, E8231, E8278, E8372H, N100-4, M100-4, MR100-3, 423S, M21-4, 423S, 424D, 823F, 824F, 825FT, 826FT, 827F, 828FT,";
    QString searchModel ="";
    QFont fontBold, fontNormal;
    fontBold.setWeight(QFont::ExtraBold);
    fontNormal.setWeight(QFont::Normal);
    QString txtBuf = ui->textBrowser->toPlainText();
    //qDebug() << txtBuf.toLatin1();
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
       }
       //highlighting the recommended unlock code
       searchModel = " " + ui->label_model->text() + ",";
       if (txtV1.indexOf(searchModel) >= 0) ui->label_txtV1->setFont(fontBold);
       else ui->label_txtV1->setFont(fontNormal);
       if (txtV2.indexOf(searchModel) >= 0) ui->label_txtV2->setFont(fontBold);
       else ui->label_txtV2->setFont(fontNormal);
       if (txtV201.indexOf(searchModel) >= 0) ui->label_txtV201->setFont(fontBold);
       else ui->label_txtV201->setFont(fontNormal);
}

void MainWindow::on_exitbutton_clicked()
{
    close9600();
}

void MainWindow::on_centralWidget_destroyed()
{
    close9600();
}

void MainWindow::on_pushButton_3_clicked()
{
    close9600();
    ui->PortBox->clear();
         //Find available serial ports
         ui->PortBox->addItem("---");
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
