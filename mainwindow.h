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
    void updateUI(const QString &requestData);

    void on_btnStartStop_clicked();

public:
    MainWindow(QWidget *parent = nullptr);
    void handleClientRequest(QTcpSocket *clientSocket);
    char *extractTargetURL(const char *request);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpServer* server;
    QLabel*labelPort=new QLabel();
    QTextEdit*textEdit=new QTextEdit();

    bool isListening;

};
#endif
