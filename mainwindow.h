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

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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
    // 获取实时价格
    void requestPrice();
    // 解析响应
    void parseResponse(const QByteArray& data);
    // 汇率换算
    void translateRMB(double usdPrice);
    void updateDisplayWindow();
    void scrollAxisXYRange(qint64 time, const QLineSeries* series);

private slots:
    void onTimeout();
    // 处理时间精度
    void onIntervalChanged();

    void on_startEndBtn_clicked();

    void on_historyRecord_dateTimeChanged(const QDateTime &dateTime);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
