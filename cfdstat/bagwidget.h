 #ifndef BAG_WIDGET_H
 #define BAG_WIDGET_H

 #include "lchildwidget.h"
 #include "ui_generaldatawidget.h"
 #include "datastructs.h"

class LSearch;
class QSplitter;
class QCheckBox;

struct BagState
{
    BagState() {reset();}

    double payed;
    double cur_size;
    double div_size;
    int p_count;

    QString toStr() {return QString("size: %1/%2,  divs %3,  count %4").arg(QString::number(payed, 'f', 1)).arg(QString::number(cur_size, 'f', 1)).arg(QString::number(div_size, 'f', 1)).arg(p_count);}
    void reset() {payed = 0; cur_size = 0; div_size = 0; p_count = 0;}

};

////////////////BagWdiget///////////////////////////////////////////
class BagWidget : public LChildWidget, public Ui::ConfiguratorGeneralDataWidget
{
    Q_OBJECT
public:
    BagWidget(QWidget *parent = 0);
    virtual ~BagWidget() {}

    void buy(const ConfiguratorAbstractRecord&);
    void sell(const ConfiguratorAbstractRecord&);
    void refresh();

    virtual void save(QSettings&);
    virtual void load(QSettings&);

protected:
    LSearch *m_search;
    ConfiguratorAbstractData  m_data;
    BagState r_state; // cfd rub
    BagState u_state; // cfd usd
    BagState b_state; // bonds
    QSplitter *m_splitter;
    QTableWidget *m_statisticTable;
    QCheckBox *m_hideSellingCheckBox;

    ConfiguratorAbstractData  m_countryData;
    ConfiguratorAbstractData  m_companyData;


    void initBoxes();
    void initBagTable();
    void initStatisticTable();
    QList<int> headerList() const;
    int findRec(int, const QString&) const;
    QStringList recToRow(const ConfiguratorAbstractRecord&) const;
    QString companyIcon(const QString&) const;
    QString companyCurrency(int) const;
    void updateColors();
    void updateIcons();
    void recalcState(const ConfiguratorAbstractRecord&);
    void recalcState();
    void readGeneralData();
    void recalcStatisticTableData(); //обновление сводной таблицы, выполняется после заполнения таблицы портфеля

public slots:
    void slotBagUpdate(const ConfiguratorAbstractRecord&);
    void slotBagRefreshTable();
    
signals:
    void signalNextOperation(int, const ConfiguratorAbstractRecord&);
    void signalGetOperationsHistory(ConfiguratorAbstractData*&);
    void signalGetDivsData(ConfiguratorAbstractData*&);


protected slots:
    void slotTimer();
    void slotHideSoldChanged();
    void slotWasSearch();

private:
    void setStatisticRow(int, const double&, const double&,	quint8 precision = 1);

    //вычисления для заполнения сводной таблицы (статы)
    void statistic_calcBuySum(const ConfiguratorAbstractData*); //определение суммы потраченной на покупку всех бумаг (вместе с добавленной коммисией)
    void statistic_calcSellSum(const ConfiguratorAbstractData*); //определение суммы полученной при продаже всех бумаг (с учетом вычтенной коммисии)
    void statistic_calcPaperCount(const ConfiguratorAbstractData*); //определение текущего количетсва всех бумаг разных типов
    void statistic_calcBagPrice(); //определение текущей стоимости портфеля (цена всех бумаг на сегодня)
    void statistic_calcCommissionSum(const ConfiguratorAbstractData*); //определение суммы удержанной коммисии при всех операциях купли/продажи бумаг
    void statistic_calcTaxSum(const ConfiguratorAbstractData*); //определение суммы налогов, удержанной при выплате дивов
    void statistic_calcReceivedDivs(const ConfiguratorAbstractData*); //определение суммы всех полученных купонов и дивов (чистая сумма т.е. с учетом вычтенного налога)

};





 #endif


