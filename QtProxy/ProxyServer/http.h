#pragma once
#include <QObject>
#include <QTcpSocket>
#include<QTextEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class HTTP; }
QT_END_NAMESPACE
class HTTP : public QObject
{
    Q_OBJECT
public:
    explicit HTTP(QObject *parent = nullptr);
    ~HTTP();

public slots:
    void interceptRequest(QTcpSocket *clientSocket, const QByteArray &request);

private:
   // QTextEdit*textEdit=new QTextEdit(this);
    // Adaugă metode sau membri pentru a implementa funcționalitățile specifice
};
