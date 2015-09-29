#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    url = QUrl::fromEncoded("http://192.168.0.101:8081");
    startRequest(url);
    jpegBA.clear();
    imageReady=false;
    status=0;
    ui->imgDebug->hide();
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

    QByteArray response = reply->readAll();
//    disconnect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));

    switch(status)
    {
    case 0: // No START FRAME has been received
        parseImageData(response);

        break;
    case 1: // Frame is already being built
        assembleJPEG(response);
        break;
    default:
        break;
    }

    //        ui->imgDebug->appendPlainText("New packet:");
    //        ui->imgDebug->appendPlainText(response);
}

void Widget::assembleJPEG(QByteArray packet)
{
    // Is new packet going to go over the limit of the JPEG frame size?
//    qDebug() << "Current size: " << jpegBA.size() << ", "
//             << "JPEG frame: " << jpegSize << ", "
//             << "incoming packet: " << packet.size() << ", bytes needed: " << jpegSize-jpegBA.size();

    //    jpegBA.append(packet);

    // Only append as many bytes as needed to complete the frame.
    quint32 bytesNeeded = jpegSize-jpegBA.size();
    jpegBA.append(packet.left(bytesNeeded));
//    qDebug() << "Current size: " << jpegBA.size();
    if(jpegBA.size()>=jpegSize)
    {
        jpegBA.chop(2);
        displayJPEG();
        jpegBA.clear();
        status=0;

        packet.remove(0, bytesNeeded+2);
        if(packet.size()>0)
        {
//            qDebug() << packet.size() << " bytes left in packet";
            parseImageData(packet);
            //            ui->imgDebug->appendPlainText("Bytes left:");
            //            ui->imgDebug->appendPlainText(packet);
        }

    }
//    connect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));

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

void Widget::parseImageData(QByteArray data)
{
    if(data.at(0)=='-' && data.at(1)=='-')
    {
        //        jpegBA.clear();
        // Collect JPEG frame size
        data.remove(0,61);
        QByteArray chunk = data;

        data = data.trimmed();
        data.remove(5,8);
        jpegSize = data.toLong();
        //        ui->imgDebug->clear();
        //        ui->imgDebug->appendPlainText(QString("%1 bytes expected").arg(jpegSize));

        chunk.remove(0,14);
        assembleJPEG(chunk); // send to jpeg assembler
        status=1;
    }

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
