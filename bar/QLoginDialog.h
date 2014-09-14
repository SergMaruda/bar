#ifndef QLOGINDIALOG_H
#define QLOGINDIALOG_H

#include <QDialog>
namespace Ui {class QLoginDialog;};

class QLoginDialog : public QDialog
{
    Q_OBJECT

public:
    QLoginDialog(QWidget *parent = 0);
    ~QLoginDialog();

private:
    Ui::QLoginDialog *ui;
private slots:
  virtual void accept();
};

#endif // QLOGINDIALOG_H
