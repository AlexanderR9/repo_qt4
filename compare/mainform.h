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

    QString mainTitle() const {return QString("Compare files, (Qt4)!");}
    QString projectName() const {return "compare";};
    void load();
    void save();

    void startCompare();
    void loadData();
    void getDirStructs();

protected slots:
    void slotAction(int); //virtual slot from parent

};


class LSearch;
class QSplitter;
class QTableWidget;


class MainWidget : public QWidget , public Ui_MainWidget
{
    Q_OBJECT
public:
	MainWidget(QWidget *parent = 0);
    virtual ~MainWidget() {}

    void load(QSettings&);
    void save(QSettings&);

    void readFiles(QString, QString);
    void getDirStructs(QString, QString, bool);
    void compareRPMS();
    void removeRows();

    inline void setBinary(bool b) {is_binary = b;}

protected:
    QStringList headerLabels();
    void initTable();
    bool is_binary;

    QStringList host1_rpms;
    QStringList host2_rpms;
    LSearch	*m_search2;
    LSearch	*m_search3;
    QSplitter 	*h_splitter;


    void readFile(const QString&, QStringList&);
    void readBinFile(const QString&, QStringList&);
    void fillTable(const QStringList&, QTableWidget*);
    void compareRPMS(const QStringList&, const QStringList&, QTableWidget*);
    void removeRows(QTableWidget*);
    void getFiles(const QString&, QStringList&);
    void getFolders(const QString&, QStringList&);
    void setFileSizeColumn(const QString&, QTableWidget*);
    void compareBin();


private:
    void nextDir(const QString&, QString, QStringList&);

};


#endif

