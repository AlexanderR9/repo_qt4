 #ifndef DIVSTAT_WIDGET_H
 #define DIVSTAT_WIDGET_H

 #include "lchildwidget.h"
 #include "ui_divstatwidget.h"
 #include "datastructs.h"


class QSplitter;
class LSearch;
class LChartWidget;
class DivCalc;


////////////////DivStatWidget///////////////////////////////////////////
class DivStatWidget : public LChildWidget, public Ui::DivStatWidget
{
    Q_OBJECT
public:
    DivStatWidget(QWidget *parent = 0);
    virtual ~DivStatWidget() {}

    void save(QSettings&);
    void load(QSettings&);

protected:
    QSplitter *v_splitter;
    QSplitter *h_splitter1;
    QSplitter *h_splitter2;
    LSearch *m_search;
    LSearch *m_search2;
    LSearch *m_search3;
    LChartWidget *m_chart;
    DivCalc *m_calc;
    ConfiguratorAbstractData  m_data;

    void saveTableState(QSettings&, const QTableWidget*);
    void loadTableState(QSettings&, QTableWidget*);


    void initChart();
    void initWidgets();
    void initCalc();
    void initSearchs();
    void initTables();
    void fillCalendarTable();
    void fillStatTable(ConfiguratorAbstractData*);
    void fillBagTable(ConfiguratorAbstractData*);
    void updateColors();
    void repaintStatChart(QTableWidget*);

    QList<int> headerList() const;
    QList<int> headerList2() const;
    QList<int> headerList3() const;
    QStringList recToRow(const ConfiguratorAbstractRecord&, const QList<int>&) const;

signals:
    void signalGetCompanyByID(int, QString&);
    void signalGetCurrencyByID(int, QString&);
    void signalGetDivData(ConfiguratorAbstractData*&);
    void signalGetHistoryData(ConfiguratorAbstractData*&);


protected slots:
    void slotTimer();
    void slotCalendarChart();
    void slotStatChart();
    void slotBagChart();


};



 #endif


