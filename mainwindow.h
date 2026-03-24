#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QChart>
#include <QLineSeries>
#include <QTimer>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QLabel>
#include <QThread>
#include "worker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ExchangeRate {
public:
    // default USD
    double toUSD = 1.0;
    double toCNY;
    double toHKD;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QChart *chart;
    QLineSeries *series;
    QTimer *timer;
    QSqlDatabase db;

    QDateTimeAxis *axisX;
    QValueAxis *axisY;
    // 默认时间间隔
    int currentInterval = 3;
    // 滑动窗口显示范围
    qint64 displayWindow = 0;
    QNetworkAccessManager *manager;
    bool isWorking = false;
    QLabel* status;
    // 汇率
    ExchangeRate* rate;
    double usdPrice;
    // 子线程
    QThread* m_th;
    Worker* m_worker;

private:
    void initChart();
    void initTimer();

    void initDB();
    void initNetwork();

    void createTable();
    void insertData(qint64 timestamp, double price);
    void loadHistoryData();

    // 精度与x轴绑定
    void updateAxisFormat();
    // 根据时间精度更新图表
    void queryAndUpdateChart(qint64 time = QDateTime::currentSecsSinceEpoch());
    // 获取实时价格(转到自线程中执行)
    //void requestPrice();
    // 解析响应
    //void parseResponse(const QByteArray& data);
    // 汇率换算
    void requestExchangeRate();
    void updateDisplayWindow();
    void scrollAxisXYRange(qint64 time, const QLineSeries* series);
    // USD 转其他货币
    double USDToOther(const double usdPrice, const QString& type);

private slots:
    //void onTimeout();
    // 处理时间精度
    void onIntervalChanged();

    void on_startEndBtn_clicked();

    void on_historyRecord_dateTimeChanged(const QDateTime &dateTime);

    void on_rateCombox_currentTextChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
