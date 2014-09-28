#include "QLoginDialog.h"
#include "ui_QLoginDialog.h"
#include "QBarApplication.h"
#include <QtSql\qsqltablemodel.h>
#include <QtSql\qsqlrecord.h>
#include <QToolTip>
#include <qstatusbar>


QLoginDialog::QLoginDialog(QWidget *parent)
    : QDialog(parent)
  {
  ui = new Ui::QLoginDialog();
  ui->setupUi(this);

  ui->lineEditPassword->setEchoMode(QLineEdit::Password);
  auto users = QBarApplication::instance()->model_users();
  
  auto app = QBarApplication::instance();
  bool save = app->settings().value("login/save_password", false).toBool();
  QCheckBox* p_check = ui->checkBoxSave;
  p_check->setChecked(save);

  if(save)
    {
    QString pass = app->settings().value("login/last_password").toString();
    QString user = app->settings().value("login/last_user").toString();
    ui->lineEditPassword->setText(pass);
    ui->lineEditUser->setText(user);
    }

  }

//----------------------------------------------------------------------------------------------
QLoginDialog::~QLoginDialog()
  {
  delete ui;
  }

//----------------------------------------------------------------------------------------------
void QLoginDialog::accept()
  {
  auto app = QBarApplication::instance();
  QString user_name = ui->lineEditUser->text();
  int user_id = app->userID(user_name);
  QPoint pos = QCursor::pos();
  QString pass = app->userPassword(user_id);

  if(ui->lineEditPassword->text() == pass && !pass.isEmpty())
    {
    app->setCurrentUser(user_id);

    auto app = QBarApplication::instance();
    bool is_checked = ui->checkBoxSave->isChecked();
    app->settings().setValue("login/save_password", is_checked);

    if(!is_checked)
      {
      pass = "";
      user_name = "";
      }

    app->settings().setValue("login/last_password", pass);
    app->settings().setValue("login/last_user", user_name);


    QString new_pwd = ui->lineEditNewPassword->text();
    QString new_pwd_copy = ui->lineEditNewPasswordCopy->text();

    if(!new_pwd.isEmpty())
      {
      if(new_pwd != new_pwd_copy)
        {
        std::wstring str = L"Новый пароль не верный";
        QToolTip::showText(pos, QString::fromUtf16(str.c_str()), 0, QRect(), 500);
        return;
        }
      else
        {
        if(false == app->changePassword(pass, new_pwd))
          {
          std::wstring str = L"Пароль должен быть длиной от 4-х до 15-ти символов и содержать латинские буквы и цифри";
          QToolTip::showText(pos, QString::fromUtf16(str.c_str()), 0, QRect(), 500);
          return;
          }
        }
      }
    QDialog::accept();
    }
  else
    {
    std::wstring str = L"Неправильный логин/пароль";
    QToolTip::showText(pos, QString::fromUtf16(str.c_str()), 0, QRect(), 500);
    }
  }
