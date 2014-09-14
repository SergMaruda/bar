#include "TransactionsViewModel.h"
#include "TransactionStatus.h"
#include <QtSql\qsqlrecord.h>
#include <QtGui\qcolor.h>
#include <QtGui\qbrush.h>
#include "..\..\Src\qtbase\src\sql\kernel\qsqlindex.h"
#include "..\..\Src\qtbase\src\sql\kernel\qsqlfield.h"
#include <QtGui\qicon.h>
#include "QBarApplication.h"

//---------------------------------------------------------------------------------------------------------------------------------------
Qt::ItemFlags TransactionsViewModel::flags(const QModelIndex &index) const
  {
  Qt::ItemFlags f = QSqlTableModel::flags(index);
  int row = index.row();

  if(record(row).value(0).toInt() == TransactionStatus::WaitingForPurchasePayment && (index.column() == TransactionsViewColumn::Number || index.column() == TransactionsViewColumn::Price || index.column() == TransactionsViewColumn::Sum))
    return Qt::ItemFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

  if(record(row).value(0).toInt() == TransactionStatus::WaitingForPayment && (index.column() == TransactionsViewColumn::Number))
    return Qt::ItemFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

  return  Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }

//---------------------------------------------------------------------------------------------------------------------------------------
QVariant TransactionsViewModel::data(const QModelIndex &idx, int role /*=Qt::DisplayRole*/) const
  {
  if (role == Qt::CheckStateRole)
    return QVariant();

  int tr_status(-1);

  int row = idx.row();
  if (role == Qt::BackgroundRole || role == Qt::DecorationRole)
    if(row < rowCount())
      {
      QSqlRecord rec = record(row);
      tr_status = rec.value(0).toInt();
      }

  if (role == Qt::BackgroundRole) 
    {
    if(tr_status == TransactionStatus::WaitingForPayment)
      {
      QColor color(255,0,0, 128);           
      return QBrush(color);
      } 
    else if(tr_status == TransactionStatus::Payed)
      {
      QColor color(0,255,0, 128);           
      return QBrush(color);
      }
    else if(tr_status == TransactionStatus::WaitingForPurchasePayment)
      {
      QColor color(255,0,0, 64);           
      return QBrush(color);
      }
    else if(tr_status == TransactionStatus::PurchasePayed)
      {
      QColor color(0,255,0, 64);            
      return QBrush(color);
      }
    else if(tr_status == TransactionStatus::PrepareGoodwithdrawal)
      {
      QColor color(255,255,0, 128);            
      return QBrush(color);
      }
    else if(tr_status == TransactionStatus::Goodwithdrawal)
      {
      QColor color(255,255,0, 64);            
      return QBrush(color);
      }
    else if(tr_status == TransactionStatus::SaleCanceled)
      {
      QColor color(0,0,255, 64);            
      return QBrush(color);
      }
    }

  if(idx.isValid() && role ==  Qt::DecorationRole)
    {
    int col = idx.column();
    if(col == 1)
      {
      int good_id = record(idx.row()).value(8).toInt();
      return QBarApplication::instance()->goodIcon(good_id);
      }
    }

  if (role == Qt::DecorationRole) 
    {
    int row = idx.row();
    QSqlRecord rec = record(row);
    int tr_status = rec.value(0).toInt();
    QColor color(0,0,255, 64);


    if(tr_status == TransactionStatus::WaitingForPayment && idx.column() == 2)
      {
      return QIcon(":/bar/Resources/edit_icon.png");
      }

    if(tr_status == TransactionStatus::WaitingForPurchasePayment && (idx.column() == 2 || idx.column() == 3 || idx.column() == 4))
      {
      return QIcon(":/bar/Resources/edit_icon.png");
      }
    }
  return QSqlTableModel::data(idx, role);
  }

bool TransactionsViewModel::selectRow( int row )
  { 
  QSqlIndex sql_idx;
  QSqlRecord rec = record();
  sql_idx.append(rec.field(0));
  setPrimaryKey(sql_idx);
  return __super::selectRow(row);
  }
