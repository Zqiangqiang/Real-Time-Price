#include "worker.h"

Worker::Worker(QObject *parent)
    : QObject{parent}
{
    manager = new QNetworkAccessManager(this);
}

void Worker::requestPrice()
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

        // JSON解析在子线程！
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isArray()) return;

        QJsonObject obj = doc.array().at(0).toObject();
        QJsonArray prices = obj["spreadProfilePrices"].toArray();

        if (prices.isEmpty()) return;

        QJsonObject p = prices.at(0).toObject();

        double bid = p["bid"].toDouble();
        double ask = p["ask"].toDouble();

        double mid = (bid + ask) / 2.0;

        qint64 timestamp = obj["ts"].toVariant().toLongLong() / 1000;

        // 发回主线程(将解析出来的价格和时间戳发送回主线程)
        emit priceReady(mid, timestamp);
        reply->deleteLater();
    });
}


