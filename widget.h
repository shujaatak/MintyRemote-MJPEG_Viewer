#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QMessageBox>
#include <QNetworkSession>
#include <QRegExp>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

protected:
    void resizeEvent(QResizeEvent* ev) Q_DECL_OVERRIDE;

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void startRequest(QUrl url);

private slots:
    void httpFinished();
    void httpReadyRead();

    void displayJPEG();

    void on_stopButton_clicked();

private:
    void resizeImage();
    bool attemptStart();
    void assembleJPEG();

    Ui::Widget *ui;
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    bool httpRequestAborted;
    quint16 jpegSize;
    quint16 payloadSize;
    QByteArray jpegBA;
    bool imageReady;
    bool frameStarted;
    quint8 status;
    QString mjpegLine1;
    QString mjpegLine2;
    QString mjpegLine3;
    QByteArray response;
};

#endif // WIDGET_H
