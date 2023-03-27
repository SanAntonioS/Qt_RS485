#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SerialPortInit();
}

// 串口初始化（参数配置）
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
    connect(ui->SendWordOrder,&QPushButton::clicked,this,&MainWindow::DataSend); // 发送数据
    connect(ui->SendButton,&QPushButton::clicked,this,&MainWindow::DataSend);    // 发送数据
//connect(ui->SendEditBtn1,&QPushButton::clicked,this,&MainWindow::DataSend);  // 发送数据
//connect(ui->SendEditBtn2,&QPushButton::clicked,this,&MainWindow::DataSend);  // 发送数据
//connect(ui->SendEditBtn3,&QPushButton::clicked,this,&MainWindow::DataSend);  // 发送数据
}

// 刷新串口
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

// 接收数据,使用read () / readLine () / readAll ()
void MainWindow::DataReceived()
{
    char BUF[512] = {0};                                       // 存储转换类型后的数据
    QByteArray data = serial->readAll();                      // 读取数据
    qDebug()<<"DataReceived:"<<data;
    if(!data.isEmpty())                                 // 接收到数据
    {
        if(data == "\xFF\x11\r\x02\xF9")
        {
            qDebug()<<"Control command send successfuly";
        }
        else if(data.at(3) == 0x01)
        {
            qDebug()<<"Recieve correct data";
        }
        QString str = ui->DataReceived->toPlainText();  // 返回纯文本
        str += tr(data);                                // 数据是一行一行传送的，要保存所有数据
        ui->DataReceived->clear();                      // 清空之前的数据
        ui->DataReceived->append(str);                  // 将数据放入控件中
        qDebug() << "str info: " << ui->DataReceived->toPlainText();

         // 清除之前的数据，防止追加，因为每次获取的数据不一样
        int index = str.indexOf("\r\n");                // 找到，返回索引值，找不到，返回-1
        if(index != -1)
        {
            snprintf(BUF,500,"%s", str.left(index + 2).toUtf8().data()); // QString转为char * 类型
            qDebug() << "BUF info: " << BUF;
            str.remove(0,index + 2);

            // 处理获取到的数据，将其放入对应的控件中
            // ....
        }
    }
}

#define Search 1
#define Control 2

QByteArray messageSend;

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
        uchar CRC = 0;
        for ( int i = 0 ; i < 12 ; i ++ ) {
           CRC = CRC ^ buf[i];
        }
        buf[12] = CRC;
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    default:break;
    }
}

// 发送数据，write ()
void MainWindow::DataSend()
{
    qDebug()<<"DataSend:"<<messageSend;
    serial->write(messageSend);
}

// 开关显示灯
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

// 串口开关
void MainWindow::on_OpenSerialButton_clicked()
{
    if(serial->isOpen())                                        // 如果串口打开了，先给他关闭
    {
        serial->clear();
        serial->close();
        // 关闭状态，按钮显示“打开串口”
        ui->OpenSerialButton->setText("打开串口");
        // 禁止操作“发送字符操作”
        ui->SendWordOrder->setDisabled(true);
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
        ui->SendWordOrder->setDisabled(false);
        ui->SendButton->setDisabled(false);
        // 打开状态，颜色为红色
        ui->OpenSerialButton->setStyleSheet("color: red;");
        // 打开，显示灯为绿色
        LED(false);
    }
}

// 控件中添加 #
void MainWindow::on_SendButton_clicked()
{
    DataProess(Search);
}

// 清空接收到的数据
void MainWindow::on_ClearShowButton_clicked()
{
    ui->DataReceived->setText("");
}

void MainWindow::on_SendWordOrder_clicked()
{
    DataProess(Control);
}
