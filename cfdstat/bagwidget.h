 #ifndef BAG_WIDGET_H
 #define BAG_WIDGET_H

 #include "lchildwidget.h"
 #include "ui_generaldatawidget.h"
 #include "datastructs.h"

class LSearch;

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

protected:
    LSearch *m_search;
    ConfiguratorAbstractData  m_data;
    BagState r_state; // cfd rub
    BagState u_state; // cfd usd
    BagState b_state; // bonds

    ConfiguratorAbstractData  m_countryData;
    ConfiguratorAbstractData  m_companyData;


    void initTable();
    QList<int> headerList() const;
    int findRec(int, const QString&) const;
    QStringList recToRow(const ConfiguratorAbstractRecord&) const;
    QString companyIcon(const QString&) const;
    void updateColors();
    void updateIcons();
    void recalcState(const ConfiguratorAbstractRecord&);
    void recalcState();
    void readGeneralData();

public slots:
    void slotBagUpdate(const ConfiguratorAbstractRecord&);
    
signals:
    void signalNextOperation(int, const ConfiguratorAbstractRecord&);

protected slots:
    void slotTimer();


};





 #endif


