#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);

signals:
    void priceReady(double price, qint64 timestamp);

public slots:
    void requestPrice();  // 在子线程执行

private:
    QNetworkAccessManager *manager;

};

#endif // WORKER_H
