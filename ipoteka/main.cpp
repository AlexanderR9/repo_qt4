    #include "qapplication.h"
    #include "mainwidget.h"
    
    
int main(int argc, char ** argv)
{
    QApplication app(argc,argv);
    MainWidget *form = new MainWidget();
    form->show();

    return app.exec();
};