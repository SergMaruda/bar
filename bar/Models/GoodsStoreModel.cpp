#include "GoodsStoreModel.h"
#include <QtSql\qsqlquery.h>
#include <QtSql\qsqlrecord.h>
#include "QBarApplication.h"
#include <QtGui\qicon.h>

//--------------------------------------------------------------------------------------------------------------
QGoodsStoreModels::QGoodsStoreModels( QObject *parent /*= 0*/, QSqlDatabase db /*= QSqlDatabase()*/ ):
  QSqlTableModel(parent, db)
  {
  setTable("GOODS_STORE");
  }

//--------------------------------------------------------------------------------------------------------------
QGoodsStoreModels::~QGoodsStoreModels()
  {

  }

//--------------------------------------------------------------------------------------------------------------
int QGoodsStoreModels::GoodNumber( int good_id ) const
  {
  QString query_str = QString("SELECT* FROM GOODS_STORE WHERE ID = %1").arg(good_id);
  QSqlQuery sql_qur(database());
  bool qres = sql_qur.exec(query_str);
  sql_qur.next();

  QSqlRecord rec = sql_qur.record();
  int number = rec.value(3).toInt();
  return number;
  }

//--------------------------------------------------------------------------------------------------------------
double QGoodsStoreModels::GoodPrice( int good_id ) const
  {
  QString query_str = QString("SELECT [Цена] FROM GOODS_STORE WHERE ID = %1").arg(good_id);
  QSqlQuery sql_qur(database());
  bool qres = sql_qur.exec(query_str);
  sql_qur.next();
  double price = sql_qur.record().value(0).toDouble();
  return price;
  }

//--------------------------------------------------------------------------------------------------------------
QString QGoodsStoreModels::GoodName( int good_id ) const
  {
  QString query_str = QString("SELECT [Наименование] FROM GOODS_STORE WHERE ID = %1").arg(good_id);
  QSqlQuery sql_qur(database());
  bool qres = sql_qur.exec(query_str);
  sql_qur.next();
  return sql_qur.record().value(0).toString();
  }

//---------------------------------------------------------------------------------------------------------------
QVariant QGoodsStoreModels::data( const QModelIndex &idx, int role /*= Qt::DisplayRole*/ ) const
  {
  if(idx.isValid() && role ==  Qt::DecorationRole)
    {
    int col = idx.column();
    if(col == 1)
      {
      int good_id = record(idx.row()).value(0).toInt();
      return QBarApplication::instance()->goodIcon(good_id);
      }
    }

  return QSqlTableModel::data(idx, role);
  }


//---------------------------------------------------------------------------------------------------------------------------------------
Qt::ItemFlags QGoodsStoreModels::flags(const QModelIndex &index) const
  {
  if(index.isValid())
    {
    int col = index.column();

    auto app = QBarApplication::instance();

    int user_role = app->currentUserRole();

    if(user_role == 0)
      {
      if(col == 1 || col == 2)
        return Qt::ItemFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
      }
    }
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;;
  }