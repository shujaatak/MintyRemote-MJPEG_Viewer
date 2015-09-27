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
    //    ui->imgDebug->appendPlainText("New packet:");
    //    ui->imgDebug->appendPlainText(response);
    if(response.at(0)=='-' && response.at(1)=='-')
    {
        jpegBA.clear();

        // Collect JPEG frame size
        response.remove(0,61);
        QByteArray chunk = response;

        response = response.trimmed();
        response.remove(5,8);
        jpegSize = response.toLong();
        //        ui->imgDebug->clear();
        //        ui->imgDebug->appendPlainText(QString("%1 bytes expected").arg(jpegSize));

        chunk.remove(0,14);
        assembleJPEG(chunk); // send to jpeg assembler
    }
    else
    {
        assembleJPEG(response);
    }
}

void Widget::assembleJPEG(QByteArray packet)
{
    jpegBA.append(packet);
    if(jpegBA.size()>=jpegSize)
    {
        jpegBA.chop(2);
        displayJPEG();
        //        jpegBA.clear();
    }
}

void Widget::displayJPEG()
{
    QPixmap img;
    img.loadFromData(jpegBA);
    ui->imgOutput->setPixmap(img);
    imageReady=true;

    const QPixmap *p = ui->imgOutput->pixmap();
    int w = ui->imgOutput->width();
    int h = ui->imgOutput->height();
    qDebug() << "width: " << w;
    ui->imgOutput->setPixmap(p->scaled(w,h,Qt::KeepAspectRatio));

}

//void Widget::on_stopButton_clicked()
//{
//    disconnect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));
//}

void Widget::resizeEvent(QResizeEvent *ev)
{
    if(imageReady)
    {
        const QPixmap *p = ui->imgOutput->pixmap();
        int w = ui->imgOutput->width();
        int h = ui->imgOutput->height();
        qDebug() << "width: " << w;
        ui->imgOutput->setPixmap(p->scaled(w,h,Qt::KeepAspectRatio));
    }
    QWidget::resizeEvent(ev);
}
