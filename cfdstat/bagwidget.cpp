 #include "bagwidget.h"
 #include "lstatic.h"
 #include "cfdenums.h"
#include "generaldatawidget.h"
#include "lcommonsettings.h"

#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QSplitter>
#include <QCheckBox>
#include <QTableWidget>


#define COMPANY_COL				0
#define TABLE_ROW_HEIGHT		26
#define TABLE_ICON_SIZE			24
#define SOLD_ROW_COLOR			QColor(230, 225, 225)
#define DIVS_ROW_COLOR			QColor(200, 250, 200)
#define ERR_VALUE				-9999

#define CALC_STAT_TABLE_TIMEOUT		1855

/////////// BagWidget /////////////////////////////
BagWidget::BagWidget(QWidget *parent)
	:LChildWidget(parent),
	 m_search(NULL),
	 m_splitter(NULL),
	 m_statisticTable(NULL),
	 m_hideSellingCheckBox(NULL)
{
    setupUi(this);
    setObjectName("bag_page");

    initBoxes();
    initBagTable();
    readGeneralData();
    initStatisticTable();

    searchLineEdit->clear();
    m_search = new LSearch(searchLineEdit, this);
    m_search->addTable(tableWidget, countLabel);
    m_search->exec();
    connect(m_search, SIGNAL(signalSearched()), this, SLOT(slotWasSearch()));

    QTimer *timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(slotTimer()));
    timer1->start(CALC_STAT_TABLE_TIMEOUT);
}
void BagWidget::initBoxes()
{
    listWidget->clear();
    listWidget->hide();
    tableListBox->layout()->removeWidget(listWidget);
    tableListBox->setTitle("Statistic");
    tableListBox->setMaximumWidth(500);
    tableBox->setMinimumWidth(500);

    m_hideSellingCheckBox = new QCheckBox("Hide sold papers");
    tableBox->layout()->addWidget(m_hideSellingCheckBox);
    connect(m_hideSellingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotHideSoldChanged()));

	if (layout()) delete layout();
	QHBoxLayout *lay = new QHBoxLayout(0);
	setLayout(lay);

	m_splitter = new QSplitter(Qt::Horizontal, this);
	lay->addWidget(m_splitter);
	m_splitter->addWidget(tableBox);
	m_splitter->addWidget(tableListBox);

}
void BagWidget::recalcStatisticTableData()
{
	ConfiguratorAbstractData *history_data = NULL;
	emit signalGetOperationsHistory(history_data); //запрос истории совершенных операций

	ConfiguratorAbstractData *div_data = NULL;
	emit signalGetDivsData(div_data); //запрос истории выплат дивов и купонов у DivCalc

	if (!history_data || !div_data)
	{
		qWarning()<<QString("BagWidget::recalcStatistic() WARNING: !history_data || !div_data");
		return;
	}

	statistic_calcBuySum(history_data);
	statistic_calcSellSum(history_data);
	statistic_calcPaperCount(history_data);
	statistic_calcCommissionSum(history_data);
	statistic_calcBagPrice();

	statistic_calcReceivedDivs(div_data);
	statistic_calcTaxSum(div_data);
}
void BagWidget::statistic_calcBuySum(const ConfiguratorAbstractData *data)
{
	if (!data) return;

	int n = data->count();
	double u_sum = 0;
	double r_sum = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = data->recAt(i);
		if (rec.value(ftTypeOperation).toInt() != opBuy) continue;

		QString currency = companyCurrency(rec.value(ftCompany).toInt());
		if (currency == "usd") u_sum += rec.value(ftPrice).toDouble();
		else if (currency == "rur") r_sum += rec.value(ftPrice).toDouble();
		else qWarning()<<QString("BagWidget::statistic_calcBuySum()  WARNING:  invalid currency %1").arg(currency);
	}
	setStatisticRow(0, u_sum, r_sum);
}
void BagWidget::statistic_calcSellSum(const ConfiguratorAbstractData *data)
{
	if (!data) return;

	int n = data->count();
	double u_sum = 0;
	double r_sum = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = data->recAt(i);
		if (rec.value(ftTypeOperation).toInt() != opSell) continue;

		QString currency = companyCurrency(rec.value(ftCompany).toInt());
		if (currency == "usd") u_sum += rec.value(ftPrice).toDouble();
		else if (currency == "rur") r_sum += rec.value(ftPrice).toDouble();
		else qWarning()<<QString("BagWidget::statistic_calcSellSum()  WARNING:  invalid currency %1").arg(currency);
	}
	setStatisticRow(1, u_sum, r_sum);
}
void BagWidget::statistic_calcCommissionSum(const ConfiguratorAbstractData *data)
{
	if (!data) return;

	int n = data->count();
	double u_sum = 0;
	double r_sum = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = data->recAt(i);
		int type = rec.value(ftTypeOperation).toInt();
		if (type == opSell || type == opBuy)
		{
			QString currency = companyCurrency(rec.value(ftCompany).toInt());
			double commis = rec.value(ftCommission).toDouble();
			if (currency == "usd") u_sum += commis;
			else if (currency == "rur") r_sum += commis;
			else qWarning()<<QString("BagWidget::statistic_calcCommissionSum()  WARNING:  invalid currency %1").arg(currency);
		}
	}
	setStatisticRow(5, (-1)*u_sum, (-1)*r_sum);
}
void BagWidget::statistic_calcTaxSum(const ConfiguratorAbstractData *div_data)
{
	if (!div_data) return;

	int n = div_data->count();
	double u_sum = 0;
	double r_sum = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = div_data->recAt(i);
		double div_size = rec.value(ftDivSize).toDouble(); // 1 ps
		int n_papers = rec.value(ftCount).toInt();
		double f_tax = rec.value(ftNalogSize).toDouble(); //factor by divsize for calc tax
		QString currency = companyCurrency(rec.value(ftCompany).toInt());
		double tax = div_size*n_papers*f_tax;

		if (rec.value(ftCompany).toInt() == 1) qDebug()<<rec.toString();

		if (currency == "usd") u_sum += tax;
		else if (currency == "rur") r_sum += tax;
		else qWarning()<<QString("BagWidget::statistic_calcTaxSum()  WARNING:  invalid currency %1").arg(currency);
	}
	setStatisticRow(6, (-1)*u_sum, (-1)*r_sum);
}
void BagWidget::statistic_calcReceivedDivs(const ConfiguratorAbstractData *div_data)
{
	if (!div_data) return;

	double b_coupons = 0;
	double rs_divs = 0;
	double us_div = 0;

	int n = div_data->count();
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = div_data->recAt(i);
		double div_size = rec.value(ftDivSize).toDouble() * rec.value(ftCount).toInt(); //for n ps
		double f_tax = rec.value(ftNalogSize).toDouble(); //factor by divsize for calc tax
		if (f_tax > 0) div_size *= (1-f_tax);
		QString currency = companyCurrency(rec.value(ftCompany).toInt());

		int op_type = rec.value(ftTypeOperation).toInt();
		switch (op_type)
		{
			case opCouponReceive:
			{
				b_coupons += div_size;
				break;
			}
			case opDivReceive:
			{
				if (currency == "usd") us_div += div_size;
				else if (currency == "rur") rs_divs += div_size;
				else qWarning()<<QString("BagWidget::statistic_calcReceivedDivs()  WARNING:  invalid currency %1").arg(currency);
				break;
			}
			default: break;
		}
	}

	setStatisticRow(3, ERR_VALUE, b_coupons);
	setStatisticRow(4, us_div, rs_divs);
	LStatic::setTableRowColor(m_statisticTable, 3, DIVS_ROW_COLOR);
	LStatic::setTableRowColor(m_statisticTable, 4, DIVS_ROW_COLOR);

}
void BagWidget::statistic_calcPaperCount(const ConfiguratorAbstractData *data)
{
	if (!data) return;

	int n = data->count();
	uint b_count = 0;
	uint rs_count = 0;
	uint us_count = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = data->recAt(i);
		QString currency = companyCurrency(rec.value(ftCompany).toInt());
		bool is_cfd = rec.value(ftPaperType).trimmed().toLower().contains("cfd");
		uint p_count = rec.value(ftCount).toUInt();
		int type = rec.value(ftTypeOperation).toInt();
		switch (type)
		{
			case opBuy:
			{
				if (is_cfd)
				{
					if (currency == "usd") us_count += p_count;
					else if (currency == "rur") rs_count += p_count;
					else qWarning()<<QString("BagWidget::statistic_calcPaperCount()  WARNING:  invalid currency %1").arg(currency);
				}
				else b_count += p_count;
				break;
			}
			case opSell:
			{
				if (is_cfd)
				{
					if (currency == "usd") us_count -= p_count;
					else if (currency == "rur") rs_count -= p_count;
					else qWarning()<<QString("BagWidget::statistic_calcPaperCount()  WARNING:  invalid currency %1").arg(currency);
				}
				else b_count -= p_count;
				break;
			}
			default: break;
		}
	}
	setStatisticRow(7, ERR_VALUE, b_count);
	setStatisticRow(8, us_count, rs_count);
}
void BagWidget::statistic_calcBagPrice()
{
	int n_rows = tableWidget->rowCount();
	double r_price = 0;
	double u_price = 0;
    for (int i=0; i<n_rows; i++)
    {
		QString currency = tableWidget->item(i, 2)->text().trimmed().toLower();
		double line_price = tableWidget->item(i, 6)->text().trimmed().toDouble();
		if (currency == "usd") u_price += line_price;
		else if (currency == "rub") r_price += line_price;
		else qWarning()<<QString("BagWidget::statistic_calcBagPrice()  WARNING:  invalid currency %1").arg(currency);
    }
	setStatisticRow(2, u_price/1000, r_price/1000, 2);
}
void BagWidget::setStatisticRow(int row, const double &usd_value, const double &rur_value, quint8 precision)
{
	if (row < 0 || row >= m_statisticTable->rowCount()) return;

	QStringList row_data;
	row_data << ((usd_value == ERR_VALUE) ? QString("---") : QString::number(usd_value, 'f', precision));
	row_data << ((rur_value == ERR_VALUE) ? QString("---") : QString::number(rur_value, 'f', precision));
	LStatic::setTableRow(row, m_statisticTable, row_data);
}
void BagWidget::slotTimer()
{
    recalcState();
    recalcStatisticTableData();
    m_search->exec();
    qobject_cast<QTimer*>(sender())->stop();
}
void BagWidget::recalcState()
{
    for (int i=0; i<tableWidget->rowCount(); i++)
    {
		QString v = tableWidget->item(i, 2)->text().trimmed().toLower();
		QString type = tableWidget->item(i, 3)->text().trimmed().toLower();
		int count = tableWidget->item(i, 4)->text().toInt();
		double p = tableWidget->item(i, 5)->text().toDouble();
		double cs = tableWidget->item(i, 6)->text().toDouble();
		double d = tableWidget->item(i, 8)->text().toDouble();

		if (v == "usd")
		{
			u_state.payed += p;
			u_state.cur_size += cs;
			u_state.div_size += d;
			u_state.p_count += count;
		}
		else
		{
			if (type =="cfd")
			{
				r_state.payed += p;
				r_state.cur_size += cs;
				r_state.div_size += d;
				r_state.p_count += count;
			}
			else
			{
				b_state.payed += p;
				b_state.cur_size += cs;
				b_state.div_size += d;
				b_state.p_count += count;
			}
		}
    }

    double kurs = lCommonSettings.paramValue("kurs").toDouble();
    double sum = r_state.cur_size + u_state.cur_size*kurs + b_state.cur_size;

    tableBox->setTitle(QString("State: usd(%1)  rub(%2)  bonds(%3), current price of bag %4  ").arg(u_state.toStr()).
			    arg(r_state.toStr()).arg(b_state.toStr()).arg(QString::number(sum, 'f', 1)));

}
void BagWidget::recalcState(const ConfiguratorAbstractRecord &rec)
{
    if (rec.record.value(ftCurrency).trimmed().toLower() == "usd")
    {
		u_state.payed += rec.record.value(ftPayedSize).toDouble();
		u_state.cur_size += rec.record.value(ftPrice).toDouble();
		u_state.div_size += rec.record.value(ftCouponSize).toDouble();
    }
    else
    {
		r_state.payed += rec.record.value(ftPayedSize).toDouble();
		r_state.cur_size += rec.record.value(ftPrice).toDouble();
		r_state.div_size += rec.record.value(ftCouponSize).toDouble();
    }
    tableBox->setTitle(QString("State: usd(%1)  rub(%2)").arg(u_state.toStr()).arg(r_state.toStr()));
}
QList<int> BagWidget::headerList() const
{
    QList<int> list;
    list << ftCompany;
    list << ftKKS;
    list << ftCurrency;
    list << ftPaperType;
    list << ftCount;
    list << ftPayedSize; //стоимость, которая была заплачена за все бумаги, включая коммисию
    list << ftPrice; //текущая стоимость всех бумаг
    list << ftDifference; //текущая разница
    list << ftCouponSize; //все полученные дивиденды/купоны по этому инструменту
    //list << ftCountry; 
    return list;
}
void BagWidget::refresh()
{
    updateColors();
    LStatic::resizeTableContents(tableWidget);
    m_search->exec();
    updateIcons();
}
void BagWidget::updateIcons()
{
    int row_count = tableWidget->rowCount();
    for (int i=0; i<row_count; i++)
    {
		tableWidget->setRowHeight(i, TABLE_ROW_HEIGHT);
		QTableWidgetItem *company_item = tableWidget->item(i, COMPANY_COL);
		if (!company_item) continue;

		QString company_name = company_item->text();
		int pos = company_name.indexOf("(");
		if (pos > 0) company_name = company_name.left(pos).trimmed();

			QString flag_name = companyIcon(company_name);
		if (flag_name.length() > 5)
		{
			QIcon icon(QString(":/icons/images/flag/%1").arg(flag_name));
			company_item->setIcon(icon);
		}
    }
}
void BagWidget::updateColors()
{
    if (!lCommonSettings.paramValue("table_colors").toBool()) return;

    int col = headerList().indexOf(ftDifference);
    int col_count = headerList().indexOf(ftCount);
    if (col < 0 || col_count < 0) return;

    for (int i=0; i<tableWidget->rowCount(); i++)
    {
		double d = tableWidget->item(i, col)->text().toDouble();
		if (d < 0) tableWidget->item(i, col)->setTextColor(Qt::red);
		else if (d > 0) tableWidget->item(i, col)->setTextColor(Qt::blue);

		//check count
		int n = tableWidget->item(i, col_count)->text().toInt();
		if (n < 0)
		{
			 LStatic::setTableRowColor(tableWidget, i, Qt::red);
		}
		else if (n == 0)
		{
			 LStatic::setTableRowColor(tableWidget, i, SOLD_ROW_COLOR);
		}
    }
}
void BagWidget::initBagTable()
{
    LStatic::fullClearTable(tableWidget);

    QList<int> fields(headerList());
    QStringList headers;
    for (int i=0; i<fields.count(); i++)
    {
		if (fields.at(i) == ftPrice) headers.append("Current price");
		else headers.append(ConfiguratorEnums::interfaceTextByType(fields.at(i)));
    }

    LStatic::setTableHeaders(tableWidget, headers);
    tableWidget->verticalHeader()->hide();
    tableWidget->setIconSize(QSize(TABLE_ICON_SIZE, TABLE_ICON_SIZE));
}
void BagWidget::initStatisticTable()
{
	m_statisticTable = new QTableWidget(this);
	m_statisticTable->setSelectionMode(QAbstractItemView::NoSelection);
	tableListBox->layout()->addWidget(m_statisticTable);

    LStatic::fullClearTable(m_statisticTable);
    QStringList headers;
    headers << "usd" << "rur";
    LStatic::setTableHeaders(m_statisticTable, headers, Qt::Horizontal);

    headers.clear();
    headers << "Payed of buy operations" << "Getted of sell operations" << "Current bag price (x1000)";
    headers << "Received coupons (clean)" << "Received divs (clean)";
    headers << "Commission" << "Tax" << "Bond count" << "Stock count";
    LStatic::setTableHeaders(m_statisticTable, headers, Qt::Vertical);

}
void BagWidget::buy(const ConfiguratorAbstractRecord &rec)
{
    emit signalNextOperation(opBuy, rec);
}
void BagWidget::sell(const ConfiguratorAbstractRecord &rec)
{
    qDebug("BagWidget::sell");
    qDebug()<<rec.toString();
    emit signalNextOperation(opSell, rec);
}
void BagWidget::slotBagUpdate(const ConfiguratorAbstractRecord &rec)
{
	//qDebug()<<QString("BagWidget::slotBagUpdate - %1").arg(rec.toString());
    QString kks = rec.record.value(ftKKS);
    int id = rec.record.value(ftID).toInt();
    int pos = findRec(id, kks);
    if (pos < 0) //в таблице еще нет строки с такой бумагой
    {
		m_data.records.append(rec);
		QStringList list = recToRow(m_data.records.last());
		LStatic::addTableRow(tableWidget, list);
    }
    else //в таблице уже есть строка с такой бумагой
    {
		m_data.records[pos].copy(rec);
		QStringList list = recToRow(m_data.recAt(pos));
		LStatic::setTableRow(pos, tableWidget, list);
    }

    if (rec.record.value(ftCount).toInt() < 0)
	QMessageBox::critical(this, tr("Critical error"), tr("Paper count < 0 for company - %1").arg(rec.record.value(ftCompany)));

}
QStringList BagWidget::recToRow(const ConfiguratorAbstractRecord &rec) const
{
    QStringList list;
    QList<int> keys = headerList();
    for (int i=0; i<keys.count(); i++)
    {
		QString s = rec.record.value(keys.at(i), QString("???"));
		list.append(s);
    }
    return list;
}
QString BagWidget::companyIcon(const QString &company_name) const
{
    const ConfiguratorAbstractRecord *company_rec = m_companyData.recByFieldValue(ftName, company_name);
    int id_country = -1;
    if (company_rec) id_country = company_rec->value(ftCountry, QString("-2")).toInt();
    if (id_country < 0) 
    {
    	qWarning()<<QString("WARNING: id_country=%1   company_name=%2").arg(id_country).arg(company_name);
    }
    else
    {
		const ConfiguratorAbstractRecord *country_rec = m_countryData.recByFieldValue(ftID, QString::number(id_country));
		if (country_rec) return country_rec->value(ftImage);
    }
    return QString();
}
QString BagWidget::companyCurrency(int c_id) const
{
	QString currency = "??";
	for (int i=0; i<m_companyData.count(); i++)
	{
		if (m_companyData.recAt(i).value(ftID, QString("-1")).toInt() == c_id)
		{
			int v = m_companyData.recAt(i).value(ftCurrency, QString("-1")).toInt();
			if (v == 2) currency = "usd";
			else if (v == 1) currency = "rur";
			else currency = QString::number(v);
			break;
		}
	}
	return currency;
}
int BagWidget::findRec(int id, const QString &kks) const
{
    for (int i=0; i<m_data.count(); i++)
    {
        const ConfiguratorAbstractRecord &rec = m_data.records.at(i);
        if ((rec.record.value(ftID).toInt()) == id && rec.record.value(ftKKS).trimmed() == kks)
	    return i;
    }
    return -1;
}
void BagWidget::readGeneralData()
{
    m_countryData.generalType = gdCountry;
    m_companyData.generalType = gdCompany;
    GeneralDataFileReader::loadDataFormFile(gdCountry, m_countryData);
    GeneralDataFileReader::loadDataFormFile(gdCompany, m_companyData);

    if (m_countryData.invalid() || m_companyData.invalid())
    {
        m_err = QObject::tr("Error loading general data, check application settings.");
        showErr();
        return;
    }
    qDebug()<<QString("BagWidget::readGeneralData() - Data loaded ok!  country count %1,  company count %3").arg(m_countryData.count()).arg(m_companyData.count());
}
void BagWidget::slotBagRefreshTable()
{
	refresh();
}
void BagWidget::save(QSettings &settings)
{
	LChildWidget::save(settings);
	settings.setValue(QString("%1/splitter_state").arg(objectName()), m_splitter->saveState());
}
void BagWidget::load(QSettings &settings)
{
	LChildWidget::load(settings);

	QByteArray ba = settings.value(QString("%1/splitter_state").arg(objectName()), QByteArray()).toByteArray();
	if (!ba.isEmpty()) m_splitter->restoreState(ba);

}
void BagWidget::slotHideSoldChanged()
{
	bool is_hide = m_hideSellingCheckBox->isChecked();
	//m_search->exec();
	int n_hiden = 0;
	int n_rows = tableWidget->rowCount();
    for (int i=0; i<n_rows; i++)
	{
	  	if (tableWidget->item(i, 1)->background().color() == SOLD_ROW_COLOR)
	   	{
	   		if (is_hide) tableWidget->hideRow(i);
	   		else tableWidget->showRow(i);
	   	}

	  	if (tableWidget->isRowHidden(i)) n_hiden++;
	}

    if (!is_hide) m_search->exec();
    else countLabel->setText(QString("Record number: %1/%2").arg(n_rows-n_hiden).arg(n_rows));
}
void BagWidget::slotWasSearch()
{
	bool is_hide = m_hideSellingCheckBox->isChecked();
	if (!is_hide) return;

	int n_hiden = 0;
	int n_rows = tableWidget->rowCount();
    for (int i=0; i<n_rows; i++)
    {
	  	if (tableWidget->item(i, 1)->background().color() == SOLD_ROW_COLOR) tableWidget->hideRow(i);
	  	if (tableWidget->isRowHidden(i)) n_hiden++;
    }

	 countLabel->setText(QString("Record number: %1/%2").arg(n_rows-n_hiden).arg(n_rows));
}


