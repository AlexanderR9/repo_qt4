 #ifndef MAINFORM_H
 #define MAINFORM_H 

 #include "lmainwidget.h"
 #include "lchildwidget.h"


class QTabWidget;
class QListWidget;
class QLabel;
class QSplitter;
class QStackedWidget;
class ConfiguratorGeneralDataWdiget;
class BagWidget;
class DivStatWidget;
class HistoryWidget;
class DivCalc;


// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);

protected:
    QStackedWidget 		*m_stackedWidget;
    DivCalc 			*m_divsCalcObj;

    QString projectName() const {return "cfdstat";}
    virtual void load();
    virtual void save();

    void initActions();
    void initWidgets();
    void initCommonSettings();
    void initCalcDivObj();

    QString mainTitle() const;
    void updateButtons();

    void saveCurrentData();
    void actGeneralData();
    void actBag();
    void actDivStat();
    void actBuy();
    void actSell();
    void actHistory();
    void actUpdatePrices();
    void actClearQueryHistory();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotWidgetChanged(int);

private:
    ConfiguratorGeneralDataWdiget* generalDataWidget() const;
    BagWidget* bagWidget() const;
    DivStatWidget* divStatWidget() const;
    HistoryWidget* operationsHistoryWidget() const;

};
 

 #endif






