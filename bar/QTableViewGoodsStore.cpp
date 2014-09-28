#include "QTableViewGoodsStore.h"
#include <QtGui\qevent.h>
#include <QtCore\qmimedata.h>
#include <QtCore\qlogging.h>
#include <QtCore\qdebug.h>
#include <QtWidgets\qmessagebox.h>
#include <QtCore\qbuffer.h>
#include "..\..\Src\qtbase\src\sql\kernel\qsqlquery.h"
#include "..\..\Src\qtbase\src\sql\models\qsqltablemodel.h"
#include "QBarApplication.h"
#include "UserRoles.h"
#include <QtCore\qtemporaryfile.h>
#include "filedownloader.h"

//--------------------------------------------------------------------------------------------------------
QTableViewGoodsStore::QTableViewGoodsStore(QWidget *parent)
  : QTableView (parent)
  {
  bool is_owner = QBarApplication::instance()->currentUserRole() > UR_BARTENDER;
  if(is_owner)
    setDragDropMode(QAbstractItemView::DropOnly);
  }

//--------------------------------------------------------------------------------------------------------
QTableViewGoodsStore::~QTableViewGoodsStore()
  {
  }

//--------------------------------------------------------------------------------------------------------
void QTableViewGoodsStore::dragEnterEvent(QDragEnterEvent *e)
  {
  e->acceptProposedAction();
  }

//--------------------------------------------------------------------------------------------------------
void QTableViewGoodsStore::dropEvent(QDropEvent *e)
  {
  bool has_img = e->mimeData()->hasImage();
  QPixmap icon;

  QPoint pos = mapFromGlobal(QCursor::pos());
  QModelIndex idx = indexAt(pos);

  if(icon.size().isEmpty() && e->mimeData()->hasImage())
    {
    QImage image = qvariant_cast<QImage>(e->mimeData()->imageData());
    icon = QPixmap::fromImage(image);
    }

  if(icon.size().isEmpty() && e->mimeData()->hasText())
    {
    QString str = e->mimeData()->text();
    str.replace("file:///","");
    QPixmap px_tmp;
    if(px_tmp.load(str))
      icon = px_tmp; 
    }

  if(icon.size().isEmpty() && e->mimeData()->hasUrls())
    {
    auto urls = e->mimeData()->urls();
    QUrl url  = *urls.begin();
    auto str = url.toString();
    FileDownloader down(url);

    QPixmap px_tmp;
    if(px_tmp.loadFromData(down.downloadedData()))
      icon = px_tmp; 
    }

  if(!icon.size().isEmpty())
    {
    QByteArray bytes;

    QPixmap new_px = icon.scaledToHeight(64);
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    bool res = new_px.save(&buffer, "PNG");

    if(res)
      {
      QModelIndex idx_new = model()->index(idx.row(),0);
      int goog_id = idx_new.data().toInt();

      QString quesry_Str = QString("INSERT INTO goods_icons values(%1, null)").arg(goog_id);
      QSqlQuery qqq(quesry_Str);

      quesry_Str = QString("UPDATE goods_icons SET icon = (?) WHERE id = %1").arg(goog_id);
      QSqlQuery query;
      query.prepare(quesry_Str);
      query.addBindValue(bytes);
      bool res = query.exec();
      QBarApplication::instance()->removeGoodIcon(goog_id);
      static_cast<QSqlTableModel*>(model())->select();
      }
    }
  }

//--------------------------------------------------------------------------------------------------------
void QTableViewGoodsStore::dragMoveEvent( QDragMoveEvent* )
  {
  }

