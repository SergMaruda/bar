#include "bar.h"
#include <QtSql\qsqldatabase.h>
#include <QtSql\qsqltablemodel.h>
#include <QtSql\qsqlrecord.h>
#include <QtSql\qsqlquery.h>
#include <QtCore\qdatetime.h>


class QSqlTableModelMy: public QSqlTableModel
  {
  public:
  explicit QSqlTableModelMy(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()):
  QSqlTableModel(parent, db)
    {

    }

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const
    {
    if (role == Qt::CheckStateRole)
      return QVariant();

    if (role == Qt::BackgroundRole) 
      {
      int row = idx.row();
      QSqlRecord rec = record(row);
      int val = rec.value(0).toInt();

      if(val == 0)
        {
        QColor color(255,0,0, 128);           
        return QBrush(color);
        }
      if(val == 1)
        {
        QColor color(0,255,0, 128);           
        return QBrush(color);
        }
      }
    return QSqlTableModel::data(idx, role);
    }
  };


//------------------------------------------------------------
bar::bar(QWidget *parent)
    : QMainWindow(parent)
  {
  ui.setupUi(this);
  ui.tableViewGood->hideColumn(1);

   
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName("..\\Database\\bar.db");
  bool ok = m_db.open();
  if(!ok)
    {
    m_db.setDatabaseName("..\\..\\..\\Database\\bar.db");
    ok = m_db.open();
    }

  model_goods = new QSqlTableModel(this, m_db);

  model_goods->setTable("GOODS_VIEW");
  model_goods->select();
  ui.tableViewGood->setModel(model_goods);
  ui.tableViewGood->resizeColumnsToContents();
  ui.tableViewGood->show();
  QObject::connect(ui.tableViewGood, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnDoubleClick(QModelIndex)));
  QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(OnOnConfirmSelling()));

  model_transactions_view = new QSqlTableModelMy(this, m_db);

  model_transactions_view->setTable("TransactionsView");
  model_transactions_view->select();

  ui.tableViewTransactions->setModel(model_transactions_view);
  ui.tableViewTransactions->hideColumn(0);
  ui.tableViewTransactions->resizeColumnsToContents();

  model_transactions = new QSqlTableModel(this, m_db);
  model_transactions->setTable("Transactions");
  model_transactions->select();
  }

//------------------------------------------------------------
bar::~bar()
  {
  }

//------------------------------------------------------------
void bar::OnDoubleClick(QModelIndex idx)
  {
  int row = idx.row();
  QSqlRecord rec = model_goods->record(row);
  int good_id = rec.value(0).toInt();
  float good_price = rec.value(2).toFloat();

  QDateTime curr = QDateTime::currentDateTime();
  QSqlRecord rec_tr = model_transactions->record();

  QString query_str = QString("SELECT TR_ID FROM Transactions WHERE GOOD_ID = %1 AND STATUS = 0").arg(good_id);
  QSqlQuery sql_qur(m_db);
  bool qres = sql_qur.exec(query_str);

  int count = sql_qur.record().count();
  int tr_id(-1);
  
  if(sql_qur.next())
    tr_id = sql_qur.value(0).toInt();

  if(tr_id == -1)
    {
    rec_tr.setValue(0, QVariant());  //TR_ID
    rec_tr.setValue(1, good_id);     //GOOD_ID
    rec_tr.setValue(2, 0);           //STATUS
    rec_tr.setValue(3, 1);           //Number
    rec_tr.setValue(4, good_price);  //Price
    rec_tr.setValue(5, QString("")); //Comment
    rec_tr.setValue(6, curr);        //DATE
    bool res = model_transactions->insertRecord(-1, rec_tr);
    }
  else
    {
    QString query_str = QString("UPDATE Transactions SET Number=Number+1 WHERE TR_ID = %1").arg(tr_id);
    QSqlQuery sql_qur(m_db);
    bool qres = sql_qur.exec(query_str);
    }

  model_transactions->submitAll();
  model_transactions->select();
  model_transactions_view->select();

  ui.tableViewTransactions->scrollToBottom();
  }

//------------------------------------------------------------
void bar::OnOnConfirmSelling()
  {
  QString query_str = "UPDATE Transactions\n"
                      "SET STATUS=1\n"
                      "WHERE STATUS=0;";

  QSqlQuery sql_qur(m_db);
  bool res = sql_qur.exec(query_str);
  if(res == false);

  model_transactions_view->select();
  model_transactions->select();
  }
