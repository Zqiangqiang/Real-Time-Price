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

private:
    void initChart();
    void initTimer();

    void initDB();
    void createTable();
    void insertData(qint64 timestamp, double price);
    void loadHistoryData();

    // 精度与x轴绑定
    void updateAxisFormat();
    // 根据时间精度更新图表
    void queryAndUpdateChart();


private slots:
    void onTimeout();
    // 处理时间精度
    void onIntervalChanged();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
