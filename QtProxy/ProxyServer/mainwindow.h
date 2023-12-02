#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLineEdit>
#include<QLabel>
#include<QVBoxLayout>
#include<QPlainTextEdit>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<QCoreApplication>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private slots:
    void onNewConnection();
    void onReadyRead();
    void on_btnRequests_clicked();
    void updateUI(const QString &requestData);

public:
    MainWindow(QWidget *parent = nullptr);
    void handleClientRequest(QTcpSocket *clientSocket);
    char *extractTargetURL(const char *request);
    void HTTPtraffic(QByteArray&response);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpServer* server;
    QLabel*labelPort=new QLabel();
    QLabel*target=new QLabel();
    QLabel*labelResponse=new QLabel(this);
    QTextEdit*textEdit=new QTextEdit();

};
#endif // MAINWINDOW_H
