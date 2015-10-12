#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->stopButton->hide();
    ui->imgDebug->hide();
    mjpegLine1.append("--BoundaryString\r\n"
                      "Content-type: image/jpeg\r\n"
                      "Content-Length:\\s+");
    mjpegLine2.append("\\d+");
    mjpegLine3.append("(\r\n)+");

    jpegBA.clear();
    frameStarted = false;
    response.clear();

    url = QUrl::fromEncoded("http://192.168.0.146:8081");
    //    url = QUrl::fromEncoded("http://24.52.217.108:8090");
    startRequest(url);

    //    imageReady=false;
    //    status=0;
    //    ui->imgDebug->hide();
    //    ui->imgOutput->hide();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::startRequest(QUrl url)
{
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),
            this, SLOT(httpFinished()));
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));
    //    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
    //            this, SLOT(updateDataReadProgress(qint64,qint64)));
}

void Widget::httpFinished()
{
    if (httpRequestAborted) {
        //        if (file) {
        //            file->close();
        //            file->remove();
        //            delete file;
        //            file = 0;
        //        }
        reply->deleteLater();
        //        progressDialog->hide();
        return;
    }

    //    progressDialog->hide();
    //    file->flush();
    //    file->close();

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        //        file->remove();
        QMessageBox::information(this, tr("HTTP"),
                                 tr("Download failed: %1.")
                                 .arg(reply->errorString()));
        //        downloadButton->setEnabled(true);
    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(this, tr("HTTP"),
                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            url = newUrl;
            reply->deleteLater();
            //            file->open(QIODevice::WriteOnly);
            //            file->resize(0);
            startRequest(url);
            return;
        }
    } else {
        //        QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
        //        statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));
        //        downloadButton->setEnabled(true);
    }
    reply->deleteLater();
    reply = 0;
}

void Widget::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply

    response = reply->readAll();
//disconnect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));

    while(!response.isEmpty())
    {
//        qDebug() << "response has " << response.size() << " bytes, " << frameStarted;
        if(!frameStarted)
        {
            if(attemptStart())
            {
                frameStarted = true;
                jpegBA.clear();
                payloadSize=0;
            }
            else
            {
                response.clear();
            }
        }
        else
        {
            // Assign bytes to existing payload
            assembleJPEG();
        }
    }
//connect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));
}

bool Widget::attemptStart()
{
    QRegExp rx = QRegExp(mjpegLine1);
    int pos = rx.indexIn(response);
    if(pos != -1)
    {
        ui->imgDebug->clear();
        ui->imgDebug->appendPlainText(QString("%1 bytes received:").arg(response.size()));

        ui->imgDebug->appendPlainText("MJpeg header found");
        ui->imgDebug->appendPlainText(QString("%1 bytes matched").arg(rx.matchedLength()));
        response.remove(0,pos+rx.matchedLength());

        rx.setPattern(mjpegLine2);
        rx.indexIn(response);
        jpegSize = rx.cap(0).toLong();
        ui->imgDebug->appendPlainText(QString("%1 bytes expected").arg(jpegSize));
        response.remove(0,rx.matchedLength());

        rx.setPattern(mjpegLine3);
        rx.indexIn(response);
        ui->imgDebug->appendPlainText(QString("%1 ws chars removed").arg(rx.matchedLength()));
        response.remove(0,rx.matchedLength());

        ui->imgDebug->appendPlainText(QString("%1 payload bytes left").arg(response.size()));
        //            ui->imgDebug->appendPlainText(response);
        return true;
    }
    else
    {
        ui->imgDebug->appendPlainText("No frame start");
        return false;
    }
}

void Widget::assembleJPEG()
{
    quint16 bytesNeeded = jpegSize-payloadSize;
    ui->imgDebug->appendPlainText(QString("%1 bytes needed").arg(bytesNeeded));

    // Two options:
    // 1. The response array has enough bytes to complete the frame
        // 2. The response array will not complete the frame
    if(response.size()>=bytesNeeded)
    {
        ui->imgDebug->appendPlainText("Complete JPEG");
        jpegBA.append(response.left(bytesNeeded));
        payloadSize+=bytesNeeded;
        response.remove(0,bytesNeeded);
//        frameStarted=false;
//        response.clear();
    }

    else
    {
        ui->imgDebug->appendPlainText("Use up response");
        jpegBA.append(response);
        payloadSize+=response.size();
        response.clear();
    }

    if(payloadSize>=jpegSize)
    {
        jpegBA.chop(2);
        displayJPEG();
        jpegBA.clear();
        frameStarted=false;
        response.clear();
    }
}

void Widget::displayJPEG()
{
    disconnect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));

    QPixmap img;
    //    qDebug() << "jpegBA size: " << jpegBA.size();
    img.loadFromData(jpegBA);
    if(!img.isNull())
    {
        int w = ui->imgOutput->width();
        int h = ui->imgOutput->height();

        ui->imgOutput->setPixmap(img.scaled(w,h,Qt::KeepAspectRatio));
    }
    //        ui->imgOutput->setPixmap(img);
    imageReady=true;
    //    resizeImage();
    connect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));
}

void Widget::on_stopButton_clicked()
{
    disconnect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));
}

void Widget::resizeEvent(QResizeEvent *ev)
{
    if(imageReady)
    {
        resizeImage();
    }
    QWidget::resizeEvent(ev);
}

void Widget::resizeImage()
{
    if(imageReady)
    {
        const QPixmap *p = ui->imgOutput->pixmap();
        int w = ui->imgOutput->width();
        int h = ui->imgOutput->height();
        ui->imgOutput->setPixmap(p->scaled(w,h,Qt::KeepAspectRatio));
    }
}
