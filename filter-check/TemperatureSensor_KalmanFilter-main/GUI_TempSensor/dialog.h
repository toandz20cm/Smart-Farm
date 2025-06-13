#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_SearchButton_clicked();
    void on_ConnectButton_clicked();
    void on_DisconnectButton_clicked();
    void addLogs(QString message);

    void readData();

    void updateTemp(QString SensorReadTemp);
    void updateTempKF(QString SensorReadTempKF);

    void updateHum(QString SensorReadHum);
    void updateHumKF(QString SensorReadHumKF);

private:
    Ui::Dialog *ui;
    QSerialPort *device;

    float temp_val, hum_val;
    float temp_kf_val, hum_kf_val;

};
#endif // DIALOG_H
