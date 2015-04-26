#include "QBarApplication.h"
#include <QtSql\qsqltablemodel.h>
#include <QtSql\qsqldatabase.h>
#include <QtWidgets\qmessagebox.h>
#include <QtSql\qsqlquery.h>
#include <QtGui\qicon.h>
#include <QtCore\qregularexpression.h>
#include <QtSql\qsqlerror.h>
#include <QtCore\qfileinfo.h>

static QBarApplication* g_inst = nullptr;

static bool tryOpenExisting(QSqlDatabase& i_db, const QString& str)
  {
  i_db.setDatabaseName(str);
  return QFileInfo::exists(str) && i_db.open();
  }

//--------------------------------------------------------------------
QBarApplication::QBarApplication(int &argc, char **argv)
  : QApplication(argc, argv),
  m_settings("SimpleSoft", "BarMgm")
  {
  g_inst = this;

  QSqlDatabase m_db = QSqlDatabase::addDatabase("QSQLITE");
  bool ok = tryOpenExisting(m_db, ".\\bar.db");
  if(!ok)
    ok = tryOpenExisting(m_db, "..\\..\\..\\Database\\bar.db");

  if(!ok)
    ok = tryOpenExisting(m_db, "..\\Database\\bar.db");

  if(!ok)
    QMessageBox::critical(nullptr, "Critial error", "No database file found");

  if(ok)
    {
    QSqlQuery query("PRAGMA foreign_keys = ON;");
    }
  if(!ok)
    {
    QMessageBox::critical(nullptr,"Ошибка открытия базы данных", "Невозможно открыть файл базы данных. Возможно отсутствует файл либо доступ на чтение");
    return;
    }

  mp_model_users = new QSqlTableModel(this);
  mp_model_users->setTable("USERS");
  mp_model_users->select();
  m_login_date = QDateTime::currentDateTime();
  }

//--------------------------------------------------------------------
QBarApplication::~QBarApplication()
  {
  }

//--------------------------------------------------------------------
QBarApplication* QBarApplication::instance()
  {
  return g_inst;
  }

//--------------------------------------------------------------------
int QBarApplication::currentUser()
  {
  return m_current_user_id;
  }

//--------------------------------------------------------------------
QSqlTableModel* QBarApplication::model_users()
  {
  return mp_model_users;
  }

//--------------------------------------------------------------------
QString QBarApplication::userPassword(int id) const
  {
  QSqlQuery query(QString("SELECT PASSWORD FROM USERS WHERE USER_ID = %1").arg(id));
  query.next();
  return query.value(0).toString();
  }

//--------------------------------------------------------------------
QString QBarApplication::userName( int id ) const
  {
  QSqlQuery query(QString("SELECT LOGIN FROM USERS WHERE USER_ID = %1").arg(id));
  query.next();
  return query.value(0).toString();
  }

//--------------------------------------------------------------------
void QBarApplication::setCurrentUser(int id)
  {
  m_current_user_id = id;
  }

//--------------------------------------------------------------------
void QBarApplication::removeGoodIcon( int id )
  {
  m_icons.erase(id);
  }

//--------------------------------------------------------------------
QIcon QBarApplication::goodIcon( int id ) const
  {
  auto icn = m_icons.find(id);
  if(icn != m_icons.end())
    return icn->second;

  QString query_str = QString("SELECT id, icon from goods_icons where id = %1").arg(id);
  QSqlQuery query;
  query.exec(query_str);
  bool res = query.next();

  if(!res)
    {
    query_str = QString("SELECT id, icon from goods_icons where id = %1").arg(9999);
    query.exec(query_str);
    res = query.next();
    }

  if(res)
    {
    QByteArray image_arr = query.value(1).toByteArray();
    QPixmap pic;
    pic.loadFromData(image_arr);
    QIcon icon(pic);
    m_icons[id] = icon;
    return icon;
    }
  return QIcon();
  }

//--------------------------------------------------------------------
QString QBarApplication::currentUserName() const
  {
  return userName(m_current_user_id);
  }

//--------------------------------------------------------------------
QString QBarApplication::currentUserPassword() const
  {
  return userPassword(m_current_user_id);
  }

//--------------------------------------------------------------------
QSettings& QBarApplication::settings()
  {
  return m_settings;
  }

//--------------------------------------------------------------------
int QBarApplication::_userRole( int id ) const
  {
  QSqlQuery query(QString("SELECT ROLE_ID FROM USERS WHERE USER_ID = %1").arg(id));
  query.next();
  return query.value(0).toInt();
  }

//--------------------------------------------------------------------
int QBarApplication::currentUserRole()
  {
  return _userRole(m_current_user_id);
  }

//--------------------------------------------------------------------
int QBarApplication::userID( QString i_user_name ) const
  {
  QSqlQuery query(QString("SELECT USER_ID FROM USERS WHERE LOGIN = \"%1\"").arg(i_user_name));
  if(query.next())
    return query.value(0).toInt();
  return -1;
  }

//--------------------------------------------------------------------
bool QBarApplication::changePassword(const QString& old_pass, const QString& new_pass )
  {
  if(old_pass == currentUserPassword())
    {
    QRegularExpression reg("^(?=.*\\d).{4,15}$");
    QRegularExpressionMatch m = reg.match(new_pass); 
    if(m.hasMatch())
      {
      QString str = QString("UPDATE USERS SET PASSWORD=\"%1\" WHERE USER_ID=%2").arg(new_pass).arg(currentUser());
      QSqlQuery query(str);
      auto err = query.lastError();

      return err.isValid() == false;
      }
    else
      return false;
    }
  return false;
  }
