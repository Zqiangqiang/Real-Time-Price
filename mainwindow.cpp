#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>

// 金衡盎司
static double Ounce = 31.1035;
// 港两
static double Tael = 37.429;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initChart();
    initDB();

    ui->historyRecord->setDateTime(QDateTime::currentDateTime());

    loadHistoryData(); // 加载历史
    if (!series->points().isEmpty()) {
        // 初始化x轴范围
        qint64 first = series->points().first().x() / 1000;
        qint64 last  = series->points().last().x() / 1000;

        axisX->setRange(
            QDateTime::fromSecsSinceEpoch(first),
            QDateTime::fromSecsSinceEpoch(last)
        );

        // 初始化y轴范围
        double minY = series->points().first().y();
        double maxY = minY;

        for (const QPointF &p : series->points()) {
            minY = qMin(minY, p.y());
            maxY = qMax(maxY, p.y());
        }

        // 加一点边距（非常关键）
        double padding = (maxY - minY) * 0.1;

        // 防止完全不波动
        if (padding < 0.5) padding = 0.5;

        axisY->setRange(minY - padding, maxY + padding);
    }

    initTimer();
    initNetwork();
    // 获取当日汇率
    requestExchangeRate();
    connect(ui->accuracyCombo, &QComboBox::currentTextChanged, this, &MainWindow::onIntervalChanged);

    status = new QLabel(this);
    status->setPixmap(QPixmap(":/finished.png").scaled(20, 20));
    ui->statusbar->addWidget(new QLabel("Current status: "));
    ui->statusbar->addWidget(status);
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

void MainWindow::initNetwork()
{
    manager = new QNetworkAccessManager(this);
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

void MainWindow::queryAndUpdateChart(qint64 startTime)
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

    // 扫描整个数据表，数据量达到100W及后将存在巨大的性能开销
    // QString sql = QString(R"(
    //     SELECT
    //         (timestamp / %1) * %1 AS t,
    //         AVG(price)
    //     FROM price_data
    //     GROUP BY t
    //     ORDER BY t
    // )").arg(currentInterval);

    // 只查询对应精度下允许的范围
    updateDisplayWindow();

    QString sql = QString(R"(
        SELECT
            (timestamp / %1) * %1 AS t,
            AVG(price)
        FROM price_data
        WHERE timestamp BETWEEN %2 AND %3
        GROUP BY t
        ORDER BY t
    )").arg(currentInterval).arg(startTime - displayWindow).arg(startTime);

    QSqlQuery query(sql);
    while (query.next()) {
        qint64 t = query.value(0).toLongLong();
        double price = query.value(1).toDouble();
        // 区间内点平均值
        series->append(t * 1000, price);
    }

    // 更新坐标轴范围
    scrollAxisXYRange(startTime, series);
}

void MainWindow::requestPrice()
{
    QUrl url("https://forex-data-feed.swissquote.com/public-quotes/bboquotes/instrument/XAU/USD");
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Request failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        parseResponse(data);
        reply->deleteLater();
    });
}

void MainWindow::parseResponse(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isArray()) return;

    QJsonArray arr = doc.array();

    if (arr.isEmpty()) return;

    QJsonObject firstObj = arr.at(0).toObject();
    QJsonArray prices = firstObj["spreadProfilePrices"].toArray();

    if (prices.isEmpty()) return;

    QJsonObject p = prices.at(0).toObject();

    // bid表示市场愿意买的价格（你卖出时成交）
    // ask表示市场愿意卖的价格（你买入时成交）
    double bid = p["bid"].toDouble();
    double ask = p["ask"].toDouble();

    // 使用中间价
    usdPrice = (bid + ask) / 2.0;
    QString type = ui->rateCombox->currentText().trimmed();
    qDebug() << type;
    double price = USDToOther(usdPrice, type);
    qDebug() << "bid:" << bid << "ask:" << ask << "mid:" << price;

    ui->realtimePriceLabel->setText("Price: " +  QString::number(price));
    // 使用查询时间更符合实际情况
    qint64 timestamp = firstObj["ts"].toVariant().toLongLong() / 1000;
    //qint64 timestamp = QDateTime::currentSecsSinceEpoch();

    insertData(timestamp, usdPrice);
    queryAndUpdateChart();
}

void MainWindow::requestExchangeRate()
{
    QUrl url("https://v6.exchangerate-api.com/v6/d0cf0058fd36cc793904de72/latest/USD");

    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Request failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        // 解析并生成ExchangeRate
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return;
        QJsonObject obj = doc.object();
        if (!obj["conversion_rates"].isObject()) return;
        QJsonObject subObj = obj["conversion_rates"].toObject();

        rate = new ExchangeRate();
        rate->toCNY = subObj["CNY"].toDouble();
        rate->toHKD = subObj["HKD"].toDouble();
        qDebug() << "CNY " << rate->toCNY << " HKD " << rate->toHKD;

        reply->deleteLater();
    });
}

void MainWindow::updateDisplayWindow()
{
    if (currentInterval <= 3) {
        displayWindow = 300;        // 只显示最近5分钟
    }
    else if (currentInterval <= 60) {
        displayWindow = 1800;       // 30分钟
    }
    else if (currentInterval <= 900) {
        displayWindow = 3 * 3600;   // 3小时
    }
    else if (currentInterval <= 3600) {
        displayWindow = 1 * 86400;  // 1天
    }
    else {
        displayWindow = 14 * 86400; // 14天
    }
}

void MainWindow::scrollAxisXYRange(qint64 nowTime, const QLineSeries* series)
{
    //滑动窗口 来更新x轴
    axisX->setRange(
        QDateTime::fromSecsSinceEpoch(nowTime - displayWindow),
        QDateTime::fromSecsSinceEpoch(nowTime)
    );

    // 更新y轴范围
    if (!series->points().isEmpty()) {

        double minY = series->points().first().y();
        double maxY = minY;

        for (const QPointF &p : series->points()) {
            minY = qMin(minY, p.y());
            maxY = qMax(maxY, p.y());
        }

        // 加一点边距（非常关键）
        double padding = (maxY - minY) * 0.1;

        // 防止完全不波动
        if (padding < 0.5) padding = 0.5;

        axisY->setRange(minY - padding, maxY + padding);
    }
}

double MainWindow::USDToOther(const double usdPrice, const QString &type)
{
    if (type == "USD") {
        return usdPrice;
    }else if (type == "CNY") {
        // 转换成人民币每克
        return usdPrice * rate->toCNY / Ounce;
    } else if (type == "HKD") {
        // 转换成港币每两
        return usdPrice / Ounce * Tael * rate->toHKD;
    } else {
        return -1;
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
    //timer->start(currentInterval * 1000);
}

void MainWindow::onTimeout()
{
    // double price = 100 + (rand() % 20); // 模拟价格
    // qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    // insertData(timestamp, price);
    // // 根据精度重新查询
    // queryAndUpdateChart();
    requestPrice();

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

void MainWindow::on_startEndBtn_clicked()
{
    if (!isWorking) {
        timer->start(currentInterval * 1000);
        ui->historyRecord->setDisabled(true);
        ui->startEndBtn->setText("end record");
        status->setPixmap(QPixmap(":/working.png").scaled(20, 20));
    } else {
        timer->stop();
        ui->historyRecord->setDisabled(false);
        ui->startEndBtn->setText("start record");
        status->setPixmap(QPixmap(":/finished.png").scaled(20, 20));
    }
    isWorking = !isWorking;
}

void MainWindow::on_historyRecord_dateTimeChanged(const QDateTime &dateTime)
{
    qint64 historyTime = dateTime.toSecsSinceEpoch();
    queryAndUpdateChart(historyTime);
}

void MainWindow::on_rateCombox_currentTextChanged(const QString &arg1)
{
    double price = USDToOther(usdPrice, arg1.trimmed());

    ui->realtimePriceLabel->setText("Price: " +  QString::number(price));
    ui->rateCombox->setCurrentText(arg1);
}

