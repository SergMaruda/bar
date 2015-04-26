#include "bar.h"
#include "QBarApplication.h"
#include "QLoginDialog.h"


extern "C" _declspec(dllexport) int StartPlugin(int argc, char *argv[]);


int StartPlugin(int argc, char *argv[])
  {
  QBarApplication* p_app(nullptr);
  int res = 0;
  do 
    {
    QBarApplication a(argc, argv);
    p_app= &a;
    QLoginDialog diag;
    if(diag.exec() == QLoginDialog::Rejected)
      {
      return 0;
      }

    bar w;
    w.show();
    int res = p_app->exec();
    }
  while(res != 10);

  return 0;
  }
