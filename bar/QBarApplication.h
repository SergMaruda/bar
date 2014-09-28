#ifndef QBARAPPLICATION_H
#define QBARAPPLICATION_H

#include <QApplication>
#include <QtCore\qsettings.h>
#include <QtCore\qdatetime.h>
#include <map>
#include <QtGui\qicon.h>

class QSqlTableModel;

class QBarApplication : public QApplication
{
  Q_OBJECT

public:
    QBarApplication(int &argc, char **argv);
    ~QBarApplication();
    static QBarApplication* instance();
    int currentUser();
    int currentUserRole();
    QString currentUserName() const ;
    void setCurrentUser(int);
    QSqlTableModel* model_users();
    QString userPassword(int id) const;
    QString userName(int id) const;
    int userID(QString i_user_name) const;
    QIcon goodIcon(int id) const;
    void removeGoodIcon(int id);

    QSettings& settings();

private:
    int _userRole(int id) const;

    int m_current_user_id;
    QDateTime m_login_date;
    QSqlTableModel* mp_model_users;
    QSettings m_settings;
    mutable std::map<int, QIcon> m_icons;
};

#endif // QBARAPPLICATION_H
