 #ifndef MAINFORM_H
 #define MAINFORM_H 

#include "lmainwidget.h"
#include "ui_mainwidget.h"


class MainWidget;
class QSettings;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);

protected:
    MainWidget	*m_centralWidget;

    void initActions();
    void initWidgets();
    void initCommonSettings();

    QString mainTitle() const {return QString("Bradys table, (Qt4)!");}
    QString projectName() const {return "bradys";};
    void load();
    void save();

    void start();

protected slots:
    void slotAction(int); //virtual slot from parent

};




class MainWidget : public QWidget , public Ui_MainWidget
{
    Q_OBJECT
public:
	MainWidget(QWidget *parent = 0);
    virtual ~MainWidget() {}

    void calc(quint8);
    void load(QSettings&);
    void save(QSettings&);

protected:
    QStringList headerLabels();
    void initTable();
    void addBradisRow(double, quint8);


private:
    double pi() {return 3.14159265;}

};


#endif

