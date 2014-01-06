#ifndef BAR_H
#define BAR_H

#include <QtWidgets/QMainWindow>
#include "ui_bar.h"
#include <QtSql/qsqldatabase.h>

class QSqlTableModel;
class QSqlDatabase;

class bar : public QMainWindow
{
    Q_OBJECT

public:
    bar(QWidget *parent = 0);
    ~bar();

private:
    Ui::barClass ui;
    QSqlTableModel* model_goods;
    QSqlTableModel* model_transactions_view;
    QSqlTableModel* model_transactions;
    QSqlDatabase m_db;
private Q_SLOTS:
  void OnDoubleClick(QModelIndex);
  void OnOnConfirmSelling();

};

#endif // BAR_H
