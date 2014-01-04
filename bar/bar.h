#ifndef BAR_H
#define BAR_H

#include <QtWidgets/QMainWindow>
#include "ui_bar.h"

class bar : public QMainWindow
{
    Q_OBJECT

public:
    bar(QWidget *parent = 0);
    ~bar();

private:
    Ui::barClass ui;
private Q_SLOTS:
  void OnDoubleClick(QModelIndex);

};

#endif // BAR_H
