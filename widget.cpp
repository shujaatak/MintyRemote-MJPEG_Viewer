#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    url = QUrl::fromEncoded("http://24.52.217.108:8090");
    startRequest(url);
    jpegBA.clear();
    connect(ui->stopButton,SIGNAL(clicked(bool)),this,SLOT(stopTransfer()));
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
    //    delete file;
    //    file = 0;
}

void Widget::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    //    if (file)
    //        file->write(reply->readAll());
    qDebug() << reply->bytesAvailable() << " bytes available";
    QByteArray response = reply->readAll();
    if(response.at(0)=='-' && response.at(1)=='-')
    {
        // Collect JPEG Size
        QString basize = response;
        basize.remove(0,59);
        QByteArray chunk = basize.toLocal8Bit();
        basize = basize.trimmed();
        basize.remove(5,8);
        jpegSize = basize.toLong();
        qDebug() << jpegSize << " bytes expected";
        chunk.remove(0,16); // send to jpeg assembler
        assembleJPEG(chunk);
    }
    else
    {
        assembleJPEG(response);
    }
}

void Widget::stopTransfer()
{
    disconnect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));
    //    qnam.deleteLater();
    //    qnam.blockSignals(true);
}

void Widget::assembleJPEG(QByteArray packet)
{
    jpegBA.append(packet);
    qDebug() << jpegBA.size() << " bytes received";
}

