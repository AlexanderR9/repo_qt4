 #include "divstatwidget.h"
 #include "lstatic.h"
 #include "cfdenums.h"
 #include "generaldatawidget.h"
 #include "lcommonsettings.h"
 #include "operationdialog.h"
 #include "lchart.h"
 #include "divcalc.h"

 #include <QDebug>
 #include <QDir>
 #include <QDate>
 #include <QSplitter>
 #include <QColor>
 #include <QTimer>
 #include <QTest>

 #define TIMER_INTERVAL1	770


/////////// DivStatWidget /////////////////////////////
DivStatWidget::DivStatWidget(QWidget *parent)
    :LChildWidget(parent),
    v_splitter(NULL),
    h_splitter1(NULL),
    //h_splitter2(NULL),
    m_search(NULL),
    m_search2(NULL),
    m_chart(NULL)
{
    setupUi(this);
    setObjectName("divstat_widget");

    initChart();
    initWidgets();
    initTables();
    initSearchs();

    QTimer *timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(slotTimer()));
    timer1->start(TIMER_INTERVAL1);
}
void DivStatWidget::slotTimer()
{
    QTimer *t =  qobject_cast<QTimer*>(sender());
    if (!t) return;
    t->stop();

	ConfiguratorAbstractData *div_data = NULL;
	emit signalGetDivsData(div_data);
	fillCalendarTable(div_data);
	fillStatTable(div_data);
}
void DivStatWidget::initWidgets()
{
    if (layout()) delete layout();
    QVBoxLayout *lay = new QVBoxLayout(0);
    setLayout(lay);

    v_splitter = new QSplitter(Qt::Vertical, this);
    lay->addWidget(v_splitter);

    h_splitter1 = new QSplitter(Qt::Horizontal, this);
    v_splitter->addWidget(h_splitter1);
    v_splitter->addWidget(chartBox);


    //h_splitter2 = new QSplitter(Qt::Horizontal, this);

    h_splitter1->addWidget(calendarBox);
    h_splitter1->addWidget(statBox);
    //h_splitter1->addWidget(h_splitter2);
    //h_splitter2->addWidget(bagBox);

}
void DivStatWidget::initSearchs()
{
    m_search = new LSearch(searchLineEdit, this);
    m_search->addTable(calendarTable, countLabel);
    m_search2 = new LSearch(searchLineEdit2, this);
    m_search2->addTable(statTable, countLabel2);
    //m_search3 = new LSearch(searchLineEdit3, this);
    //m_search3->addTable(bagTable, countLabel3);
}
void DivStatWidget::slotCalendarChart()
{
    QList<int> sel_cols = LStatic::selectedCols(calendarTable);
    if (sel_cols.isEmpty()) return;

    m_chart->removeChart();
    m_chart->updateAxis();
    int col = sel_cols.first();
    if (col != headerList().indexOf(ftNote)) return;

    LChartParams params(QColor(200, 100, 200), Qt::black);
    m_chart->addChart(params);

    //add points
    QList<QPointF> points;
    QPointF p0(-1, -1);
    for (int j=0; j<calendarTable->rowCount(); j++)
    {
		if (calendarTable->isRowHidden(j)) continue;

		QString s = calendarTable->item(j, col)->text().trimmed();
		QDate dt = QDate::fromString(calendarTable->item(j, 0)->text().trimmed(), DATE_MASK);

		if (points.isEmpty())
		{
			QDate dt0 = dt.addDays(-10);
			p0.setX(QDateTime(dt0).toTime_t());
			points.append(p0);
		}

		QPointF p(-1, -1);
		p.setY(s.toInt());
		p.setX(QDateTime(dt).toTime_t());
		points.append(p);
    }

    m_chart->addChartPoints(points, 0);
    m_chart->updateAxis();
}
void DivStatWidget::calcStatChartPoints(int col, QList<QPointF> &points1, QList<QPointF> &points2)
{
	points1.clear();
	points2.clear();

    for (int j=0; j<statTable->rowCount(); j++)
    {
		if (statTable->isRowHidden(j)) continue;

		QDate dt = QDate::fromString(statTable->item(j, 0)->text().trimmed(), DATE_MASK);
		if (points1.isEmpty())
		{
			QDate dt0 = dt.addDays(-10);
			points1.append(QPointF(QDateTime(dt0).toTime_t(), 0));
		}

		QString s = statTable->item(j, col)->text().trimmed();
		if (s.contains("/"))
		{
			QStringList list(s.split("/"));
			double v1 = list.at(0).toDouble();
			double v2 = list.at(1).toDouble();
			if (points2.isEmpty()) points2.append(points1.first());

			points1.append(QPointF(QDateTime(dt).toTime_t(), v1));
			points2.append(QPointF(QDateTime(dt).toTime_t(), v2));
		}
		else
		{
			points1.append(QPointF(QDateTime(dt).toTime_t(), s.toDouble()));
		}
    }
}
void DivStatWidget::slotStatChart()
{
    QList<int> sel_cols = LStatic::selectedCols(statTable);
    if (sel_cols.isEmpty()) return;
    int col = sel_cols.first();

    m_chart->removeChart();
    m_chart->updateAxis();

    QList<QPointF> points1;
    QList<QPointF> points2;
    if (col == headerList2().indexOf(ftDivSize) || col == headerList2().indexOf(ftNalogSize) || col == headerList2().indexOf(ftResult))
    {
    	calcStatChartPoints(col, points1, points2);
    }

    if (points1.isEmpty()) return;

    LChartParams params1(QColor(200, 100, 200), Qt::darkGreen);
    m_chart->addChart(params1);
    m_chart->addChartPoints(points1, 0);

    if (!points2.isEmpty())
    {
    	LChartParams params2(QColor(200, 100, 200), Qt::blue);
        m_chart->addChart(params2);
        m_chart->addChartPoints(points2, 0);
    }

    m_chart->updateAxis();
}
/*
void DivStatWidget::slotBagChart()
{
    repaintStatChart(bagTable);
}
*/
void DivStatWidget::updateColorsCalendar(const ConfiguratorAbstractData *data)
{
    if (!lCommonSettings.paramValue("table_colors").toBool()) return;
    if (data->isEmpty()) return;

    QStringList list;
    QList<QColor> colors;
    QDate cur_dt(QDate::currentDate());
    QDate dt(QDate::fromString(data->recAtValue(0, ftDateCoupon), DATE_MASK));
    int month = dt.month();
    
    QColor c1 = QColor(250, 250, 230);	
    QColor c2 = QColor(200, 220, 220);	
    QColor c3 = QColor(100, 250, 100);
    QColor cc = c1;	
    int last_c = 1;
    int cur_month_divs = 0;
    for (int i=0; i<data->count(); i++)
    {
		dt = QDate::fromString(data->recAtValue(i,ftDateCoupon), DATE_MASK);
		if (dt.month() != month)
		{
			month = dt.month();
			if (last_c == 1) {last_c = 2; cc = c2;}
			else {last_c = 1; cc = c1;}
		}
		if (dt.year() == cur_dt.year() && dt.month() == cur_dt.month())
		{
			cc = c3;
			cur_month_divs++;
		}
        LStatic::setTableRowColor(calendarTable, i, cc);
    }
    
    countLabel->setText(QString("%1,  current month count %2").arg(countLabel->text()).arg(cur_month_divs));
}
void DivStatWidget::initChart()
{
    m_chart = new LChartWidget(this);
    if (chartBox->layout()) delete chartBox->layout();
    chartBox->setLayout(new QHBoxLayout(0));
    chartBox->layout()->addWidget(m_chart);

    m_chart->removeChart();
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    m_chart->setAxisPrecision(0, 1);
    m_chart->setAxisMarksInterval(500, 100);
    m_chart->setCrossXAxisTextViewMode(2);
    m_chart->setPointSize(3);
    m_chart->updateAxis();

}
QList<int> DivStatWidget::headerList() const
{
    QList<int> list;
    list.append(ftDateCoupon);
    //list.append(ftDateOperation);
    list.append(ftNote);
    list.append(ftCompany);
    list.append(ftCurrency);
    list.append(ftKKS);
    //list.append(ftPaperType);
    list.append(ftCouponSize);
    list.append(ftCount);
    return list;
}
QList<int> DivStatWidget::headerList2() const
{
    QList<int> list;
    list.append(ftDateCoupon);
    list.append(ftCompany);
    //list.append(ftCurrency);
    list.append(ftPaperType);
    list.append(ftDivSize);
    list.append(ftNalogSize);
    list.append(ftResult);
    return list;
}
/*
QList<int> DivStatWidget::headerList3() const
{
    QList<int> list;
    list.append(ftDateOperation);
    list.append(ftCompany);
    list.append(ftCurrency);
    list.append(ftPaperType);
    list.append(ftPrice);
    list.append(ftCommission);
    return list;
}
*/
void DivStatWidget::initTables()
{
    LStatic::fullClearTable(calendarTable);
    LStatic::fullClearTable(statTable);
    //LStatic::fullClearTable(bagTable);

    QList<int> fields = headerList();
    QStringList headers;
    for (int i=0; i<fields.count(); i++)
    	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));
    LStatic::setTableHeaders(calendarTable, headers);
    
    fields.clear();
    headers.clear();
    fields = headerList2();
    for (int i=0; i<fields.count(); i++)
    {
    	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));
    	if (i > 2) headers[i].append(", rub.");
    }

    LStatic::setTableHeaders(statTable, headers);

    /*
    fields.clear();
    headers.clear();
    fields = headerList3();
    for (int i=0; i<fields.count(); i++)
    	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));
    LStatic::setTableHeaders(bagTable, headers);
    */

    calendarTable->verticalHeader()->hide();
    statTable->verticalHeader()->hide();
    //bagTable->verticalHeader()->hide();

    connect(calendarTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotCalendarChart()));
    connect(statTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotStatChart()));
    //connect(bagTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotBagChart()));
}
void DivStatWidget::fillCalendarTable(const ConfiguratorAbstractData *data)
{
    LStatic::removeAllRowsTable(calendarTable);

    for (int i=0; i<data->count(); i++)
    	LStatic::addTableRow(calendarTable, recToRow(data->records.at(i), headerList()));

    updateDaysTo();
    updateColorsCalendar(data);
    calendarTable->resizeRowsToContents();
}
void DivStatWidget::updateDaysTo()
{
	int col = headerList().indexOf(ftNote);
	if (col < 0) return;

	QList<quint8> list;
	emit signalGetWaitDays(list);

	for (int i=0; i<list.count(); i++)
		calendarTable->item(i, col)->setText(QString::number(list.at(i)));
}
/*
void DivStatWidget::fillBagTable(ConfiguratorAbstractData *data)
{
    qDebug("DivStatWidget::fillBagTable");
    double price = 0;
    double commission = 0;
    double kurs = lCommonSettings.paramValue("kurs").toDouble();

    QList<QDate> dates;
    QString company, currency;
    QStringList row_data;
    LStatic::removeAllRowsTable(bagTable);
    QString p_type;
    for (int i=0; i<data->count(); i++)
    {
		row_data.clear();
		const ConfiguratorAbstractRecord &rec = data->records.at(i);
		if (rec.record.value(ftTypeOperation).toInt() != opBuy) continue;

		int cid = rec.record.value(ftCompany).toInt();
		p_type = rec.record.value(ftPaperType).trimmed().toLower();
		row_data << rec.record.value(ftDateOperation);
		QDate dt = QDate::fromString(row_data.last(), DATE_MASK);

		emit signalGetCompanyByID(cid, company);
		emit signalGetCurrencyByID(cid, currency);
		currency = currency.trimmed().toLower();
		row_data << company << currency << p_type;

		price = rec.record.value(ftPrice).toDouble();
		commission = rec.record.value(ftCommission).toDouble();
		if (currency == "usd") {price *= kurs; commission *= kurs;}
		row_data << QString("%1").arg(QString::number(price, 'f', 1));
		row_data << QString("%1").arg(QString::number(commission, 'f', 2));

		int row_index = 0;
		if (!dates.isEmpty())
		{
			int nd = dates.count();
			for (int j=(nd-1); j>=0; j--)
			if (dt > dates.at(j)) {row_index = j+1; break;}
		}
        dates.insert(row_index, dt);
        LStatic::insertTableRow(row_index, bagTable, row_data);
    }


    price = 0;
    commission = 0;
    for (int i=0; i<bagTable->rowCount(); i++)
    {
		QTableWidgetItem *price_item = bagTable->item(i, 4);
		QTableWidgetItem *commission_item = bagTable->item(i, 5);
	
		price += price_item->text().toDouble();
		commission += commission_item->text().toDouble();

		price_item->setText(QString("%1/%2").arg(price_item->text()).arg(QString::number(price, 'f', 0)));
		commission_item->setText(QString("%1/%2").arg(commission_item->text()).arg(QString::number(commission, 'f', 0)));
    }
    bagTable->resizeRowsToContents();
}
*/
void DivStatWidget::fillStatTable(ConfiguratorAbstractData *data)
{
    if (!data) {qWarning("DivStatWidget::fillStatTable ERR: data is null!"); return;}

    int company_col = headerList().indexOf(ftCompany);
    int currency_col = headerList().indexOf(ftCurrency);

    double nalog_sum = 0;
    double divs_sum = 0;
    double kurs = lCommonSettings.paramValue("kurs").toDouble();
    LStatic::removeAllRowsTable(statTable);
    QDate cur_dt = QDate::currentDate();

    QStringList row_data;
    for (int i=0; i<data->count(); i++)
    {
    	const ConfiguratorAbstractRecord &rec = data->recAt(i);
    	QDate dt_div = QDate::fromString(rec.value(ftDateCoupon), DATE_MASK);
    	if (dt_div > cur_dt) break;

    	int delay = rec.value(ftDivDelay).toInt();
    	if (dt_div.addDays(delay) > cur_dt) continue;

    	bool is_bond = !rec.value(ftKKS).isEmpty();
    	double f_nalog = rec.value(ftNalogSize).toDouble();
    	//if (i < 50) qDebug()<<QString("f_nalog=%1").arg(f_nalog);
    	double div_size = rec.value(ftDivSize).toDouble();
    	if (calendarTable->item(i, currency_col)->text().trimmed().toLower() == "usd") div_size *= kurs;
    	double p_count = rec.value(ftCount).toInt();

    	//current row calc
    	double x = p_count*div_size;
    	divs_sum += x;
    	nalog_sum += (x*f_nalog);


    	row_data.clear();
    	row_data.append(rec.value(ftDateCoupon));
    	row_data.append(calendarTable->item(i, company_col)->text());
    	row_data.append(is_bond ? "bond" : "cfd");
    	row_data.append(QString("%1/%2").arg(QString::number(x, 'f', 1)).arg(QString::number(divs_sum, 'f', 0)));
    	row_data.append(QString("%1/%2").arg(QString::number(x*f_nalog, 'f', 1)).arg(QString::number(nalog_sum, 'f', 0)));
    	row_data.append(QString("%1").arg(QString::number(divs_sum - nalog_sum, 'f', 1)));
    	LStatic::addTableRow(statTable, row_data);
    }

    updateColorsStat();
    statTable->resizeRowsToContents();
}
void DivStatWidget::updateColorsStat()
{
	int date_col = headerList2().indexOf(ftDateCoupon);
	int cur_month = -1;
	QColor line_color = Qt::lightGray;

	int rows = statTable->rowCount();
	for (int i=0; i<rows; i++)
	{
		QDate dt = QDate::fromString(statTable->item(i, date_col)->text(), DATE_MASK);
		if (cur_month != dt.month())
		{
			if (cur_month > 0) LStatic::setTableRowColor(statTable, i, line_color);
			cur_month = dt.month();
		}
	}
}

QStringList DivStatWidget::recToRow(const ConfiguratorAbstractRecord &rec, const QList<int> &fields) const
{
    QStringList list;
    for (int i=0; i<fields.count(); i++)
		list.append(recField(rec, fields.at(i)));
    return list;
}
QString DivStatWidget::recField(const ConfiguratorAbstractRecord &rec, int field) const
{
	QString s("???");
	switch (field)
	{
		case ftCompany:
		{
			emit signalGetCompanyByID(rec.record.value(ftCompany).toInt(), s);
			break;
		}
		case ftCurrency:
		{
			emit signalGetCurrencyByID(rec.record.value(ftCompany).toInt(), s);
			break;
		}
		case ftCouponSize:
		case ftDivSize:
		{
			s = rec.record.value(ftDivSize);
			break;
		}
		default:
		{
			s = rec.record.value(field, QString("???"));
			break;
		}
	}
	return s;
}
void DivStatWidget::save(QSettings &settings)
{
    settings.setValue(QString("%1/v_splitter").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/h_splitter1").arg(objectName()), h_splitter1->saveState());
    //settings.setValue(QString("%1/h_splitter2").arg(objectName()), h_splitter2->saveState());
    saveTableState(settings, calendarTable);
    saveTableState(settings, statTable);
    //saveTableState(settings, bagTable);
}
void DivStatWidget::load(QSettings &settings)
{
    QByteArray ba = settings.value(QString("%1/v_splitter").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) v_splitter->restoreState(ba);
    ba.clear();

    ba = settings.value(QString("%1/h_splitter1").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) h_splitter1->restoreState(ba);
    //ba = settings.value(QString("%1/h_splitter2").arg(objectName()), QByteArray()).toByteArray();
    //if (!ba.isEmpty()) h_splitter2->restoreState(ba);

    loadTableState(settings, calendarTable);
    loadTableState(settings, statTable);
    //loadTableState(settings, bagTable);
}
void DivStatWidget::saveTableState(QSettings &settings, const QTableWidget *tw)
{
    if (!tw) return;
    QString text = QString("%1/table_state/%2").arg(objectName()).arg(tw->objectName());
    settings.setValue(text, tw->horizontalHeader()->saveState());
}
void DivStatWidget::loadTableState(QSettings &settings, QTableWidget *tw)
{
    if (!tw) return;
    QString text = QString("%1/table_state/%2").arg(objectName()).arg(tw->objectName());
    QByteArray ba = settings.value(text, QByteArray()).toByteArray();
    if (!ba.isEmpty()) tw->horizontalHeader()->restoreState(ba);
    tw->resizeRowsToContents();
}









