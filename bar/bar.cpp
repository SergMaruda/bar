#include "stdafx.h"
#include "bar.h"

#include <QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QSqlTableModel>
#include <QtSql/qsqltablemodel.h>


bar::bar(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test");
    bool ok = db.open();

    //QSqlQuery query;
    //query.exec("SELECT * FROM Goods");

    //qDebug() << query.value(0).toString();


    QSqlTableModel *model = new QSqlTableModel(this, db);
    
      //model->setEditStrategy();
    model->setTable("Goods");
    model->setFilter("DELETED=1");
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->select();

    //model->setHeaderData(0, Qt::Horizontal, "id");
    model->setHeaderData(1, Qt::Horizontal, "Name");
    model->setHeaderData(2, Qt::Horizontal, "Salary");

   model->insertRow(0);
    
 //   model->submitAll();
    QString err = model->lastError().text();
    
    ui.tableView->setModel(model);
    //ui.tableView->hideColumn(0); // don't show the ID
    ui.tableView->show();
}

bar::~bar()
{

}
