/**
*@file mainwindow.cpp
*@brief P500+ 恒流泵控制台函数
*包括串口初始化、刷新串口、数据处理函数
*包括数据接收、数据发送、按钮等槽函数
*@author 余明明
*@version 0.9.2
*@date 2023.04.07
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

/// @brief Mainwindow主函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SerialPortInit();
}

/// @brief 串口初始化（参数配置）
void MainWindow::SerialPortInit()
{
    serial = new QSerialPort;                       //申请内存,并设置父对象

    // 获取计算机中有效的端口号，然后将端口号的名称给端口选择控件
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        serial->setPort(info);                      // 在对象中设置串口
        if(serial->open(QIODevice::ReadWrite))      // 以读写方式打开串口
        {
            ui->PortBox->addItem(info.portName());  // 添加计算机中的端口
            serial->close();                        // 关闭
        } else
        {
            qDebug() << "串口打开失败，请重试";
        }
    }

    // 参数配置
    serial->setBaudRate(QSerialPort::Baud19200);

    serial->setParity(QSerialPort::OddParity);

    serial->setDataBits(QSerialPort::Data8);

    serial->setStopBits(QSerialPort::OneStop);

    serial->setFlowControl(QSerialPort::NoFlowControl);

    // 刷新串口
    RefreshSerialPort(0);

    // 信号
    connect(serial,&QSerialPort::readyRead,this,&MainWindow::DataReceived);      // 接收数据
    connect(ui->SendRunOrder,&QPushButton::clicked,this,&MainWindow::DataSend); // 发送数据
    connect(ui->SendStopOrder,&QPushButton::clicked,this,&MainWindow::DataSend);
    connect(ui->SendButton,&QPushButton::clicked,this,&MainWindow::DataSend);    // 发送数据
//connect(ui->SendEditBtn1,&QPushButton::clicked,this,&MainWindow::DataSend);  // 发送数据
//connect(ui->SendEditBtn2,&QPushButton::clicked,this,&MainWindow::DataSend);  // 发送数据
//connect(ui->SendEditBtn3,&QPushButton::clicked,this,&MainWindow::DataSend);  // 发送数据
}

/// @brief 刷新串口
/// @param[in] index 是否已经存在串口
void MainWindow::RefreshSerialPort(int index)
{
    QStringList portNameList;                                        // 存储所有串口名
    if(index != 0)
    {
        serial->setPortName(ui->PortBox->currentText());             //设置串口号
    }
    else
    {
        ui->PortBox->clear();                                        //关闭串口号
        ui->PortBox->addItem("刷新");                                //添加刷新
        foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()) //添加新串口
        {
            portNameList.append(info.portName());
        }
        ui->PortBox->addItems(portNameList);
        ui->PortBox->setCurrentIndex(1);                             // 当前串口号为COM1
        serial->setPortName(ui->PortBox->currentText());             //设置串口号
   }
}

/// @brief  接收数据,使用read () / readLine () / readAll ()
void MainWindow::DataReceived()
{
    QByteArray BUF[] = {0};                                       // 存储转换类型后的数据
    QByteArray data = serial->readAll();                      // 读取数据
    qDebug()<<"DataReceived:"<<data;
}

/// @brief 异或校验
/// @param[in]  buf[]   参与异或校验的全部buf
/// @param[in]  len     buf[]的长度
/// @return     CRC     异或校验值
uchar MainWindow::CRC(uchar buf[], int len)
{
    uchar CRC = 0;
    for ( int i = 0 ; i < len ; i ++ ) {
       CRC = CRC ^ buf[i];
    }
    return CRC;
}

#define Search 1
#define Control 2
#define Stop 3

QByteArray messageSend;

/// @brief 按照用户点击的按钮来处理即将发送到P500+的数据包
/// @param[in]  Function    用户选择的功能
/// @param[out] messageSend 要发送给P500+的数据包
void MainWindow::DataProess(int Function)
{
    messageSend = 0;
    switch (Function) {
    case Search:{
        uchar buf[] = {0xff, 0x11, 0x05, 0x01, 0xea};
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    case Control:{
        uchar buf[13] = {0xff, 0x11, 0x0d, 0x02};
        uint FlowRate = ui->FlowRate->text().toInt() *10;
        buf[4] = (FlowRate >> 8) & 0xff;
        buf[5] = (FlowRate >> 0) & 0xff;
        uint MaxPress = ui->MaxPress->text().toInt() *10;
        buf[6] = (MaxPress >> 8) & 0xff;
        buf[7] = (MaxPress >> 0) & 0xff;
        uint MinPress = ui->MinPress->text().toInt() *10;
        buf[8] = (MinPress >> 8) & 0xff;
        buf[9] = (MinPress >> 0) & 0xff;
        buf[10] = 0x01;
        buf[11] = 0x00;
        buf[12] = CRC(buf,sizeof (buf));
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    case Stop:{
        uchar buf[13] = {0xff, 0x11, 0x0d, 0x02};
        uint FlowRate = ui->FlowRate->text().toInt() *10;
        buf[4] = (FlowRate >> 8) & 0xff;
        buf[5] = (FlowRate >> 0) & 0xff;
        uint MaxPress = ui->MaxPress->text().toInt() *10;
        buf[6] = (MaxPress >> 8) & 0xff;
        buf[7] = (MaxPress >> 0) & 0xff;
        uint MinPress = ui->MinPress->text().toInt() *10;
        buf[8] = (MinPress >> 8) & 0xff;
        buf[9] = (MinPress >> 0) & 0xff;
        buf[10] = 0x00;
        buf[11] = 0x00;
        buf[12] = CRC(buf,sizeof (buf));
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    default:break;
    }
}

/// @brief 发送数据，write ()
void MainWindow::DataSend()
{
    qDebug()<<"DataSend:"<<messageSend;
    serial->write(messageSend);
}

/// @brief 开关显示灯
void  MainWindow::LED(bool changeColor)
{
    if(changeColor == false)
    {
        // 显示绿色
        ui->LED->setStyleSheet("background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(0, 229, 0, 255), stop:1 rgba(255, 255, 255, 255));border-radius:12px;");
    }
    else
    {
        // 显示红色
        ui->LED->setStyleSheet("background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(255, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));border-radius:12px;");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

/// @brief  串口开关
/// @post   按钮按下，若选择了串口则打开串口并亮绿灯，若未选择则弹窗提示
void MainWindow::on_OpenSerialButton_clicked()
{
    if(serial->isOpen())                                        // 如果串口打开了，先给他关闭
    {
        serial->clear();
        serial->close();
        // 关闭状态，按钮显示“打开串口”
        ui->OpenSerialButton->setText("打开串口");
        // 禁止操作“发送字符操作”
        ui->SendRunOrder->setDisabled(true);
        ui->SendStopOrder->setDisabled(true);
        ui->SendButton->setDisabled(true);
        // 关闭状态，颜色为绿色
        ui->OpenSerialButton->setStyleSheet("color: green;");
        // 关闭，显示灯为红色
        LED(true);
        // 清空数据
        ui->DataReceived->clear();
    }
    else                                                        // 如果串口关闭了，先给他打开
    {
        //当前选择的串口名字
        serial->setPortName(ui->PortBox->currentText());
        //用ReadWrite 的模式尝试打开串口，无法收发数据时，发出警告
        if(!serial->open(QIODevice::ReadWrite))
        {
            QMessageBox::warning(this,tr("提示"),tr("串口打开失败!"),QMessageBox::Ok);
            return;
         }
        // 打开状态，按钮显示“关闭串口”
        ui->OpenSerialButton->setText("关闭串口");
        // 允许操作“发送字符操作”
        ui->SendRunOrder->setDisabled(false);
        ui->SendStopOrder->setDisabled(false);
        ui->SendButton->setDisabled(false);
        // 打开状态，颜色为红色
        ui->OpenSerialButton->setStyleSheet("color: red;");
        // 打开，显示灯为绿色
        LED(false);
    }
}

/// @brief  查询按钮，向P500+发送查询指令
/// @post
/// @todo   界面应实时刷新P500+的流速、压力等查询到的信息
void MainWindow::on_SendButton_clicked()
{
    DataProess(Search);
}

// 清空接收到的数据
void MainWindow::on_ClearShowButton_clicked()
{
    ui->DataReceived->setText("");
}

/// @brief  运行按钮
/// @post   P500+开始按照UI设置数值开始运行
void MainWindow::on_SendRunOrder_clicked()
{
    DataProess(Control);
}

/// @brief  停止按钮
/// @post   P500+停止运行
void MainWindow::on_SendStopOrder_clicked()
{
    DataProess(Stop);
}
