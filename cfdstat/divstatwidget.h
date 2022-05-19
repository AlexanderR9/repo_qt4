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
    LSearch *m_search;
    LSearch *m_search2;
    LChartWidget *m_chart;

    void saveTableState(QSettings&, const QTableWidget*);
    void loadTableState(QSettings&, QTableWidget*);


    void initChart();
    void initWidgets();
    void initSearchs();
    void initTables();

    QList<int> headerList() const;
    QList<int> headerList2() const;
    void fillCalendarTable(const ConfiguratorAbstractData*);
    void fillStatTable(ConfiguratorAbstractData*);

    void updateDaysTo(); //обновить столбец Note в calendarTable (daysTo)
    void updateColorsCalendar(const ConfiguratorAbstractData*); //раскрасить блоки по месяцам в calendarTable
    void updateColorsStat(); //обозначить границы переходов на следующий месяц в calendarTable
    void calcStatChartPoints(int, QList<QPointF>&, QList<QPointF>&);
    QStringList recToRow(const ConfiguratorAbstractRecord&, const QList<int>&) const;
    QString recField(const ConfiguratorAbstractRecord&, int) const;

signals:
    void signalGetCompanyByID(int, QString&) const;
    void signalGetCurrencyByID(int, QString&) const;
    void signalGetDivsData(ConfiguratorAbstractData*&);
    void signalGetWaitDays(QList<quint8>&);

protected slots:
    void slotTimer();
    void slotCalendarChart();
    void slotStatChart();

};



#endif


