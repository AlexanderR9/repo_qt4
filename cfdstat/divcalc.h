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

    //загрузить 2 таблицы m_calendarData и m_bondData из соответствующих файлов,
    //для дальнейшего определения графика дивов и купонов и их размеров соответственно
    void readGeneralData(QString&);

    void updateDivInfo(); //пересчитать контейнер m_divs
    inline bool isEmpty() const {return m_divs.isEmpty();}

protected:
    ConfiguratorAbstractData  m_calendarData;	//график дивов
    ConfiguratorAbstractData  m_bondData;		//график купонов
    const ConfiguratorAbstractData& m_operationsData; //история опрераций покупки/продажи всех бумаг


    //каждая запись m_divs содержит следующие поля:
    //ftTypeOperation - тип операции (opCouponReceive или opDivReceive)
    //ftCompany - ID of company
    //ftKKS - kks for bond or empty string for CFD
    //ftDateCoupon - date of coupons/divs
    //ftCount -	количество бумаг на дату отсечки ftDateCoupon
    //ftDivSize - размер выплаты в той валюте в которой покупалась бумага (за 1 шт.)
    //ftDivDelay - задержка выплаты в днях
    //ftNalogSize - коэффициент налога от ftDivSize (пример 0,13)
    ConfiguratorAbstractData  m_divs;			//история всех дивов и купонов и их размер (расчетные данные)



    bool invalidData() const; //признак ошибке загрузки данных из файлов при инициализации объекта (m_calendarData и m_bondData)
    int paperCountByDate(const QDate&, int, QString kks="") const; //количество бумаг, которое было в портфеле на указанную дату, если kks!="" то это bond
    void updateDivsHistory();
    void updateCouponsHistory();
    QList<QDate> emulCouponsDates(const ConfiguratorAbstractRecord&) const; //выдать список дат выплаты купонов для заданной записи (only bond)
    void insertRecByDate(ConfiguratorAbstractData&, ConfiguratorAbstractRecord&); //вставить запись в таблицу, так чтобы она была сортирована по дате дивов/купонов

signals:
    void signalGetCompanyByID(int, QString&);
    void signalGetCurrencyByID(int, QString&);
    void signalGetDivData(const ConfiguratorAbstractData*);

public slots:
    void slotSetDivsData(ConfiguratorAbstractData *&data) {data = &m_divs;}
    void slotGetWaitDays(QList<quint8>&); //список количества дней ожидания до следующей выплаты


};


 #endif //DIV_CALC_H


