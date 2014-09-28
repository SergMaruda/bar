#include "TransactionsViewModel.h"
#include "TransactionStatus.h"
#include <QtSql\qsqlrecord.h>
#include <QtGui\qcolor.h>
#include <QtGui\qbrush.h>
#include "..\..\Src\qtbase\src\sql\kernel\qsqlindex.h"
#include "..\..\Src\qtbase\src\sql\kernel\qsqlfield.h"
#include <QtGui\qicon.h>
#include "QBarApplication.h"


TransactionsViewModel::TransactionsViewModel(QObject* parent /*= 0*/, QSqlDatabase db /*= QSqlDatabase()*/):
  QSqlTableModel(parent, db)
  {
  m_trans_colors[TransactionStatus::WaitingForPayment] =         QBrush(QColor(255,0,0, 128));
  m_trans_colors[TransactionStatus::Sold] =                      QBrush(QColor(0,255,0, 128));
  m_trans_colors[TransactionStatus::WaitingForPurchasePayment] = QBrush(QColor(255,0,0, 64));
  m_trans_colors[TransactionStatus::Bought] =                    QBrush(QColor(0,255,0, 64));
  m_trans_colors[TransactionStatus::PrepareGoodwithdrawal] =     QBrush(QColor(255,255,0, 128));
  m_trans_colors[TransactionStatus::Goodwithdrawal] =            QBrush(QColor(255,255,0, 64));
  m_trans_colors[TransactionStatus::SaleCanceled] =              QBrush(QColor(0,0,255, 64));
    }

//---------------------------------------------------------------------------------------------------------------------------------------
Qt::ItemFlags TransactionsViewModel::flags(const QModelIndex &index) const
  {
  Qt::ItemFlags f = QSqlTableModel::flags(index);
  int row = index.row();
  int col = index.column();

  if(record(row).value(0).toInt() == TransactionStatus::WaitingForPurchasePayment && (col == TransactionsViewColumn::Number || col == TransactionsViewColumn::Price || col == TransactionsViewColumn::Sum))
    return Qt::ItemFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

  if(record(row).value(0).toInt() == TransactionStatus::WaitingForPayment && (col== TransactionsViewColumn::Number))
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
    auto i = m_trans_colors.find(tr_status);
    if(i!=m_trans_colors.end())
      return m_trans_colors.find(tr_status)->second;
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
