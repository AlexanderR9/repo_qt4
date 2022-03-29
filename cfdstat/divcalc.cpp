 #include "divcalc.h"
 #include "cfdenums.h"
 #include "lcommonsettings.h"
 #include "generaldatawidget.h"


 #include <QDate>
 #include <QDir>
 #include <QDebug>


DivCalc::DivCalc(const ConfiguratorAbstractData &hd,QObject *parent)
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
        QString path = lCommonSettings.paramValue("datapath").toString().trimmed();
        if (!path.isEmpty())
        {
            QDir dir(path);
            if (dir.exists()) return;
        }

        err = QObject::tr("Error loading general data, check application settings.");
        return;
    }

    qDebug()<<QString("DivCalc::readGeneralData: Data loaded ok!  bonds count %1,  calendar count %2").arg(m_bondData.count()).arg(m_calendarData.count());
}
bool DivCalc::invalidData() const
{
    return (m_calendarData.invalid() || m_bondData.invalid());
}
void DivCalc::updateDivInfo()
{
    m_divs.reset();
    QDate cur_dt = QDate::currentDate();
    int prec = lCommonSettings.paramValue("precision").toInt();

    ///calendar cfd
    QString paper_text = ConfiguratorEnums::interfaceTextByType(gdCFD).trimmed();
    for (int i=0; i<m_calendarData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec_cal = m_calendarData.records.at(i);
	int cid = rec_cal.record.value(ftCompany).toInt();
        QDate dt = QDate::fromString(rec_cal.record.value(ftDateOperation), DATE_MASK);
	int delay = rec_cal.record.value(ftDivDelay).toInt();
//	qDebug()<<QString("DivCalc::updateDivInfo() i=%1  cid=%2  dt=%3").arg(i).arg(cid).arg(dt.toString(DATE_MASK));
	if ((dt > cur_dt) || (dt.daysTo(cur_dt) < delay)) continue;   

	int n_paper = paperCountByDate(cid, dt, paper_text);
	if (n_paper <= 0) continue;
//	qDebug()<<QString("finded %1 papers").arg(n_paper);

	///prepare new record
	dt.addDays(delay);
	ConfiguratorAbstractRecord rec;
	rec.record.insert(ftCompany, QString::number(cid));
	rec.record.insert(ftTypeOperation, QString::number(opDivReceive));
	rec.record.insert(ftCount, QString::number(n_paper));
	rec.record.insert(ftPaperType, paper_text);
	rec.record.insert(ftPrice1, QString::number(-1));
	rec.record.insert(ftCommission, QString::number(-1));
	rec.record.insert(ftKKS, QString());
	rec.record.insert(ftDateOperation, dt.toString(DATE_MASK));
	double div_sum = n_paper*rec_cal.record.value(ftDivSize).toDouble();
	double nalog_sum = div_sum*rec_cal.record.value(ftNalogP).toDouble()/100; 
	rec.record.insert(ftCouponSize, QString::number(div_sum, 'f', prec));
	rec.record.insert(ftNalogSize, QString::number(nalog_sum, 'f', prec));
	rec.record.insert(ftPrice, QString::number(div_sum-nalog_sum, 'f', prec));

	m_divs.records.append(rec);
    }

    ///calendar bonds
    paper_text = ConfiguratorEnums::interfaceTextByType(gdBond).trimmed();
    for (int i=0; i<m_bondData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec_cal = m_bondData.records.at(i);
	int cid = rec_cal.record.value(ftCompany).toInt();
	int n_pay = rec_cal.record.value(ftCouponCount).toInt()/10;
	QString kks = rec_cal.record.value(ftKKS);
        QDate dt = QDate::fromString(rec_cal.record.value(ftDateCoupon), DATE_MASK);
	int delay = rec_cal.record.value(ftDivDelay).toInt();
//	qDebug()<<QString("DivCalc::updateDivInfo() i=%1  cid=%2  dt=%3  kks=%4  n_pay=%5").arg(i).arg(cid).arg(dt.toString(DATE_MASK)).arg(kks).arg(n_pay);

	QList<QDate> dates = emulCouponsDates(dt, n_pay);
	for (int j=0; j<dates.count(); j++)
	{
	    dt = dates.at(j);
	    if ((dt > cur_dt) || (dt.daysTo(cur_dt) < delay)) continue;   
	//    qDebug()<<QString("    date_%1:   %2").arg(j+1).arg(dt.toString(DATE_MASK));

	    int n_paper = paperCountByDate(cid, dt, paper_text, kks);
	    if (n_paper <= 0) continue;

	    ///prepare new record
	    dt.addDays(delay);
	    ConfiguratorAbstractRecord rec;
	    rec.record.insert(ftCompany, QString::number(cid));
	    rec.record.insert(ftTypeOperation, QString::number(opCouponReceive));
	    rec.record.insert(ftCount, QString::number(n_paper));
	    rec.record.insert(ftPaperType, paper_text);
	    rec.record.insert(ftPrice1, QString::number(-1));
	    rec.record.insert(ftPrice, QString::number(-1));
	    rec.record.insert(ftCommission, QString::number(-1));
	    rec.record.insert(ftKKS, kks);
	    rec.record.insert(ftDateOperation, dt.toString(DATE_MASK));
	    double div_sum = n_paper*rec_cal.record.value(ftCouponSize).toDouble();
	    rec.record.insert(ftCouponSize, QString::number(div_sum, 'f', prec));
	    rec.record.insert(ftNalogSize, QString::number(0, 'f', prec));

	    m_divs.records.append(rec);

	//    qDebug()<<QString("finded %1 papers, div_sum=%2").arg(n_paper).arg(div_sum);

	}
    }
}
int DivCalc::paperCountByDate(int cid, const QDate &dt, QString pt, QString kks) const
{
    int n = 0;
    for (int i=0; i<m_operationsData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec = m_operationsData.records.at(i);
        int company = rec.record.value(ftCompany).toInt();
	if (cid != company) continue;

	if (!kks.isEmpty())
	{
	    if (rec.record.value(ftKKS).trimmed() != kks) continue;
	}

        QString paper_type = rec.record.value(ftPaperType).trimmed();
//	qDebug()<<QString("DivCalc::paperCountByDate() i=%1,  cid=%2,  company=%3  type=%4  n=%5").arg(i).arg(cid).arg(company).arg(paper_type).arg(n);
	if (cid != company || paper_type.toLower() != pt.toLower()) continue;
        int op = rec.record.value(ftTypeOperation).toInt();
        QDate dt_operation = QDate::fromString(rec.record.value(ftDateOperation), DATE_MASK);
	if (op != opBuy || dt_operation >= dt) continue;	
	n += rec.record.value(ftCount).toInt();
    }
    return n;
}
void DivCalc::fillCalendarData(ConfiguratorAbstractData &data)
{
    data.reset();

    QString company;
    QString currency;
    QString kks;
    QString paper_text = ConfiguratorEnums::interfaceTextByType(gdCFD).trimmed();
    for (int i=0; i<m_calendarData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec_cal = m_calendarData.records.at(i);
	int cid = rec_cal.record.value(ftCompany).toInt();
        QDate dt = QDate::fromString(rec_cal.record.value(ftDateOperation), DATE_MASK);
	int delay = rec_cal.record.value(ftDivDelay).toInt();
	emit signalGetCompanyByID(cid, company);
	emit signalGetCurrencyByID(cid, currency);
	
	QDate dt2 = dt.addDays(delay);
        ///prepare record
        ConfiguratorAbstractRecord rec;
        rec.record.insert(ftDateCoupon, dt.toString(DATE_MASK));
        rec.record.insert(ftDateOperation, dt2.toString(DATE_MASK));
        rec.record.insert(ftCompany, company);
        rec.record.insert(ftCurrency, currency);
        rec.record.insert(ftKKS, kks);
        rec.record.insert(ftPaperType, paper_text);
        rec.record.insert(ftCouponSize, rec_cal.record.value(ftDivSize));
	insertRecByDate(data, rec);
    }
    /////////////////////////////////////////////////////////
    paper_text = ConfiguratorEnums::interfaceTextByType(gdBond).trimmed();
    for (int i=0; i<m_bondData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec_cal = m_bondData.records.at(i);
	int cid = rec_cal.record.value(ftCompany).toInt();
	int n_pay = rec_cal.record.value(ftCouponCount).toInt()/10;
	kks = rec_cal.record.value(ftKKS).trimmed();
        QDate dt = QDate::fromString(rec_cal.record.value(ftDateCoupon), DATE_MASK);
	int delay = rec_cal.record.value(ftDivDelay).toInt();
	emit signalGetCompanyByID(cid, company);
	emit signalGetCurrencyByID(cid, currency);
	//qDebug()<<QString("DivCalc::updateDivInfo() i=%1  cid=%2  dt=%3  kks=%4  n_pay=%5").arg(i).arg(cid).arg(dt.toString(DATE_MASK)).arg(kks).arg(n_pay);

        QList<QDate> dates = emulCouponsDates(dt, n_pay);
        for (int j=0; j<dates.count(); j++)
        {
            dt = dates.at(j);
	    QDate dt2 = dt.addDays(delay);
    	    ///prepare record
    	    ConfiguratorAbstractRecord rec;
    	    rec.record.insert(ftDateCoupon, dt.toString(DATE_MASK));
    	    rec.record.insert(ftDateOperation, dt2.toString(DATE_MASK));
    	    rec.record.insert(ftCompany, company);
    	    rec.record.insert(ftCurrency, currency);
    	    rec.record.insert(ftKKS, kks);
    	    rec.record.insert(ftPaperType, paper_text);
    	    rec.record.insert(ftCouponSize, rec_cal.record.value(ftCouponSize));
	    insertRecByDate(data, rec);
	}
    }
    ///////////////////////////////////////////
    int n0 = 0;
    for (int i=0; i<data.count(); i++)
    {
        QDate dt = QDate::fromString(data.recAtValue(i, ftDateCoupon), DATE_MASK);
        QDate dt_prev = (i == 0) ? dt: QDate::fromString(data.recAtValue(i-1, ftDateCoupon), DATE_MASK);
	int n = dt_prev.daysTo(dt);
	data.records[i].record.insert(ftNote, QString("%1%2").arg((n>0)?"+":"").arg(n));
	if (n == 0) n0++;
    }
    qDebug()<<QString("DivCalc::fillCalendarData n0=%1").arg(n0);

}
void DivCalc::insertRecByDate(ConfiguratorAbstractData &data, ConfiguratorAbstractRecord &rec)
{
    if (data.isEmpty()) 
    {
	data.append(rec); 
	return;
    }

    QDate dt_rec = QDate::fromString(rec.record.value(ftDateCoupon), DATE_MASK);
    QDate dt;
    for (int i=0; i<data.count(); i++)
    {
        dt = QDate::fromString(data.recAtValue(i, ftDateCoupon), DATE_MASK);
	if (dt < dt_rec) continue;    
	data.records.insert(i, rec);
	return;
    }

    data.records.append(rec);
}
QList<QDate> DivCalc::emulCouponsDates(const QDate &coupon_dt, int n_pay) const
{
    int year1 = lCommonSettings.paramValue("first_div_date").toInt();
    int year2 = lCommonSettings.paramValue("last_div_date").toInt();
    QDate f_dt(year1, 1, 1);
    QDate l_dt(year2, 1, 1);
    QList<QDate> dates;
    QDate first_dt = coupon_dt.addYears(-3);
    
    int add_days = 182;
    if (n_pay == 4) add_days = 91;

    int k = 0;
    QDate cur_dt = first_dt;
    while (cur_dt < l_dt)
    {
	k++;
	cur_dt = cur_dt.addDays(add_days);	    
	if (cur_dt.month() == coupon_dt.month())
	    if (cur_dt.day() != coupon_dt.day()) 
		cur_dt.setDate(cur_dt.year(), cur_dt.month(), coupon_dt.day());

	if (cur_dt < f_dt) continue;
	if (cur_dt > l_dt) break;
	
	dates.append(cur_dt);
    }

    return dates;
}




