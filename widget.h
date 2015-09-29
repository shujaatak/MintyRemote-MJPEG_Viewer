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

protected:
    void resizeEvent(QResizeEvent* ev) Q_DECL_OVERRIDE;

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void startRequest(QUrl url);

private slots:
    void httpFinished();
    void httpReadyRead();
    void assembleJPEG(QByteArray);
    void displayJPEG();

    void on_stopButton_clicked();

private:
    void resizeImage();
    void parseImageData(QByteArray);
    Ui::Widget *ui;
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    bool httpRequestAborted;
    quint32 jpegSize;
    QByteArray jpegBA;
    bool imageReady;
    quint8 status;
};

#endif // WIDGET_H
