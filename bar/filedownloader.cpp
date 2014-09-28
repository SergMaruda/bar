#include "filedownloader.h"
#include <QtNetwork\qnetworkrequest.h>
#include <QtNetwork\qnetworkreply.h>
#include <QtCore\qeventloop.h>

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
QObject(parent)
  {
  connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), SLOT(fileDownloaded(QNetworkReply*)));

  QNetworkRequest request(imageUrl);
  QNetworkReply* reply = m_WebCtrl.get(request);

  QEventLoop loop;
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();
  }

FileDownloader::~FileDownloader()
  {

  }

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
  {
  m_DownloadedData = pReply->readAll();
  //emit a signal
  pReply->deleteLater();
  emit downloaded();
  }

QByteArray FileDownloader::downloadedData() const
  {
  return m_DownloadedData;
  }