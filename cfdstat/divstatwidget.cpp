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

 #define TIMER_INTERVAL1	900
 #define TIMER_INTERVAL2	650
 #define TIMER_INTERVAL3	350


/////////// DivStatWidget /////////////////////////////
DivStatWidget::DivStatWidget(QWidget *parent)
    :LChildWidget(parent),
    v_splitter(NULL),
    h_splitter1(NULL),
    h_splitter2(NULL),
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
    
    if (t->interval() == TIMER_INTERVAL1)
    {
	initCalc();
	//m_search->exec();
	t->setInterval(TIMER_INTERVAL2);
	t->start();
    }
    else if (t->interval() == TIMER_INTERVAL2)
    {
	ConfiguratorAbstractData *div_data = NULL;
	emit signalGetDivData(div_data);
	fillStatTable(div_data);    
	//m_search2->exec();    

	t->setInterval(TIMER_INTERVAL3);
	t->start();
    }
    else if (t->interval() == TIMER_INTERVAL3)
    {
	ConfiguratorAbstractData *history_data = NULL;
	emit signalGetHistoryData(history_data);
	fillBagTable(history_data);    
	//m_search3->exec();    
	updateColors();
    }
}
void DivStatWidget::initWidgets()
{
    if (layout()) delete layout();
    QVBoxLayout *lay = new QVBoxLayout(0);
    setLayout(lay);

    v_splitter = new QSplitter(Qt::Vertical, this);
    lay->addWidget(v_splitter);

    h_splitter1 = new QSplitter(Qt::Horizontal, this);
    h_splitter2 = new QSplitter(Qt::Horizontal, this);

    h_splitter1->addWidget(calendarBox);
    h_splitter1->addWidget(h_splitter2);
    h_splitter2->addWidget(statBox);
    h_splitter2->addWidget(bagBox);

    v_splitter->addWidget(h_splitter1);
    v_splitter->addWidget(chartBox);
}
void DivStatWidget::initSearchs()
{
    m_search = new LSearch(searchLineEdit, this);
    m_search->addTable(calendarTable, countLabel);
//    m_search->exec();

    m_search2 = new LSearch(searchLineEdit2, this);
    m_search2->addTable(statTable, countLabel2);
//    m_search2->exec();

    m_search3 = new LSearch(searchLineEdit3, this);
    m_search3->addTable(bagTable, countLabel3);
//    m_search3->exec();
}
void DivStatWidget::initCalc()
{
    m_calc = new DivCalc(m_data, this);
    m_calc->readGeneralData(m_err);
    if (isErr()) {showErr(); return;}

    connect(m_calc, SIGNAL(signalGetCompanyByID(int, QString&)), this, SIGNAL(signalGetCompanyByID(int, QString&)));
    connect(m_calc, SIGNAL(signalGetCurrencyByID(int, QString&)), this, SIGNAL(signalGetCurrencyByID(int, QString&)));

    m_calc->fillCalendarData(m_data);
    fillCalendarTable();
    updateColors();
}
void DivStatWidget::slotCalendarChart()
{
    QList<int> sel_cols = LStatic::selectedCols(calendarTable);
    if (sel_cols.isEmpty()) return;
    int col = sel_cols.first();    
    m_chart->removeChart();
    m_chart->setPointSize(4);
    //m_chart->setOnlyPoints(true);
    if (col != 2 && col != 7) return;

    LChartParams params(QColor(200, 100, 200), Qt::black);
    m_chart->addChart(params);

    //add points
    QList<QPointF> points;
    QPointF p0(-1, -5);
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

	    if (col == 7) p0.setY(0);
	}
	
	QPointF p(-1, -1);
	if (col == 2) p.setY(s.toInt());
	if (col == 7) p.setY(s.toDouble());
	p.setX(QDateTime(dt).toTime_t());
	points.append(p);
    }
    m_chart->addChartPoints(points, 0);
    m_chart->updateAxis();
}
void DivStatWidget::repaintStatChart(QTableWidget *table)
{
    if (!table) return;

    QList<int> sel_cols = LStatic::selectedCols(table);
    if (sel_cols.isEmpty()) return;
    int col = sel_cols.first();

    m_chart->removeChart();
    m_chart->setPointSize(4);
    if (col != 4 && col != 5) return;

    LChartParams params(QColor(200, 100, 200), Qt::darkGreen);
    LChartParams params_all(QColor(200, 100, 200), Qt::blue);
    m_chart->addChart(params);
    m_chart->addChart(params_all);


    QList<QPointF> points;
    QList<QPointF> points_all;
    QPointF p0(0, 0);
    for (int j=0; j<table->rowCount(); j++)
    {
	if (table->isRowHidden(j)) continue;

	QString s = table->item(j, col)->text().trimmed();
	QDate dt = QDate::fromString(table->item(j, 0)->text().trimmed(), DATE_MASK);
	QStringList list(s.split("/"));
	double v1 = list.at(0).toDouble();
	double v2 = list.at(1).toDouble();

	if (points.isEmpty())
	{
	    QDate dt0 = dt.addDays(-10);
	    p0.setX(QDateTime(dt0).toTime_t());
	    points.append(p0);
	    points_all.append(p0);
	}

	QPointF p(-1, -1);
	p.setX(QDateTime(dt).toTime_t());
	p.setY(v1);
	points.append(p);
	p.setY(v2);
	points_all.append(p);
    }

    m_chart->addChartPoints(points, 0);
    m_chart->addChartPoints(points_all, 1);
    m_chart->updateAxis();

}
void DivStatWidget::slotStatChart()
{
    repaintStatChart(statTable);
}
void DivStatWidget::slotBagChart()
{
    repaintStatChart(bagTable);
}
void DivStatWidget::updateColors()
{
    if (!lCommonSettings.paramValue("table_colors").toBool()) return;

    if (m_data.isEmpty()) return;

    QStringList list;
    QList<QColor> colors;
    QDate cur_dt(QDate::currentDate());
    QDate dt(QDate::fromString(m_data.recAtValue(0,ftDateCoupon), DATE_MASK));
    int month = dt.month();
    
    QColor c1 = QColor(250, 250, 230);	
    QColor c2 = QColor(200, 220, 220);	
    QColor c3 = QColor(100, 250, 100);
    QColor cc = c1;	
    int last_c = 1;
    int cur_month_divs = 0;
    for (int i=0; i<m_data.count(); i++)
    {
	dt = QDate::fromString(m_data.recAtValue(i,ftDateCoupon), DATE_MASK);
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

}
QList<int> DivStatWidget::headerList() const
{
    QList<int> list;
    list.append(ftDateCoupon);
    list.append(ftDateOperation);
    list.append(ftNote);
    list.append(ftCompany);
    list.append(ftCurrency);
    list.append(ftKKS);
    list.append(ftPaperType);
    list.append(ftCouponSize);
    return list;
}
QList<int> DivStatWidget::headerList2() const
{
    QList<int> list;
    list.append(ftDateCoupon);
    list.append(ftCompany);
    list.append(ftCurrency);
    list.append(ftPaperType);
    list.append(ftDivSize);
    list.append(ftNalogSize);
    return list;
}
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
void DivStatWidget::initTables()
{
    LStatic::fullClearTable(calendarTable);
    LStatic::fullClearTable(statTable);
    LStatic::fullClearTable(bagTable);

    QList<int> fields = headerList();
    QStringList headers;
    for (int i=0; i<fields.count(); i++)
	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));
    LStatic::setTableHeaders(calendarTable, headers);
    
    fields.clear();
    headers.clear();
    fields = headerList2();
    for (int i=0; i<fields.count(); i++)
	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));
    LStatic::setTableHeaders(statTable, headers);

    fields.clear();
    headers.clear();
    fields = headerList3();
    for (int i=0; i<fields.count(); i++)
	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));
    LStatic::setTableHeaders(bagTable, headers);


    calendarTable->verticalHeader()->hide();
    statTable->verticalHeader()->hide();
    bagTable->verticalHeader()->hide();

    connect(calendarTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotCalendarChart()));
    connect(statTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotStatChart()));
    connect(bagTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotBagChart()));
}
void DivStatWidget::fillCalendarTable()
{
    LStatic::removeAllRowsTable(calendarTable);

    m_data.fields.append(headerList());
    for (int i=0; i<m_data.count(); i++)
	LStatic::addTableRow(calendarTable, recToRow(m_data.records.at(i), m_data.fields));

    //LStatic::resizeTableContents(calendarTable);
    calendarTable->resizeRowsToContents();

}
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

    //LStatic::resizeTableContents(bagTable);
    bagTable->resizeRowsToContents();
}
void DivStatWidget::fillStatTable(ConfiguratorAbstractData *data)
{
    if (!data) {qWarning("DivStatWidget::fillStatTable ERR: data is null!"); return;}

    double nalog = 0;
    double divs = 0;
    double kurs = lCommonSettings.paramValue("kurs").toDouble();

    QList<QDate> dates;
    QString company, currency;
    QStringList row_data;
    LStatic::removeAllRowsTable(statTable);
    QString p_type;
    int cur_month = 0;
    for (int i=0; i<data->count(); i++)
    {
	row_data.clear();
	const ConfiguratorAbstractRecord &rec = data->records.at(i);
	int cid = rec.record.value(ftCompany).toInt();
	p_type = rec.record.value(ftPaperType).trimmed().toLower();


	row_data << rec.record.value(ftDateOperation);
	QDate dt = QDate::fromString(row_data.last(), DATE_MASK);
	
	emit signalGetCompanyByID(cid, company);
	emit signalGetCurrencyByID(cid, currency);
	currency = currency.trimmed().toLower();
	row_data << company << currency << p_type;

	double div = (p_type == "cfd") ? rec.record.value(ftPrice).toDouble() : rec.record.value(ftCouponSize).toDouble();
	double nlg = rec.record.value(ftNalogSize).toDouble();
	if (currency == "usd") {div *= kurs; nlg *= kurs;}
	row_data << QString("%1").arg(QString::number(div, 'f', 1));
	row_data << QString("%1").arg(QString::number(nlg, 'f', 2));

	int row_index = 0;
	if (!dates.isEmpty())
	{
	    int nd = dates.count();
	    for (int j=(nd-1); j>=0; j--)
		if (dt > dates.at(j)) {row_index = j+1; break;}
 	}
        dates.insert(row_index, dt);
        LStatic::insertTableRow(row_index, statTable, row_data);
                
    }
    
    
    for (int i=0; i<statTable->rowCount(); i++)
    {
	
	QTableWidgetItem *div_item = statTable->item(i, 4);
	QTableWidgetItem *nlg_item = statTable->item(i, 5);
	QTableWidgetItem *dt_item = statTable->item(i, 0);

	divs += div_item->text().toDouble();
	nalog += nlg_item->text().toDouble();
	
	div_item->setText(QString("%1/%2").arg(div_item->text()).arg(QString::number(divs, 'f', 0)));
	nlg_item->setText(QString("%1/%2").arg(nlg_item->text()).arg(QString::number(nalog, 'f', 0)));
	
	
	///////////////////find color////////////////////////////////////
	QDate dt = QDate::fromString(dt_item->text(), DATE_MASK);
        if (cur_month > 0 && cur_month != dt.month())
        {
    	    cur_month = dt.month();
    	    LStatic::setTableRowColor(statTable, i, Qt::gray);
        }
        else cur_month = dt.month();

    }

    //LStatic::resizeTableContents(statTable);
    statTable->resizeRowsToContents();
}
QStringList DivStatWidget::recToRow(const ConfiguratorAbstractRecord &rec, const QList<int> &fields) const
{
    QStringList list;
    for (int i=0; i<fields.count(); i++)
    {
	QString s = rec.record.value(fields.at(i), QString("???"));    
	list << s;
    }
    return list;
}
void DivStatWidget::save(QSettings &settings)
{
    settings.setValue(QString("%1/v_splitter").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/h_splitter1").arg(objectName()), h_splitter1->saveState());
    settings.setValue(QString("%1/h_splitter2").arg(objectName()), h_splitter2->saveState());
    saveTableState(settings, calendarTable);
    saveTableState(settings, statTable);
    saveTableState(settings, bagTable);
}
void DivStatWidget::load(QSettings &settings)
{
    QByteArray ba = settings.value(QString("%1/v_splitter").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) v_splitter->restoreState(ba);
    ba.clear();

    ba = settings.value(QString("%1/h_splitter1").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) h_splitter1->restoreState(ba);
    ba = settings.value(QString("%1/h_splitter2").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) h_splitter2->restoreState(ba);

    loadTableState(settings, calendarTable);
    loadTableState(settings, statTable);
    loadTableState(settings, bagTable);
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









