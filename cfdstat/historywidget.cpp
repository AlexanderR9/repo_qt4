 #include "historywidget.h"
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


/////////// HistoryWidget /////////////////////////////
HistoryWidget::HistoryWidget(QWidget *parent)
    :LChildWidget(parent),
    v_splitter(NULL),
    h_splitter(NULL),
    m_chart(NULL),
    m_calc(NULL)
{
    setupUi(this);
    setObjectName("history_widget");

    initWidgets();
    readGeneralData();
    initCalc();
    initTables();
    initComboboxes();
//    initBagData();
//    qDebug("9");
    initChart();

    connect(historyTable, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(slotEditOperation()));
    connect(pricesTable, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(slotEditPrices()));
    connect(pricesTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotPriceChart()));

    slotQuery();
}
void HistoryWidget::initCalc()
{
    if (invalidData()) return;
    m_calc = new DivCalc(m_operationsData, this);
    m_calc->readGeneralData(m_err);
    if (isErr()) {showErr(); return;}

//    qDebug("1");
    if (lCommonSettings.paramValue("calc_divs").toBool())
    {
	m_calc->updateDivInfo();
	m_operationsData.addOtherData(m_calc->divInfo());
	sortDataByDate(m_operationsData); //long
    }

    connect(this, SIGNAL(signalGetDivData(ConfiguratorAbstractData*&)), m_calc, SLOT(slotSetDivData(ConfiguratorAbstractData*&)));
}
void HistoryWidget::readGeneralData()
{
    m_operationsData.generalType = gdHistory;
    m_pricesData.generalType = gdPrices;
    m_companyData.generalType = gdCompany;

    GeneralDataFileReader::loadDataFormFile(gdHistory, m_operationsData);
    GeneralDataFileReader::loadDataFormFile(gdPrices, m_pricesData);
    GeneralDataFileReader::loadDataFormFile(gdCompany, m_companyData);

    if (invalidData())
    {
	QString path = lCommonSettings.paramValue("datapath").toString().trimmed();
        if (!path.isEmpty())
	{
    	    QDir dir(path);
    	    if (dir.exists()) return;
	}

	m_err = QObject::tr("Error loading general data, check application settings.");
	showErr();
        return;
    }

    qDebug()<<QString("Data loaded ok!  prices count %1,  operations count %2,  company count %3").arg(m_pricesData.count()).arg(m_operationsData.count()).arg(m_companyData.count());
    
    sortDataByDate(m_operationsData);
    sortDataByDate(m_pricesData);
    convertCompanyCurrency();
}
void HistoryWidget::updateColors()
{
    if (!lCommonSettings.paramValue("table_colors").toBool()) return;

    QStringList list;
    QList<QColor> colors;
    QDate cur_dt(QDate::currentDate());
    
    for (int i=m_pricesData.count()-1; i>=0; i--)
    {
        int id = m_pricesData.recAtValue(i, ftCompany).toInt();
        QString kks = m_pricesData.recAtValue(i, ftKKS).trimmed();
	QString s = QString("%1*%2").arg(id).arg(kks);
	if (list.contains(s)) continue;
	else list.append(s);

	QColor color = QColor(255, 255, 255);
        QDate dt = QDate::fromString(m_pricesData.recAtValue(i, ftDateOperation), DATE_MASK);
	int d = dt.daysTo(cur_dt);
//	qDebug()<<QString("id=%1  kks=%2  dt=%3  daysTo=%4").arg(id).arg(kks).arg(dt.toString(DATE_MASK)).arg(d);

	if (d > 7) color = QColor(250, 250, 230);	
	if (d > 14) color = QColor(250, 250, 100);	
	if (d > 21) color = QColor(250, 250, 0);	
	if (d > 30) color = QColor(250, 150, 0);	
	if (d > 45) color = QColor(250, 0, 0);	
	if (d > 60) color = QColor(150, 0, 0);	
	if (d > 90) color = QColor(50, 70, 20);	
	colors.append(color);
    }    
    
    for (int i=m_pricesData.count()-1; i>=0; i--)
    {
        int id = m_pricesData.recAtValue(i, ftCompany).toInt();
        QString kks = m_pricesData.recAtValue(i, ftKKS).trimmed();
	QString s = QString("%1*%2").arg(id).arg(kks);
	int pos = list.indexOf(s);
	if (pos >= 0)
	{
	    LStatic::setTableRowColor(pricesTable, i, colors.at(pos));
	}
    }
}
void HistoryWidget::initChart()
{
    m_chart = new LChartWidget(this);
    if (chartBox->layout()) delete chartBox->layout();
    chartBox->setLayout(new QHBoxLayout(0));
    chartBox->layout()->addWidget(m_chart);

    m_chart->removeChart();
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    LChartParams params(QColor(0, 100, 0), Qt::black);
    m_chart->addChart(params);

}
void HistoryWidget::slotPriceChart()
{	
//    qDebug("HistoryWidget::slotPriceChart()");
    QList<int> sel_rows = LStatic::selectedRows(pricesTable);
    if (sel_rows.isEmpty()) return;
    
    QString co = pricesTable->item(sel_rows.first(), 1)->text();
    int id = pricesTable->item(sel_rows.first(), 1)->data(Qt::UserRole).toInt(); 
    QString kks = pricesTable->item(sel_rows.first(), 2)->text(); 

    QMap<QString, double> map;
    getPricesByID(id, kks, map);    
    chartBox->setTitle(QString("Prices: [%1]  id=%2,  points count %3").arg(co).arg(id).arg(map.count()));


    QList<QPointF> points;
    convertPricesToPoints(map, points);
    for (int j=0; j<points.count(); j++)
    {
	QDateTime dt;
	dt.setTime_t(points.at(j).x());
    }
    m_chart->clearChartPoints();
    m_chart->addChartPoints(points, 0);
    m_chart->updateAxis();
}
void HistoryWidget::convertPricesToPoints(const QMap<QString, double> &map, QList<QPointF> &points)
{
    points.clear();
    QStringList keys(map.keys());
    for (int i=0; i<keys.count(); i++)
    {
	QDate dt(QDate::fromString(keys.at(i), DATE_MASK));
	uint x = QDateTime(dt).toTime_t();
	double y = map.value(keys.at(i));
	QPointF p(x, y);
	if (points.isEmpty()) {points.append(p); continue;}
	if (x > points.last().x()) {points.append(p); continue;}
	
	int index = 0;
	for (int j=0; j<points.count(); j++)
	{
	    if (x > points.at(j).x()) index++;
	    else break;
	}
	points.insert(index, p);
    }
}
void HistoryWidget::initBagData()
{
    if (invalidData()) return;

    for (int i=0; i<m_operationsData.count(); i++)
	updateBag(m_operationsData.recAt(i));
}
void HistoryWidget::slotEditPrices()
{
    QList<int> sel_rows = LStatic::selectedRows(pricesTable);
    if (sel_rows.isEmpty())
    {
        QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("Not selected table data"));
        return;
    }
    if (sel_rows.count() > 1)
    {
        QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("You must select only one record"));
        return;
    }

    ConfiguratorAbstractRecord rec(m_pricesData.recAt(sel_rows.first()));
    rec.record.insert(ftDateOperation, QDate::currentDate().toString(DATE_MASK));

    EditPricesDialog d(rec, this);
    d.init();
    d.exec();

    if (d.isApply())
    {
	setPrice(rec);
    }

}
void HistoryWidget::slotEditOperation()
{
    QList<int> sel_rows = LStatic::selectedRows(historyTable);
    if (sel_rows.isEmpty())
    {
        QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("Not selected table data"));
        return;
    }
    if (sel_rows.count() > 1)
    {
        QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("You must select only one record"));
        return;
    }
    
    ConfiguratorAbstractRecord &rec = m_operationsData.records[sel_rows.first()];
    int type = rec.record.value(ftTypeOperation).toInt();
    if (type != opBuy && type != opSell)
    {
        QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("You can not edit this operation"));
        return;
    }

    OperationEditDialog d(rec, this);
    d.init();
    d.exec();

    if (d.isApply())
    {
	updateOperation(sel_rows.first());
    }
}
void HistoryWidget::updateOperation(int row)
{
    QStringList list = recToRow(m_operationsData.recAt(row), m_operationsData.fields);
    LStatic::setTableRow(row, historyTable, list);

    setPrice(m_operationsData.recAt(row));	
}
void HistoryWidget::initComboboxes()
{
    currencyComboBox->clear();
    currencyComboBox->addItem("none", 0);
    companyComboBox->clear();
    companyComboBox->addItem("none", 0);
    for (int i=0; i<m_companyData.count(); i++)
    {
	int id = m_companyData.recAtValue(i, ftID).toInt();
	QString name = m_companyData.recAtValue(i, ftName).trimmed();
	companyComboBox->addItem(name, id);

	QString crs = m_companyData.recAtValue(i, ftCurrency).trimmed();
	if (currencyComboBox->findText(crs) < 0) currencyComboBox->addItem(crs);

    }

    operationTypeComboBox->clear();
    operationTypeComboBox->addItem("none", 0);
    QList<int> op_list = ConfiguratorEnums::operationTypes();
    for (int i=0; i<op_list.count(); i++)
    {
	QString text = ConfiguratorEnums::interfaceTextByType(op_list.at(i));
	operationTypeComboBox->addItem(text, op_list.at(i));
    }
    
    paperTypeComboBox->clear();
    paperTypeComboBox->addItem("none", 0);
    paperTypeComboBox->addItem(ConfiguratorEnums::interfaceTextByType(gdCFD), gdCFD);
    paperTypeComboBox->addItem(ConfiguratorEnums::interfaceTextByType(gdBond), gdBond);

    /////////////////////////////////////////////////////////
    connect(companyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuery()));
    connect(currencyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuery()));
    connect(operationTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuery()));
    connect(paperTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuery()));
    connect(searchLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotQuery()));

}
void HistoryWidget::clearQuery()
{
    paperTypeComboBox->setCurrentIndex(0);
    operationTypeComboBox->setCurrentIndex(0);
    companyComboBox->setCurrentIndex(0);
    currencyComboBox->setCurrentIndex(0);
    searchLineEdit->clear();
    slotQuery();
}
void HistoryWidget::slotQuery()
{
//    qDebug("HistoryWidget::slotQuery()");

    int id_company = companyComboBox->itemData(companyComboBox->currentIndex()).toInt();    
    QString paper = paperTypeComboBox->currentText();    
    QString currency = currencyComboBox->currentText();    
    QString operation = operationTypeComboBox->currentText();    
    QString text = searchLineEdit->text().trimmed();


    ///////////////////// check operations table //////////////////////////////
    int vn = 0;
    int rows = historyTable->rowCount();
    int cols = historyTable->columnCount();
    for (int i=0; i<rows; i++)
    {
	int cid = m_operationsData.recAtValue(i, ftCompany).toInt();
	bool visible = true;
	if ((id_company != 0) && (cid != id_company)) visible = false;
	if ((currency != "none") && (companyCurrencyByID(cid) != currency)) visible = false;
	if ((paper != "none") && (historyTable->item(i, 3)->text() != paper)) visible = false;
//	if ((operation != "none") && (historyTable->item(i, 1)->text() != operation)) visible = false;
	if ((operation != "none") && !operation.contains(historyTable->item(i, 1)->text())) visible = false;

	if (visible && !text.isEmpty())
	{
	    visible = false;
	    for (int j=0; j<cols; j++)
		if (historyTable->item(i, j)->text().contains(text)) {visible = true; break;}
	}

	historyTable->setRowHidden(i, !visible);
	if (visible) vn++;
    }
    operationsLabel->setText(QString("Operations count: %1/%2").arg(vn).arg(rows));


    ///////////////////// check prices table //////////////////////////////
    vn = 0;
    rows = pricesTable->rowCount();
    cols = pricesTable->columnCount();
    for (int i=0; i<rows; i++)
    {
	int cid = m_pricesData.recAtValue(i, ftCompany).toInt();
	QString kks = m_pricesData.recAtValue(i, ftKKS).trimmed();
	bool visible = true;
	if ((id_company != 0) && (cid != id_company)) visible = false;
	if ((currency != "none") && (companyCurrencyByID(cid) != currency)) visible = false;
	if (paper != "none")
	{
	    if (paper == ConfiguratorEnums::interfaceTextByType(gdCFD) && !kks.isEmpty()) visible = false;
	    if (paper == ConfiguratorEnums::interfaceTextByType(gdBond) && kks.isEmpty()) visible = false;
	}

	if (visible && !text.isEmpty())
	{
	    visible = false;
	    for (int j=0; j<cols; j++)
		if (pricesTable->item(i, j)->text().contains(text)) {visible = true; break;}
	}

	pricesTable->setRowHidden(i, !visible);
	if (visible) vn++;
    }
    pricesLabel->setText(QString("Line count: %1/%2").arg(vn).arg(rows));

    LStatic::resizeTableContents(historyTable);
    LStatic::resizeTableContents(pricesTable);

}
void HistoryWidget::initTables()
{
    initTable(historyTable);
    initTable(pricesTable);
    fillTable(historyTable);
    fillTable(pricesTable);
    LStatic::resizeTableContents(historyTable);
    LStatic::resizeTableContents(pricesTable);

    updateColors();
}
void HistoryWidget::initTable(QTableWidget *table)
{
//    qDebug("HistoryWidget::initTable(QTableWidget *table)");
    if (!table) return;
    LStatic::fullClearTable(table);

    const QList<int> &fields = table->objectName().contains("history") ? m_operationsData.fields : m_pricesData.fields;
    QStringList headers;
    for (int i=0; i<fields.count(); i++)
	headers << ConfiguratorEnums::interfaceTextByType(fields.at(i));

    LStatic::setTableHeaders(table, headers);
    table->verticalHeader()->hide();
}
int HistoryWidget::findMinDateOperation(const ConfiguratorAbstractData &data, int start_index) const
{
    if (start_index < 0 || start_index >= data.count()) return -1;

    int index = start_index;    
    QDate min_dt;
    for (int i=start_index; i<data.count(); i++)
    {
	QDate dt = QDate::fromString(data.recAt(i).record.value(ftDateOperation), DATE_MASK);
	if (i == start_index) {min_dt = dt; continue;}
	if (min_dt > dt) {min_dt = dt; index = i;}
    }
    return index;
}
void HistoryWidget::sortDataByDate(ConfiguratorAbstractData &data)
{
    int ft = ftDateOperation;
    if (data.count() < 2 || !data.fields.contains(ft)) return;
    
    int start_index = 0;
    while (start_index < data.count())
    {
	int min_index = findMinDateOperation(data, start_index);
	if (min_index > start_index)
	    data.replaceRecords(start_index, min_index);
	start_index++;
    }
    
    

/*
    int ft = ftDateOperation;
    if (data.count() < 2 || !data.fields.contains(ft)) return;

    bool was_changed = false;
    for (int i=1; i<data.count(); i++)
    {
	QDate dt1 = QDate::fromString(data.recAt(i-1).record.value(ft), DATE_MASK);
	QDate dt2 = QDate::fromString(data.recAt(i).record.value(ft), DATE_MASK);
	if (dt1 > dt2) 
	{
	    data.replaceRecords(i-1, i);
	    was_changed = true;
	    break;
	}
    }

    if (was_changed) sortDataByDate(data);
    */
}
void HistoryWidget::fillTable(QTableWidget *table)
{
    if (!table) return;
    LStatic::removeAllRowsTable(table);

    const ConfiguratorAbstractData &data = table->objectName().contains("history") ? m_operationsData : m_pricesData;
    for (int i=0; i<data.count(); i++)
    {
	LStatic::addTableRow(table, recToRow(data.records.at(i), data.fields));
	if (!table->objectName().contains("history"))
	    table->item(table->rowCount()-1, 1)->setData(Qt::UserRole, data.recAtValue(i, ftCompany).toInt());
    }
}
QStringList HistoryWidget::recToRow(const ConfiguratorAbstractRecord &rec, const QList<int> &fields) const
{
    QStringList list;
    bool ok;
    for (int i=0; i<fields.count(); i++)
    {
	QString s = rec.record.value(fields.at(i), QString("???"));    
	if (ConfiguratorEnums::isDoubleField(fields.at(i)))
	{
	    double d = s.toDouble(&ok);
	    if (!ok) s = "??";
	    else s = QString::number(d, 'f', lCommonSettings.paramValue("precision").toInt());
	}
	else if (fields.at(i) == ftCompany)
	{
	    s = companyByID(s.toInt());
	}
	else if (fields.at(i) == ftTypeOperation)
	{
	    s = ConfiguratorEnums::interfaceTextByType(s.toInt());
	}

	list << s;
    }

    return list;
}
bool HistoryWidget::invalidData() const
{
    return (m_operationsData.invalid() || m_pricesData.invalid() || m_companyData.invalid());
}
void HistoryWidget::convertCompanyCurrency()
{
    if (m_companyData.invalid()) return;

    ConfiguratorAbstractData  currency_data;
    GeneralDataFileReader::loadDataFormFile(gdCurrency, currency_data);
    if (currency_data.invalid()) return;

    int n = m_companyData.count();
    for (int i=0; i<n; i++)
    {
	int cur_id = m_companyData.recAt(i).record.value(ftCurrency).toInt();
	const ConfiguratorAbstractRecord *cur_rec = currency_data.recByFieldValue(ftID, QString::number(cur_id));
	if (!cur_rec) qWarning()<<QString("HistoryWidget::convertCompanyCurrency() ERR: not found currency by id=%1").arg(cur_id);
	else m_companyData.setRecordValue(i, ftCurrency, cur_rec->record.value(ftName));
    }
}
void HistoryWidget::initWidgets()
{
    if (layout()) delete layout();
    QVBoxLayout *lay = new QVBoxLayout(0);
    setLayout(lay);
    lay->addWidget(queryBox);

    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter = new QSplitter(Qt::Horizontal, this);
    lay->addWidget(v_splitter);

    v_splitter->addWidget(h_splitter);
    v_splitter->addWidget(chartBox);
    h_splitter->addWidget(operationsBox);
    h_splitter->addWidget(pricesBox);
}
void HistoryWidget::save(QSettings &settings)
{
    settings.setValue(QString("%1/v_splitter").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/h_splitter").arg(objectName()), h_splitter->saveState());
}
void HistoryWidget::load(QSettings &settings)
{
    QByteArray ba = settings.value(QString("%1/v_splitter").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) v_splitter->restoreState(ba);
    ba = settings.value(QString("%1/h_splitter").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) h_splitter->restoreState(ba);
}
void HistoryWidget::slotGetCurrentPrice(const ConfiguratorAbstractRecord &rec, double &p1)
{
    p1 = -1;
    int id = rec.record.value(ftID, QString("-1")).toInt();
    QString kks = rec.record.value(ftKKS).trimmed();
    if (id <= 0) {qWarning("HistoryWidget::slotGetCurrentPrice: ERR - invalid ID conpany"); return;}

    for (int i=(m_pricesData.count()-1); i>=0; i--)
    {
	const ConfiguratorAbstractRecord &p_rec = m_pricesData.records.at(i);
	if ((p_rec.record.value(ftCompany).toInt()) == id && p_rec.record.value(ftKKS).trimmed() == kks)
	{
	    p1 = m_pricesData.recAtValue(i, ftPrice1).toDouble();
	    break;
	}
    }    
}
QString HistoryWidget::companyCurrencyByID(int id) const
{
    for (int i=(m_companyData.count()-1); i>=0; i--)
    {
	if (m_companyData.recAtValue(i, ftID).toInt() == id)
	    return m_companyData.recAtValue(i, ftCurrency);
    }
    return "unknown";
}
void HistoryWidget::getPricesByID(int id, const QString &kks, QMap<QString, double> &map)
{
    map.clear();
    for (int i=0; i<m_pricesData.count(); i++)
    {
	const ConfiguratorAbstractRecord &p_rec = m_pricesData.records.at(i);
	if (p_rec.record.value(ftCompany).toInt() == id && p_rec.record.value(ftKKS).trimmed() == kks)
	{
	    double p = m_pricesData.recAtValue(i, ftPrice1).toDouble();
	    QString dt = m_pricesData.recAtValue(i, ftDateOperation);
	    map.insert(dt, p);
	}
    }
}
QString HistoryWidget::companyByID(int id) const
{
    for (int i=(m_companyData.count()-1); i>=0; i--)
    {
	if (m_companyData.recAtValue(i, ftID).toInt() == id)
	    return QString("%1 (%2)").arg(m_companyData.recAtValue(i, ftName)).arg(m_companyData.recAtValue(i, ftShortName));
    }
    return "unknown";
}
void HistoryWidget::convertToPricesRecord(const ConfiguratorAbstractRecord &rec1, ConfiguratorAbstractRecord &rec2)
{
    rec2.reset();
    rec2.copy(rec1);
    const QList<int> &fields = rec2.record.keys();
    for (int i=0; i<fields.count(); i++)
	if (!m_pricesData.fields.contains(fields.at(i)))
	    rec2.record.remove(fields.at(i));
}
int HistoryWidget::countPaper(int id, const QString &kks) const
{
    int n = 0;
    for (int i=0; i<m_operationsData.count(); i++)
    {
	if (m_operationsData.recAt(i).value(ftKKS) == kks && m_operationsData.recAt(i).value(ftCompany).toInt() == id)
	{	
	    switch(m_operationsData.recAt(i).value(ftTypeOperation).toInt())
	    {
		case opBuy: {n += m_operationsData.recAt(i).value(ftCount).toInt(); break;}
		case opSell: {n -= m_operationsData.recAt(i).value(ftCount).toInt(); break;}
		default: break;
	    }
	}
    }
    return n;
}
double HistoryWidget::payedSize(int id, const QString &kks) const
{
    double payed = 0;
    for (int i=0; i<m_operationsData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec = m_operationsData.recAt(i);
	int op_type = rec.record.value(ftTypeOperation).toInt();	

	switch(op_type)
	{
	    case opBuy:
	    {
		if (rec.record.value(ftKKS) == kks && rec.record.value(ftCompany).toInt() == id)
		    payed += rec.record.value(ftPrice).toDouble();
		break;
	    }
	    case opSell:
	    {
		if (rec.record.value(ftKKS) == kks && rec.record.value(ftCompany).toInt() == id)
		    payed -= rec.record.value(ftPrice).toDouble();
		break;
	    }
	    default: break;
	}

//	if (op_type != opBuy) continue;
//	if (rec.record.value(ftKKS) == kks && rec.record.value(ftCompany).toInt() == id)
//	    payed += rec.record.value(ftPrice).toDouble();
    }
    return payed;
}
double HistoryWidget::divsSize(int id, const QString &kks) const
{
    double divs = 0;
    for (int i=0; i<m_operationsData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec = m_operationsData.recAt(i);
	int op_type = rec.record.value(ftTypeOperation).toInt();	
	if ((op_type != opCouponReceive) && (op_type != opDivReceive)) continue;

	if (rec.record.value(ftKKS) == kks && rec.record.value(ftCompany).toInt() == id)
	{
	    if (op_type == opDivReceive) divs += rec.record.value(ftPrice).toDouble();
	    else divs += rec.record.value(ftCouponSize).toDouble();
	}
    }
    return divs;
}
double HistoryWidget::lastPrice(int id, const QString &kks) const
{
    double p1 = -1;
    QDate last_date;
    for (int i=0; i<m_pricesData.count(); i++)
    {
	const ConfiguratorAbstractRecord &rec = m_pricesData.recAt(i);
	if (rec.record.value(ftKKS) == kks && rec.record.value(ftCompany).toInt() == id)
	{	
	    if (p1 < 0)
	    {
		p1 = rec.record.value(ftPrice1).toDouble();
		last_date = QDate::fromString(rec.record.value(ftDateOperation), DATE_MASK);
	    }
	    else
	    {
		QDate dt = QDate::fromString(rec.record.value(ftDateOperation), DATE_MASK);
		if (dt > last_date)
		{
		    p1 = rec.record.value(ftPrice1).toDouble();
		    last_date = dt;
		}
	    }
	}
    }
    return p1;
}
void HistoryWidget::setPrice(const ConfiguratorAbstractRecord &rec)
{
    qDebug("HistoryWidget::setPrice");
    QString dt = rec.record.value(ftDateOperation);
    QString kks = rec.record.value(ftKKS);
    int id = rec.record.value(ftCompany).toInt();
    int pos = -1;
    for (int i=0; i<m_pricesData.count(); i++)
    {
	
	if (m_pricesData.recAt(i).value(ftDateOperation) == dt && m_pricesData.recAt(i).value(ftKKS) == kks && 
		m_pricesData.recAt(i).value(ftCompany).toInt() == id)
	{
	    pos = i;
	    break;
	}
    }

    if (pos < 0)
    {
	ConfiguratorAbstractRecord rec2;
	convertToPricesRecord(rec, rec2);
        LStatic::addTableRow(pricesTable, recToRow(rec2, m_pricesData.fields));
	LStatic::resizeTableContents(pricesTable);
	m_pricesData.records.append(rec2);
	pricesTable->item(pricesTable->rowCount()-1, 1)->setData(Qt::UserRole, id);
    }
    else
    {
	m_pricesData.setRecordValue(pos, ftPrice1, rec.record.value(ftPrice1));
	QStringList list = recToRow(m_pricesData.recAt(pos), m_pricesData.fields);
	LStatic::setTableRow(pos, pricesTable, list);
    }

    updateBag(rec);
    slotQuery();
}
void HistoryWidget::saveData()
{
    if (!m_calc->isEmpty())
    {
	QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QString("Divs list is not empty."));	
	return;
    }
    
    QString f1 = GeneralDataFileReader::xmlFileByType(m_operationsData.generalType);
    QString f2 = GeneralDataFileReader::xmlFileByType(m_pricesData.generalType);
    QString question = QObject::tr("Are you sure want rewrite files %1 and %2?").arg(f1).arg(f2);

    if (QMessageBox::question(this, MSGBOX_REWRITEFILE_TITLE, question, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
        GeneralDataFileReader::writeDataToFile(m_operationsData, m_err);
        if (isErr()) {showErr(); return;}

        GeneralDataFileReader::writeDataToFile(m_pricesData, m_err);
        if (isErr()) {showErr(); return;}
    }
}
void HistoryWidget::slotNextOperation(int op_type, const ConfiguratorAbstractRecord &rec)
{
    qDebug("HistoryWidget::slotNextOperation");
    m_operationsData.records.append(rec);
    m_operationsData.records.last().record.insert(ftTypeOperation, QString::number(op_type));

    if (!rec.record.contains(ftNalogSize))
	m_operationsData.records.last().record.insert(ftNalogSize, QString::number(0));

    LStatic::addTableRow(historyTable, recToRow(m_operationsData.records.last(), m_operationsData.fields));
    LStatic::resizeTableContents(historyTable);
    
    setPrice(m_operationsData.records.last());	
}
void HistoryWidget::updateBag(const ConfiguratorAbstractRecord &rec)
{
    QString kks = rec.record.value(ftKKS);
    int id = rec.record.value(ftCompany).toInt();
    int count = countPaper(id, kks);
    double p1 = lastPrice(id, kks);
    if (p1 < 0 || count < 0) {qWarning("HistoryWidget::updateBag - ERR: (p1 < 0 || count < 0)"); return;}
    double payed = payedSize(id, kks);
    double diff = p1*count - payed;
    double divs = divsSize(id, kks);
    
    //prepare bag record
    int prec = lCommonSettings.paramValue("precision").toInt();
    ConfiguratorAbstractRecord bag_rec;
    bag_rec.record.insert(ftCurrency, companyCurrencyByID(id));
    bag_rec.record.insert(ftID, QString::number(id));
    bag_rec.record.insert(ftCompany, companyByID(id));
    bag_rec.record.insert(ftCount, QString::number(count));
    bag_rec.record.insert(ftKKS, kks);
    bag_rec.record.insert(ftPaperType, kks.isEmpty() ? "cfd" : "bond");
    bag_rec.record.insert(ftPrice, QString::number(p1*count, 'f', prec));
    bag_rec.record.insert(ftPayedSize, QString::number(payed, 'f', prec));
    bag_rec.record.insert(ftDifference, QString::number(diff, 'f', prec));
    bag_rec.record.insert(ftCouponSize, QString::number(divs, 'f', prec));

    emit signalBagUpdate(bag_rec);
}










