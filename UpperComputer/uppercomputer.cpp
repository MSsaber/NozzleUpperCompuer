#pragma execution_character_set("utf-8")
#include <QTextCodec>
#include <QMessageBox>
#include "uppercomputer.h"
#include "ui_uppercomputer.h"
#include "wlannet.h"

/*
* base16的ENCODE和DECODE
* 因为通讯目标是c51单片机，降低其他编码的长度增加程度，仅用base16
*/
int hex_digit(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  return 0;
}

//for encode base16
void ascii2hex(const char *input, uint8_t inlen, char *output, uint8_t *outlen)
{
    int now = 0;
    int output_index = 0;

    while(now < inlen)
    {
        sprintf((char*)(output+output_index),"%02X", *input);
        input++;
        output_index+=2;
        now++;
    }

    *outlen = output_index;
}

// for decode base16
void hex2ascii(const char *input, uint8_t inlen, char *output, uint8_t *outlen)
{
  int output_index = 0;

  for (size_t input_index = 0; input_index < inlen; input_index += 2){
    output[output_index] = (char)(hex_digit(input[input_index]) * 16 + hex_digit(input[input_index+1]));
    output_index++;
  }

   *outlen = output_index;
}

UpperComputer::UpperComputer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UpperComputer)
{
    ui->setupUi(this);
    _tcp_channel = std::unique_ptr<QTcpSocket>(new QTcpSocket(this));
    //绑定网络连接事件
    connect(_tcp_channel.get(), &QTcpSocket::connected, this, &UpperComputer::on_connect);
    //绑定网络数据接收事件
    connect(_tcp_channel.get(), &QTcpSocket::readyRead, this, &UpperComputer::on_receive);
    //绑定网络断开事件
    connect(_tcp_channel.get(), &QTcpSocket::disconnected, this, &UpperComputer::on_disconnect);

}

UpperComputer::~UpperComputer()
{
    _tcp_channel->disconnect();
    HostpotStop();
    delete ui;
}

void UpperComputer::on_tcpButton_clicked()
{
    if (!_wifi_status) {
        QMessageBox::critical(this, "错误", "热点未打开");
        return;
    }

    _device_list[0].ip = ui->ip_edit->toPlainText();
    _device_list[0].port = ui->port_edit->toPlainText().toInt();

    //根据当前连接状态决定tcp操作
    if (!_connect_status) {
        _tcp_channel->connectToHost(_device_list[0].ip, _device_list[0].port);
    } else {
        _tcp_channel->disconnectFromHost();
    }
}

void UpperComputer::NozzleButtonTextSet()
{
    //需要根据网络传输中实际的喷头状态设置
    QString text = _nozzle_status ? "关闭喷淋" : "开启喷淋";
    QString log_text = "设备[" + _device_list[0].wifi_name + "]"
            + QString(_nozzle_status ? " 喷淋开启" : " 喷淋关闭");

    ui->nozzleButton->setText(text);
    SetLog(client_log, log_text);
    ui->label_device_status->setText(_nozzle_status ? "运行中" : "未开启");
}

void UpperComputer::on_nozzleButton_clicked()
{
    _nozzle_button_status = !_nozzle_button_status;
    if(!_nozzle_status) {
        _tcp_channel->write("DKJDQ1");
    } else {
        _tcp_channel->write("GBJDQ1");
    }
    SetLog(client_log, !_nozzle_status ? "开启喷淋" : "关闭喷淋" );
}


void UpperComputer::on_settingButton_clicked()
{
    int8_t len = 0;
    QString temp = ui->temperature_edit->toPlainText();
    QString aq = ui->air_quality_edit->toPlainText();
    int8_t temp_int = (int8_t)temp.toInt();
    int8_t aq_int = (int8_t)aq.toInt();
    char param1[sizeof(char) * 2], param2[sizeof(char) * 2];
    ascii2hex((const char *)&temp_int, 1, param1, (uint8_t *)&len);
    ascii2hex((const char *)&aq_int, 1, param2, (uint8_t *)&len);
    QString setting_cmd = "LT+" + QString(param1) + QString(param2);

    if (!_connect_status) {
        //必须连接设备才可以操作
        QMessageBox::critical(this, "错误", "设备未连接");
        return;
    }

    QString log_text = "设备[" + _device_list[0].wifi_name + "] 设置阈值 -- 温度:" + temp + " , 空气质量:" + aq;
    SetLog(client_log, setting_cmd);

    _tcp_channel->write(setting_cmd.toLatin1());
    qDebug() << "发送设置数据:" + setting_cmd;
}

void UpperComputer::SetLog(UpperComputer::LogType lt, const QString &log)
{
    QString log_text = log;
    switch (lt) {
    case no_device:
        log_text = "无设备连接";
        break;
    case device_connect:
        if (!_device_list.size()) {
            log_text = "未检测到设备,程序异常,请重新添加设备";
        } else {
            log_text = "设备[" + _device_list[0].ip + "] 连接成功";
        }
        break;
    case device_status:
    case client_log:
    default:
        break;
    }
    ui->label_log->setText(log_text);
}

void UpperComputer::DisplayConnectStatus(bool is_connect)
{
    QString text = is_connect ? "已连接" : "未连接";
    ui->label_connect_status->setText(text);
}

void UpperComputer::DisplayNozzleStatus(bool is_enable)
{
    QString text = is_enable ? "已开启" : "未开启";
    ui->label_device_status->setText(text);
}

void UpperComputer::on_connect()
{
    this->_connect_status = true;
    DisplayConnectStatus(this->_connect_status);
    ui->tcpButton->setText("断开连接");
    ui->label_connect_status->setText("已连接");
    SetLog(device_connect);
    qDebug() << "连接设备";
}

void UpperComputer::on_receive()
{
    QTextCodec *tc = QTextCodec::codecForName("GBK");
    QByteArray msg = _tcp_channel->readAll();
    QString data = tc->toUnicode(msg);
    QStringList data_list = data.split("\r");

    qDebug() << data;
    qDebug() << data_list;

    if (data_list.length() > 1) {
        //多行数据为状态数据
        //0 温度 1 湿度 2 空气质量 3 电机状态(1开、0关)
        QStringList status_info = data_list[0].split(";");
        QStringList limit_info = data_list[1].split(";");
        //显示传感器数据
        ui->label_temperature->setText(status_info[0].split(":")[1]);
        ui->label_humidity->setText(status_info[1].split(":")[1]);
        ui->label_air_quality->setText(status_info[2].split(":")[1]);
        //记录电机状态
        _nozzle_status = status_info[3].split(":")[1] == "0" ? false : true;
        //显示阈值数据
        ui->label_temperature_limit->setText(limit_info[0].split(":")[1]);
        ui->label_air_quality_limit->setText(limit_info[2].split(":")[1]);
        qDebug() << status_info;
        qDebug() << limit_info;
    } else if (data_list.length() == 1) {
        QStringList nozzle_op_res = data_list[0].split("-");
        //参数错误 目标回复RT+OK:JDQ1-[状态]
        if (nozzle_op_res.length() != 2) {
            return;
        }
        //根据电机回复状态设置_nozzle_status
        if (nozzle_op_res[1] == "OPEN") {
            _nozzle_status = true;
        } else {
            _nozzle_status = false;
        }
    }
    NozzleButtonTextSet();
}

void UpperComputer::on_disconnect()
{
    this->_connect_status = false;
    DisplayConnectStatus(this->_connect_status);
    ui->tcpButton->setText("连接");
    ui->label_connect_status->setText("未连接");
    SetLog(no_device);
    qDebug() << "断开连接";
}

void UpperComputer::on_wifiButton_clicked()
{
    UpperComputer::DeviceInfo di;

    if (!_wifi_status) {
        _wifi_status = true;
        di.wifi_name = ui->wifi_name_edit->toPlainText();
        di.wifi_pw = ui->wifi_pw_edit->toPlainText();
        //打开热点
        SettingHostpot(di.wifi_name,di.wifi_pw);
        HostpotStart();

        //清空容器
        _device_list.clear();
        //添加设备到容器
        _device_list.push_back(di);

        //改变控件的文字
        ui->wifiButton->setText("关闭热点");
        ui->label_wifi_status->setText("已打开");
        qDebug() << "打开热点 ：" << di.wifi_name;
    } else {
        _wifi_status = false;
        _connect_status = false;
        //先关闭网络
        _tcp_channel->disconnect();
        ui->tcpButton->setText("连接");
        ui->label_connect_status->setText("未连接");
        SetLog(no_device);
        //关闭热点
        HostpotStop();

        //改变控件的文字
        ui->wifiButton->setText("开启热点");
        ui->label_wifi_status->setText("未打开");
        qDebug() << "关闭热点 ：" << _device_list[0].wifi_name;
    }
}

