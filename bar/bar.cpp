#include "bar.h"
#include <QtSql\qsqldatabase.h>
#include <QtSql\qsqltablemodel.h>

bar::bar(QWidget *parent)
    : QMainWindow(parent)
  {
  ui.setupUi(this);
  ui.tableViewGood->hideColumn(1);

   
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("..\\Database\\bar.db");
  bool ok = db.open();

  QSqlTableModel *model = new QSqlTableModel(this, db);

  model->setTable("Goods");
  model->setFilter("DELETED=0");
  model->setEditStrategy(QSqlTableModel::OnRowChange);
  model->select();
  ui.tableViewGood->setModel(model);

  model->setHeaderData(0, Qt::Horizontal, "Наименование");
  model->setHeaderData(1, Qt::Horizontal, "Цена");
  ui.tableViewGood->hideColumn(2); // don't show the ID
  ui.tableViewGood->resizeColumnsToContents();
  ui.tableViewGood->show();
   QObject::connect(ui.tableViewGood, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnDoubleClick(QModelIndex)));
  //connect(ui.tableViewGood,)
   ui.tableViewGood->mousePressEvent()
  }

bar::~bar()
  {
  }

void bar::OnDoubleClick(QModelIndex idx)
  {
  idx;
  int i(0);
  ++i;
  }
