 #include "divcalc.h"
 #include "cfdenums.h"
 #include "lcommonsettings.h"
 #include "generaldatawidget.h"


 #include <QDate>
 #include <QDir>
 #include <QDebug>


DivCalc::DivCalc(const ConfiguratorAbstractData &hd, QObject *parent)
    :QObject(parent),
    m_operationsData(hd)
{

}
void DivCalc::readGeneralData(QString &err)
{
    m_calendarData.generalType = gdDivCalendar;
    m_bondData.generalType = gdBond;
    GeneralDataFileReader::loadDataFormFile(m_calendarData.generalType, m_calendarData);
    GeneralDataFileReader::loadDataFormFile(m_bondData.generalType, m_bondData);

    if (invalidData())
    {
        err = QObject::tr("Error loading general data, check application settings.");
        m_calendarData.reset();
        m_bondData.reset();
        return;
    }

    qDebug()<<QString("DivCalc::readGeneralData - Data loaded ok!  bonds count %1,  calendar count %2").arg(m_bondData.count()).arg(m_calendarData.count());
}
bool DivCalc::invalidData() const
{
    return (m_calendarData.invalid() || m_bondData.invalid());
}
void DivCalc::updateDivsHistory()
{
    int year1 = lCommonSettings.paramValue("first_div_date").toInt();
    int year2 = lCommonSettings.paramValue("last_div_date").toInt();

    for (int i=0; i<m_calendarData.count(); i++)
    {
    	const ConfiguratorAbstractRecord &rec = m_calendarData.records.at(i);
        QDate dt_div = QDate::fromString(rec.record.value(ftDateOperation), DATE_MASK);
        if (dt_div.year() < year1 || dt_div.year() > year2) continue;

    	int cid = rec.record.value(ftCompany).toInt();
        int n_papers = paperCountByDate(dt_div, cid);
        if (n_papers <= 0) continue;

        double nalog = rec.value(ftNalogP).toDouble()/100;

        //prepare div record
    	ConfiguratorAbstractRecord div_rec;
    	div_rec.record.insert(ftTypeOperation, QString::number(opDivReceive));
    	div_rec.record.insert(ftCompany, QString::number(cid));
    	div_rec.record.insert(ftKKS, QString());
    	div_rec.record.insert(ftDateCoupon, dt_div.toString(DATE_MASK));
    	div_rec.record.insert(ftCount, QString::number(n_papers));
    	div_rec.record.insert(ftDivSize, rec.value(ftDivSize));
    	div_rec.record.insert(ftDivDelay, rec.value(ftDivDelay));
    	div_rec.record.insert(ftNalogSize, QString::number(nalog, 'f', 2));

    	//insert to container by sort div_date
    	insertRecByDate(m_divs, div_rec);

        //qDebug()<<div_rec.toString();

    }
}
void DivCalc::updateCouponsHistory()
{
    for (int i=0; i<m_bondData.count(); i++)
    {
    	const ConfiguratorAbstractRecord &rec = m_bondData.records.at(i);
    	int cid = rec.record.value(ftCompany).toInt();
    	QString kks = rec.record.value(ftKKS);

    	//look dates coupons
    	QList<QDate> dates = emulCouponsDates(rec);
    	for (int j=0; j<dates.count(); j++)
    	{
    		const QDate& dt = dates.at(j);
    		int n_papers = paperCountByDate(dt, cid, kks);
            if (n_papers <= 0) continue;

            //prepare div record
        	ConfiguratorAbstractRecord coupon_rec;
        	coupon_rec.record.insert(ftTypeOperation, QString::number(opCouponReceive));
        	coupon_rec.record.insert(ftCompany, QString::number(cid));
        	coupon_rec.record.insert(ftKKS, kks);
        	coupon_rec.record.insert(ftDateCoupon, dt.toString(DATE_MASK));
        	coupon_rec.record.insert(ftCount, QString::number(n_papers));
        	coupon_rec.record.insert(ftDivSize, rec.value(ftCouponSize));
        	coupon_rec.record.insert(ftDivDelay, rec.value(ftDivDelay));
        	coupon_rec.record.insert(ftNalogSize, QString::number(0));

        	//insert to container by sort div_date
        	insertRecByDate(m_divs, coupon_rec);
    	}
    }
}
void DivCalc::updateDivInfo()
{
    m_divs.reset();

    updateDivsHistory();
    updateCouponsHistory();

    qDebug()<<QString("DivCalc::updateDivInfo()  data divs size %1").arg(m_divs.count());
    /*
    if (m_divs.count() > 10)
    {
    	for (int i=0; i<30; i++)
    		qDebug()<<m_divs.recAt(i).toString();
    }
    */
}

int DivCalc::paperCountByDate(const QDate &dt, int cid, QString kks) const
{
    int n = 0;
    for (int i=0; i<m_operationsData.count(); i++)
    {
        const ConfiguratorAbstractRecord &rec = m_operationsData.records.at(i);
        if (cid != rec.record.value(ftCompany).toInt()) continue;
        if (kks != rec.record.value(ftKKS).trimmed()) continue;

        QDate dt_operation = QDate::fromString(rec.record.value(ftDateOperation), DATE_MASK);
        if (dt_operation > dt) break;
        int p_count = rec.record.value(ftCount).toInt();
        int operation_type = rec.record.value(ftTypeOperation).toInt();
        switch (operation_type)
        {
        	case opBuy: {n += p_count; break;}
            case opSell: {n -= p_count; break;}
            default: break;
        }
    }
    return n;
}
void DivCalc::insertRecByDate(ConfiguratorAbstractData &data, ConfiguratorAbstractRecord &rec)
{
    if (!data.isEmpty())
    {
		QDate dt_rec = QDate::fromString(rec.record.value(ftDateCoupon), DATE_MASK);
		for (int i=0; i<data.count(); i++)
		{
			if (QDate::fromString(data.recAtValue(i, ftDateCoupon), DATE_MASK) < dt_rec) continue;
			data.records.insert(i, rec);
			return;
		}
    }
    data.records.append(rec);
}
QList<QDate> DivCalc::emulCouponsDates(const ConfiguratorAbstractRecord &bond_rec) const
{
	QList<QDate> list;
	if (!bond_rec.fields().contains(ftKKS) || !bond_rec.fields().contains(ftDateOffer) || !bond_rec.fields().contains(ftDateCoupon) || !bond_rec.fields().contains(ftCouponCount))
	{
		qWarning()<<QString("DivCalc::emulCouponsDates - WARNING rec is not bond data [%1]").arg(bond_rec.toString());
		return list;
	}


    QDate coupon_dt = QDate::fromString(bond_rec.record.value(ftDateCoupon), DATE_MASK);
    coupon_dt = coupon_dt.addYears(-20);
	int n_pay = bond_rec.record.value(ftCouponCount).toInt()/10;
    int add_days = ((n_pay == 4) ? 91 : 182);

    int year1 = lCommonSettings.paramValue("first_div_date").toInt();
    int year2 = lCommonSettings.paramValue("last_div_date").toInt();
    while (2 > 1)
    {
    	if (coupon_dt.year() > year2) break;
    	if (coupon_dt.year() < year1)
    	{
    		coupon_dt = coupon_dt.addYears(1);
    		continue;
    	}
    	list.append(coupon_dt);

    	for (int i=1; i<=(n_pay-1); i++)
    	{
    		QDate next_coupon_dt = coupon_dt.addDays(add_days*i);
    		if (next_coupon_dt.year() <= year2) list.append(next_coupon_dt);
    	}
    	coupon_dt = coupon_dt.addYears(1);
    }

    /*
    if (bond_rec.value(ftKKS).contains("100ZL"))
    {
    	qDebug()<<QString("year1=%1  year2=%2  n_pay=%3  add_days=%4").arg(year1).arg(year2).arg(n_pay).arg(add_days);
    	for (int i=0; i<list.count(); i++)
    		qDebug()<<QString("coupon date: %1").arg(list.at(i).toString(DATE_MASK));
    }
    */

	return list;
}
void DivCalc::slotGetWaitDays(QList<quint8> &list)
{
	list.clear();
	if (isEmpty()) return;

	list.append(0);
	QDate dt_prev = QDate::fromString(m_divs.records.first().record.value(ftDateCoupon), DATE_MASK);
	for (int i=1; i<m_divs.count(); i++)
	{
		QDate dt = QDate::fromString(m_divs.recAt(i).record.value(ftDateCoupon), DATE_MASK);
		list.append(dt_prev.daysTo(dt));
		dt_prev = dt;
	}
}




