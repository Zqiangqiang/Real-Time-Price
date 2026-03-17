#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    initChart();
    initDB();
    loadHistoryData(); // 加载历史
    if (!series->points().isEmpty()) {
        // 初始化x轴范围
        qint64 first = series->points().first().x() / 1000;
        qint64 last  = series->points().last().x() / 1000;

        axisX->setRange(
            QDateTime::fromSecsSinceEpoch(first),
            QDateTime::fromSecsSinceEpoch(last)
        );
    }
    initTimer();

    connect(ui->accuracyCombo, &QComboBox::currentTextChanged, this, &MainWindow::onIntervalChanged);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initDB()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Price.db");
    if (!db.open()) {
        qDebug() << "open database failed " + db.lastError().databaseText();
    }
    createTable();
}

void MainWindow::createTable()
{
    QSqlQuery query;
    // 表名为 price_data
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS price_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER,
            price REAL
        )
    )";

    if (!query.exec(sql)) {
        qDebug() << "Create table failed:" << query.lastError();
    }
}

void MainWindow::insertData(qint64 timestamp, double price)
{
    QSqlQuery query;

    query.prepare("INSERT INTO price_data (timestamp, price) VALUES (?, ?)");

    query.addBindValue(timestamp);
    query.addBindValue(price);

    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError();
    }
}

void MainWindow::loadHistoryData()
{
    QSqlQuery query("SELECT timestamp, price FROM price_data ORDER BY timestamp ASC");

    while (query.next()) {
        qint64 t = query.value(0).toLongLong();
        double price = query.value(1).toDouble();

        series->append(t * 1000, price);
    }
}

void MainWindow::updateAxisFormat()
{
    if (currentInterval <= 60) {
        axisX->setFormat("hh:mm:ss");
    }
    else if (currentInterval <= 1800) {
        axisX->setFormat("hh:mm");
    }
    else if (currentInterval <= 3600) {
        axisX->setTickCount(7);
        axisX->setFormat("MM-dd hh");
    }
    else {
        // 时间跨度大是可以调整间隔
        axisX->setFormat("yyyy-MM-dd");
    }
}

void MainWindow::queryAndUpdateChart()
{

    /*
     * 聚合公式 (timestamp / interval) * interval
        SELECT
            (timestamp / 60) * 60 AS t,
            AVG(price)
        FROM price_data
        GROUP BY t
        ORDER BY t
    */

    series->clear();

    QString sql = QString(R"(
        SELECT
            (timestamp / %1) * %1 AS t,
            AVG(price)
        FROM price_data
        GROUP BY t
        ORDER BY t
    )").arg(currentInterval);

    QSqlQuery query(sql);

    while (query.next()) {
        qint64 t = query.value(0).toLongLong();
        double price = query.value(1).toDouble();
        // 区间内点平均值
        series->append(t * 1000, price);
    }

    // 更新X轴范围
    if (!series->points().isEmpty()) {
        qint64 first = series->points().first().x() / 1000;
        qint64 last  = series->points().last().x() / 1000;

        axisX->setRange(
            QDateTime::fromSecsSinceEpoch(first),
            QDateTime::fromSecsSinceEpoch(last)
        );
    }
}

void MainWindow::initChart()
{
    // 初始化图表
    series = new QLineSeries();
    series->setName("Gold");

    axisX = new QDateTimeAxis();
    axisY = new QValueAxis();
    chart = new QChart();
    // 修改横轴格式
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText("Time");
    // 修改纵轴格式
    axisY->setTitleText("Price");
    // 设置y轴范围，否则超出点不显示
    axisY->setRange(80, 140);

    // 先加series
    chart->addSeries(series);
    // 再加轴
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    // 最后attach
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->setTitle("Real-time Price");

    // 把 chart 设置给 UI 上的 QChartView
    ui->chartWidget->setChart(chart);
}

void MainWindow::initTimer()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimeout);
    // default timeout is 3 seconds
    timer->start(3000);
}

void MainWindow::onTimeout()
{
    double price = 100 + (rand() % 20); // 模拟价格
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    insertData(timestamp, price);
    // 根据精度重新查询
    queryAndUpdateChart();
}

void MainWindow::onIntervalChanged()
{
    QString text = ui->accuracyCombo->currentText();
    if (text == "3s") currentInterval = 3;
    else if (text == "1m") currentInterval = 60;
    else if (text == "15m") currentInterval = 900;
    else if (text == "30m") currentInterval = 1800;
    else if (text == "1h") currentInterval = 3600;
    else if (text == "1d") currentInterval = 86400;

    updateAxisFormat();
    queryAndUpdateChart();
}
