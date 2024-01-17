#ifndef TRAFFICDATA_H
#define TRAFFICDATA_H
#include <stdio.h>
#include <stdlib.h>
#include <QString>

class TrafficData {
public:
    TrafficData();

    void addReceivedData(const QString &data);
    void addSentData(const QString &data);
    void addIp(const QString &ip);
    void addDate(const QString &date);

    QString getReceivedData() const;
    QString getSentData() const;
    QString getDate() const;
    QString getIp() const;

private:
    QString receivedData;
    QString sentData;
    QString ip;
    QString date;

};




TrafficData::TrafficData() {
    this->receivedData = "";
    this->sentData = "";
    this->ip="";
    this->date="";
}

void TrafficData::addReceivedData(const QString &data) {
    this->receivedData.append(data);
}

void TrafficData::addSentData(const QString &data) {
    this->sentData.append(data);
}

void TrafficData::addIp(const QString &ip)
{
    this->ip.append(ip);
}

void TrafficData::addDate(const QString &date)
{
    this->date.append(date);
}

QString TrafficData::getDate() const
{
    return this->date;
}

QString TrafficData::getIp() const
{
    return this->ip;
}

QString TrafficData::getReceivedData() const {
    return this->receivedData;
}

QString TrafficData::getSentData() const {
    return this->sentData;
}

#endif // TRAFFICDATA_H
