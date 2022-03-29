 #ifndef DIV_CALC_H
 #define DIV_CALC_H


 #include "datastructs.h"


 #include <QObject>
 #include <QDate>

////////////////DivCalc///////////////////////////////////////////
class DivCalc : public QObject
{
    Q_OBJECT
public:
    DivCalc(const ConfiguratorAbstractData&, QObject *parent = 0);
    virtual ~DivCalc() {}

    void readGeneralData(QString&);
    void updateDivInfo();
    inline const ConfiguratorAbstractData& divInfo() const {return m_divs;}
    inline bool isEmpty() const {return m_divs.isEmpty();}
    void fillCalendarData(ConfiguratorAbstractData&);

protected:
    ConfiguratorAbstractData  m_calendarData;
    ConfiguratorAbstractData  m_bondData;
    ConfiguratorAbstractData  m_divs;
    const ConfiguratorAbstractData& m_operationsData;

    bool invalidData() const;
    int paperCountByDate(int, const QDate&, QString pt = "cfd", QString kks="") const; //количество бумаг, которое было в портфеле на указанную дату
    QList<QDate> emulCouponsDates(const QDate&, int) const;
    void insertRecByDate(ConfiguratorAbstractData&, ConfiguratorAbstractRecord&);

signals:
    void signalGetCompanyByID(int, QString&);
    void signalGetCurrencyByID(int, QString&);
    void signalGetDivData(const ConfiguratorAbstractData*);

public slots:
    void slotSetDivData(ConfiguratorAbstractData *&data) {data = &m_divs;}


};


 #endif


