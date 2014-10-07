#include "GoodsCheckModel.h"

#include <QtSql\qsqlrecord.h>
#include <QtGui\qcolor.h>
#include <QtGui\qbrush.h>
#include <QtSql\qsqlquery.h>
#include "..\..\Src\qtbase\src\sql\kernel\qsqlfield.h"
#include "..\..\Src\qtbase\src\sql\kernel\qsqlindex.h"
#include "QBarApplication.h"

//---------------------------------------------------------------------------------------------------------------------------------------
GoodsCheckModel::GoodsCheckModel(QObject *parent, QSqlDatabase db):
QSqlTableModel(parent, db)
  {
  QObject::connect(this, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(_dataChanged(const QModelIndex&, const QModelIndex&)));
  }


//---------------------------------------------------------------------------------------------------------------------------------------
Qt::ItemFlags GoodsCheckModel::flags(const QModelIndex &index) const
  {
  Qt::ItemFlags f = QSqlTableModel::flags(index);
  int row = index.row();
  int col = index.column();
  
  if(col != 2)
    return (f & ~Qt::ItemIsEditable);

  if(col == 2)
    return (f | Qt::ItemIsEditable);
  return  f;
  }

//---------------------------------------------------------------------------------------------------------------------------------------
QVariant GoodsCheckModel::data(const QModelIndex &idx, int role /*=Qt::DisplayRole*/) const
  {
  if (role == Qt::CheckStateRole)
    return QVariant();

  int row = idx.row();
  if (role == Qt::BackgroundRole) 
    {
    QSqlRecord rec = record(row);
    int number = rec.value(3).toInt();
    if(number < 0)
      {
      QColor color(255,0,0, 64);           
      return QBrush(color);
      }

    if(number > 0)
      {
      QColor color(0,255,0, 64);           
      return QBrush(color);
      }
    }

  if(idx.isValid() && role ==  Qt::DecorationRole)
    {
    int col = idx.column();
    if(col == 1)
      {
      int id =  record(row).value("ID").toInt();
      return QBarApplication::instance()->goodIcon(id);
      }
    }

  return QSqlTableModel::data(idx, role);
  }

//---------------------------------------------------------------------------------------------------------------------------------------
void GoodsCheckModel::_dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight)
  {
  if(topLeft.isValid() && topLeft == bottomRight)
    {
    int row = topLeft.row();
    int number = topLeft.data().toInt();

    QSqlIndex idx;
    idx.append(record().field(0));
    setPrimaryKey(idx);
    
    int id =  record(row).value("ID").toInt();
    QString quary_str(QString("insert or replace into GOODS_CHECK values(%1,%2)").arg(id).arg(number));
    bool res = QSqlQuery().exec(quary_str);
    Q_ASSERT(res);

    selectRow(row);
    }
  return;
  }
