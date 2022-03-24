 #ifndef ICALC_H 
 #define ICALC_H 

 #include "qobject.h"
 #include "qcolor.h"
 
 #define PRECISION			3
// #define MAX_INSURANCE_PERCENT		10
 #define MAX_CREDIT_PERCENT		30
 #define MAX_FP_PERCENT			80        

struct IRecord
{
    IRecord() :payment(0), inDebt(0), morePayment(0), curDebt(0), insurance(0), sum(0), month("---"), color(255,255,255){}

    double payment;
    double inDebt;
    double morePayment;
    double curDebt;
    double insurance;
    double sum;
    QString month;
    QColor color;
    
    inline void calcSum(double d) {sum = d + payment + morePayment + insurance;}
};

class ICalc
{

public:
    ICalc();
    void resetParameters();
    void checkParameters();

    inline void setPrice(double p) {m_price = p;}
    inline void setCreditPercent(double p) {m_creditPercent = p;}
    inline void setInsuranceSum(double p) {m_insuranceSum = p;}
    inline void setYears(uint y) {m_years = y;}

    inline double price() const {return m_price;}

    inline void setMorePaymentSum(double s) {m_mpSum = s;}
    inline void setMorePaymentReriod(uint p) {m_mpPeriod = p;}
    inline void setIsMaxMonthPayment(bool b) {isMaxMonthPayment = b;}

    void setFirstPaymentPercent(double p);
    void setFirstPaymentSum(double p);
    void setCreditSize(double p);

    // calc first payment
    void fpByPercent(double);
    void fpBySum(double);
    void fpByCreditSize(double);

    inline double firstPaymentPercent() const {return m_fpPercent;}
    inline double firstPaymentSum() const {return m_fpSum;}
    inline double creditSize() const {return m_creditSize;}

    inline QString err() const {return m_err;}
    inline bool isValidState() {checkParameters(); return m_err.trimmed().isEmpty();}
    inline int recordsCount() const {return m_records.count();}
//    inline QString allPeriod() const {return QString("%1 %2, %3 %4").arg(m_records.count()/12).arg(QString::fromUtf8("лет")).arg(m_records.count()%12).arg(QString::fromUtf8("месяцев"));};
    inline QString allPeriod() const {return QString("%1/%2").arg(m_records.count()/12).arg(m_records.count()%12);};
    
    bool isValidCredit() const;
    double monthPayment(uint, double);
    double monthPayment();
    void recalc();
    double insuracePayed() const;

    IRecord recordAt(uint k) const {return ((k < uint(m_records.count())) ? m_records.at(int(k)) : IRecord());}
    IRecord lastRecord() const {return (!m_records.isEmpty() ? m_records.last() : IRecord());}
    
protected:
    double 	m_price;
    double 	m_creditPercent;
    double 	m_insuranceSum;
    uint   	m_years;

    double 	m_fpPercent;
    double 	m_fpSum;
    double 	m_creditSize;

    double 	m_mpSum;
    uint 	m_mpPeriod;
    bool	isMaxMonthPayment;

    QString	m_err;

    ////////////////////////////////////
    QList<IRecord> m_records;

        

};


 #endif 


