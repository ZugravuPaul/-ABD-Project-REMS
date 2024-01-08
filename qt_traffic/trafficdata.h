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

    QString getReceivedData() const;
    QString getSentData() const;

private:
    QString receivedData;
    QString sentData;
};




TrafficData::TrafficData() {
    this->receivedData = "";
    this->sentData = "";
}

void TrafficData::addReceivedData(const QString &data) {
    this->receivedData.append(data);
}

void TrafficData::addSentData(const QString &data) {
    this->sentData.append(data);
}

QString TrafficData::getReceivedData() const {
    return this->receivedData;
}

QString TrafficData::getSentData() const {
    return this->sentData;
}

#endif // TRAFFICDATA_H
