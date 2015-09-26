#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QMessageBox>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void startRequest(QUrl url);

private slots:
    void httpFinished();
    void httpReadyRead();
    void stopTransfer();
    void assembleJPEG(QByteArray);

private:
    Ui::Widget *ui;
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    bool httpRequestAborted;
    quint16 jpegSize;
    QByteArray jpegBA;
};

#endif // WIDGET_H
