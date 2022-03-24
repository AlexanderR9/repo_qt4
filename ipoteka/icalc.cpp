 #include "icalc.h"
 #include "math.h"
 #include "qdebug.h"


 #define EVENT_COLOR	QColor(220, 220, 220)
 #define ODD_COLOR	QColor(220, 235, 220)

ICalc::ICalc()
    :isMaxMonthPayment(false)
{
    resetParameters();
}
void ICalc::recalc()
{
    m_records.clear();
    if (!isValidState()) return;
    
    double debt = m_creditSize;
    uint n_month = m_years*12;
    double b = m_creditPercent/double(12*100);

    uint index = 0;
    while (debt > 0)
    {
	index++;
	double p = monthPayment(n_month, debt);    
	if (p <= 0) break;
    
	IRecord ir;
	uint cur_month = index%12;
	uint cur_year = 1 + (index-1)/12;
	if (cur_month == 0) cur_month = 12;
	ir.month = QString("%1/%2").arg(cur_year).arg(cur_month);
	if (cur_year%2 == 0) ir.color = EVENT_COLOR;
	else ir.color = ODD_COLOR;

	ir.payment = p;
	ir.inDebt = ir.payment - debt*b;
	ir.curDebt = debt - ir.inDebt;

//////////////////////////////////////////////////////////////////////////////

    	if (index%m_mpPeriod == 0) 
	{
	    if (isMaxMonthPayment)
	    {
		double t = m_mpSum - p;
		if (t > ir.curDebt) ir.morePayment = ir.curDebt;
		else   ir.morePayment = t;
	    }
	    else
	    {	    
		if (m_mpSum > ir.curDebt) ir.morePayment = ir.curDebt;
		else   ir.morePayment = m_mpSum;
	    }
    	    ir.curDebt -= ir.morePayment; 		
	}

//////////////////////////////////////////////////////////////////////////////

	if (ir.curDebt < 0 )ir.curDebt = 0;
//    	if (index%12 == 1) ir.insurance = debt*(m_insurancePercent/double(100));	
    	if (index%12 == 1) ir.insurance = m_insuranceSum;	
	if (m_records.isEmpty()) ir.calcSum(0);
	else ir.calcSum(m_records.last().sum);
	m_records.append(ir);

	debt = ir.curDebt;	
	n_month--;
	if (n_month == 0) break;
    }
}
double ICalc::monthPayment()
{
    return monthPayment(m_years*12, m_creditSize);
}
double ICalc::monthPayment(uint n_month, double c_size)
{
    if (!isValidState() || n_month == 0) return -1;
    double b = m_creditPercent/double(12*100);
    double t = pow((b+1), n_month);
    double k = (t*b)/(t-1);
    //qDebug()<<QString("calc payment, cpercent=%0,  nmonth=%1,  csize=%2, [%3]").arg(m_creditPercent).arg(n_month).arg(c_size).arg(c_size*k);
    return (c_size*k);
}
void ICalc::checkParameters()
{
    m_err = "err state!";
    if (!isValidCredit()) return;
    if (m_creditPercent == -1) return;
    if (m_insuranceSum == -1) return;
    if (m_fpPercent == -1) return;
    if (m_fpSum == -1) return;
    if (m_creditSize == -1) return;
    m_err.clear();
}
void ICalc::resetParameters()
{
    m_creditPercent = -1;
    m_insuranceSum = -1;
    m_fpPercent = -1;
    m_fpSum = -1;
    m_creditSize = -1;

    m_err = "err state!";
    m_records.clear();
}
bool ICalc::isValidCredit() const
{
    if (m_creditPercent < 0 || m_creditPercent > MAX_CREDIT_PERCENT) return false;
//    if (m_insurancePercent < 0 || m_insurancePercent > MAX_INSURANCE_PERCENT) return false;
    if (m_insuranceSum < 0) return false;
    return true;
}
void ICalc::setFirstPaymentPercent(double p)
{
    if (p < 0 || p > MAX_FP_PERCENT || !isValidCredit()) return;
    m_fpPercent = p;
    m_fpSum = 0.01*p*m_price;
    m_creditSize = m_price - m_fpSum;
}
void ICalc::setFirstPaymentSum(double p)
{
    if (p < 0 || p > m_price || !isValidCredit()) return;
    m_fpPercent = 100*p/m_price;
    if (m_fpPercent > MAX_FP_PERCENT) {m_fpSum = -1; m_fpPercent = -1; return;}
    m_fpSum = p;
    m_creditSize = m_price - m_fpSum;
}
void ICalc::setCreditSize(double p)
{
    if (p < 0 || p > m_price || !isValidCredit()) return;
    m_fpPercent = 100*(m_price - p)/m_price;
    if (m_fpPercent > MAX_FP_PERCENT) {m_fpSum = -1; m_fpPercent = -1; return;}
    m_fpSum = m_price - p;
    m_creditSize = p;
}
double ICalc::insuracePayed() const
{
//    if (!isValidState()) return -1;
    double s = 0;
    for (int i=0; i<m_records.count(); i++) 
	s += m_records.at(i).insurance;
    return s;
}
