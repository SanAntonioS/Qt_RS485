#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void        SerialPortInit();                      // 串口初始化（参数配置）
    void        RefreshSerialPort(int index);
    void        DataProess(int Function);

public slots:
    // 串口槽函数
    void        DataReceived();                        // 接收数据
private slots:
    // 串口槽函数
    void        DataSend();                            // 发送数据
    void        on_OpenSerialButton_clicked();         // 串口开关
    void        on_SendButton_clicked();               // 控件中添加 #
    void        on_ClearShowButton_clicked();          // 清空接收到的数据
    void        LED(bool changeColor);                 // 开关显示灯
    // 点击发送，接收数据
    void        on_SendRunOrder_clicked();
    void        on_SendStopOrder_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;

    QStringList     baudList;                           //波特率
    QStringList     parityList;                         //校验位
    QStringList     dataBitsList;                       //数据位
    QStringList     stopBitsList;                       //停止位
    QStringList     flowControlList;                    //控制流
};
#endif // MAINWINDOW_H
