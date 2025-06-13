#include "dialog.h"
#include "ui_dialog.h"
#include <QList>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDateTime>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->LCD_TEMP->display("----");
    ui->LCD_TEMP_KF->display("----");
    ui->LCD_HUM->display("----");
    ui->LCD_HUM_KF->display("----");
    this->device = new QSerialPort(this);


    temp_val = 0.0;
    hum_val = 0.0;
    temp_kf_val = 0.0;
    hum_kf_val = 0.0;

}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_SearchButton_clicked()
{
    QList<QSerialPortInfo> devices;
    devices = QSerialPortInfo::availablePorts();
    for (int i = 0; i < devices.count(); i++)
    {
        ui->textEdit->append("Searching available devices...");
        ui->textEdit->append("Found device: " + devices.at(i).portName() + "\t" + devices.at(i).description());
        ui->comboBox->addItem(devices.at(i).portName() + "\t" + devices.at(i).description());
    }
}

void Dialog::on_ConnectButton_clicked()
{
    if(ui->comboBox->count() == 0)
    {
        this->addLogs("Not found any device.");
        return;
    }


    QString portName = ui->comboBox->currentText().split("\t").first();
    this->device->setPortName(portName);

    // Open and configure available port
    if (!device->isOpen())
    {

        if (device->open(QSerialPort::ReadOnly))
        {
            this->device->setBaudRate((QSerialPort::Baud115200));
            this->device->setDataBits(QSerialPort::Data8);
            this->device->setParity(QSerialPort::NoParity);
            this->device->setStopBits(QSerialPort::OneStop);
            this->device->setFlowControl(QSerialPort::NoFlowControl);

            this->addLogs("Serial port is opened.");
            connect(this->device, SIGNAL(readyRead()), this, SLOT(readData()));

        }
        else
        {
            this->addLogs("Serial port is not opened.");
        }

   }
    else
    {
        this->addLogs("Serial port is already open.");
        return;
    }
}

void Dialog::on_DisconnectButton_clicked()
{
    if(this->device->isOpen())
    {
        this->device->close();
        this->addLogs("Serial port is close.");
    }
    else
    {
        this->addLogs("Serial port is not available more.");
        return;
    }
}

void Dialog::addLogs(QString message)
{
    QString currentDateTime = QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
    ui->textEdit->append(currentDateTime + "\t" + message);
}

void Dialog::readData()
{
    while(this->device->canReadLine())
    {
        QString line = this->device->readLine();

        QString terminator = "/";
        int position = line.lastIndexOf(terminator);
        this->addLogs(line.left(position));

        QString pos_temp= line.section("/" , 0, 0);
        QString pos_temp_KF = line.section("/", 1, 1);
        QString pos_hum = line.section("/", 2, 2);
        QString pos_hum_KF = line.section("/", 3, 3);

        Dialog::updateTemp(pos_temp);
        Dialog::updateTempKF(pos_temp_KF);
        Dialog::updateHum(pos_hum);
        Dialog::updateHumKF(pos_hum_KF);
    }
}

void Dialog::updateTemp(QString SensorReadTemp)
{
    ui->LCD_TEMP->display(SensorReadTemp);

    double DoubleTemp = QString(SensorReadTemp).toDouble();
    ui->ThermoTemp->setValue(DoubleTemp);
}

void Dialog::updateTempKF(QString SensorReadTempKF)
{
    ui->LCD_TEMP_KF->display(SensorReadTempKF);
    double DoubleTempKF = QString(SensorReadTempKF).toDouble();
    ui->ThermoTempKF->setValue(DoubleTempKF);
}
void Dialog::updateHum(QString SensorReadHum)
{
    ui->LCD_HUM->display(SensorReadHum);
    double DoubleHum = QString(SensorReadHum).toDouble();
    ui->ThermoHum->setValue(DoubleHum);
}
void Dialog::updateHumKF(QString SensorReadHumKF)
{
    ui->LCD_HUM_KF->display(SensorReadHumKF);
    double DoubleHumKF = QString(SensorReadHumKF).toDouble();
    ui->ThermoHumKF->setValue(DoubleHumKF);

}
