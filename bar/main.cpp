#include "bar.h"
#include "QBarApplication.h"
#include "QLoginDialog.h"

int main(int argc, char *argv[])
{
    QBarApplication a(argc, argv);

    QLoginDialog diag;
    if(diag.exec() == QLoginDialog::Rejected)
      {
      return 0;
      }

    bar w;
    w.show();
    return a.exec();
}
