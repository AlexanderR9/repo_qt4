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
    timer1->start(1200);
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
void BagWidget::recalcStatistic()
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

	int i_row = 0;
	int n = history_data->count();
	double u_sum, r_sum;

	//calc buy sum
	u_sum = r_sum = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = history_data->recAt(i);
		if (rec.value(ftTypeOperation).toInt() != opBuy) continue;

		QString currency = companyCurrency(rec.value(ftCompany).toInt());
		if (currency == "usd") u_sum += rec.value(ftPrice).toDouble();
		else if (currency == "rur") r_sum += rec.value(ftPrice).toDouble();
		else qWarning()<<QString("BagWidget::recalcStatistic()  WARNING:  invalid currency %1").arg(currency);
	}
	setStatisticRow(i_row, u_sum, r_sum); i_row++;


	//calc sell sum
	u_sum = r_sum = 0;
	for (int i=0; i<n; i++)
	{
		const ConfiguratorAbstractRecord &rec = history_data->recAt(i);
		if (rec.value(ftTypeOperation).toInt() != opSell) continue;

		QString currency = companyCurrency(rec.value(ftCompany).toInt());
		if (currency == "usd") u_sum += rec.value(ftPrice).toDouble();
		else if (currency == "rur") r_sum += rec.value(ftPrice).toDouble();
		else qWarning()<<QString("BagWidget::recalcStatistic()  WARNING:  invalid currency %1").arg(currency);
	}
	setStatisticRow(i_row, u_sum, r_sum); i_row++;

}
void BagWidget::setStatisticRow(int row, const double &usd_value, const double &rur_value)
{
	if (row < 0 || row >= m_statisticTable->rowCount()) return;

	QStringList row_data;
	row_data << QString::number(usd_value, 'f', 1) << QString::number(rur_value, 'f', 1);
	LStatic::setTableRow(row, m_statisticTable, row_data);
}
void BagWidget::slotTimer()
{
    recalcState();
    recalcStatistic();
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
    headers << "Payed of buy operations" << "Getted of sell operations" << "Current bag price" << "Received divs" << "Received coupons";
    headers << "Commission" << "Tax";
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


