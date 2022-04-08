 #include "qapplication.h"
 #include "mainwidget.h"

int main(int argc, char** argv)
{
    QApplication app( argc, argv );
    CFT_MainWidget mw;
    mw.show();
    return app.exec();
}

