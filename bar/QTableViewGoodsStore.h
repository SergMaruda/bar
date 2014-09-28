#ifndef QTABLEVIEWGOODSSTORE_H
#define QTABLEVIEWGOODSSTORE_H

#include <QTableView >

class QDragEnterEvent;
class QDropEvent;

class QTableViewGoodsStore : public QTableView 
{
  Q_OBJECT

public:
    QTableViewGoodsStore(QWidget *parent);
    ~QTableViewGoodsStore();

private:
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);
  void dragMoveEvent( QDragMoveEvent* );

  };

#endif // QTABLEVIEWGOODSSTORE_H
