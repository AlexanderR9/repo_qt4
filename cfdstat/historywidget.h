 #ifndef HISTORY_WIDGET_H
 #define HISTORY_WIDGET_H

 #include "lchildwidget.h"
 #include "ui_historywidget.h"
 #include "datastructs.h"


class QSplitter;
class LChartWidget;
class DivCalc;



////////////////HistoryWidget///////////////////////////////////////////
class HistoryWidget : public LChildWidget, public Ui::HistoryWidget
{
    Q_OBJECT
public:
    HistoryWidget(QWidget *parent = 0);
    virtual ~HistoryWidget() {}

    void save(QSettings&);
    void load(QSettings&);
    void setPrice(const ConfiguratorAbstractRecord&);
    void saveData();
    void initBagData();
    void clearQuery();


protected:
    QSplitter *v_splitter;
    QSplitter *h_splitter;
    ConfiguratorAbstractData  m_operationsData;
    ConfiguratorAbstractData  m_pricesData;
    ConfiguratorAbstractData  m_companyData;
    LChartWidget *m_chart;
    DivCalc *m_calc;		

    void readGeneralData();
    void convertCompanyCurrency();
    bool invalidData() const;
    void initWidgets();
    void initTables();
    void initChart();
    void initCalc();
    void initComboboxes();
    void initTable(QTableWidget*);
    void fillTable(QTableWidget*);
    QStringList recToRow(const ConfiguratorAbstractRecord&, const QList<int>&) const;
    QList<int> headerList(const ConfiguratorAbstractData&) const;
    QString companyByID(int) const;
    QString companyCurrencyByID(int) const;
    void getPricesByID(int, const QString&, QMap<QString, double>&);
    void updateOperation(int);
    void convertToPricesRecord(const ConfiguratorAbstractRecord&, ConfiguratorAbstractRecord&);
    void sortDataByDate(ConfiguratorAbstractData&);
    int findMinDateOperation(const ConfiguratorAbstractData&, int) const;
    void updateBag(const ConfiguratorAbstractRecord&);
    double lastPrice(int, const QString&) const;
    double payedSize(int, const QString&) const;
    double divsSize(int, const QString&) const;
    int countPaper(int, const QString&) const;
    void updateColors();


public slots:
    void slotGetCurrentPrice(const ConfiguratorAbstractRecord&, double&);
    void slotNextOperation(int, const ConfiguratorAbstractRecord&);
    void slotSetCompanyByID(int id, QString &s) {s = companyByID(id);}
    void slotSetCurrencyByID(int id, QString &s) {s = companyCurrencyByID(id);}
    void slotSetHistoryData(ConfiguratorAbstractData *&data) {data = &m_operationsData;}


protected slots:
    void slotQuery();
    void slotEditOperation();
    void slotEditPrices();
    void slotPriceChart();

signals:
    void signalBagUpdate(const ConfiguratorAbstractRecord&);
    void signalGetDivData(ConfiguratorAbstractData*&);


private:
    void convertPricesToPoints(const QMap<QString, double>&, QList<QPointF>&);

};



 #endif


