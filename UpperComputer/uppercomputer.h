#ifndef UPPERCOMPUTER_H
#define UPPERCOMPUTER_H

#include <vector>
#include <QTcpSocket>
#include <QMainWindow>
#include <memory>
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class UpperComputer; }
QT_END_NAMESPACE

class UpperComputer : public QMainWindow
{
    Q_OBJECT
    //预留的多设备数据架构，但是当前仅作单设备数据记录使用
    typedef struct DeviceInfo{
        QString wifi_name;
        QString wifi_pw;
        QString ip;
        int port;
        bool connect_status = false;
    } DeviceInfo;

    enum LogType{
        no_device      = 1,
        device_connect = 2,
        device_status  = 3,
        client_log     = 4,
    };

public:
    UpperComputer(QWidget *parent = nullptr);
    ~UpperComputer();

private slots:
    void on_tcpButton_clicked();

    void on_nozzleButton_clicked();

    void on_settingButton_clicked();

    void on_connect();

    void on_receive();

    void on_disconnect();

    void on_wifiButton_clicked();

private:
    void NozzleButtonTextSet();
    void SetLog(UpperComputer::LogType lt, const QString &log="");

    void DisplayConnectStatus(bool is_connect);
    void DisplayNozzleStatus(bool is_enable);
private:
    Ui::UpperComputer *ui;

    bool _nozzle_button_status = false;
    bool _nozzle_status = false;
    bool _connect_status = false;
    bool _wifi_status = false;
    std::thread _trans_task;

    std::vector<UpperComputer::DeviceInfo> _device_list;

    std::unique_ptr<QTcpSocket> _tcp_channel;
};
#endif // UPPERCOMPUTER_H
