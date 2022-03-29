 #include "bagwidget.h"
 #include "lstatic.h"
 #include "cfdenums.h"
 #include "generaldatawidget.h"
 #include "lcommonsettings.h"

 #include <QDebug>
 #include <QDir>
 #include <QTimer>


/////////// BagWidget /////////////////////////////
BagWidget::BagWidget(QWidget *parent)
	:LChildWidget(parent),
	 m_search(NULL)
{
    setupUi(this);
    initTable();

    searchLineEdit->clear();
    listWidget->clear();
    tableListBox->hide();

    m_search = new LSearch(searchLineEdit, this);
    m_search->addTable(tableWidget, countLabel);
    m_search->exec();


    QTimer *timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(slotTimer()));
    timer1->start(4000);

}
void BagWidget::slotTimer()
{
    recalcState();
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
    return list;
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
	     LStatic::setTableRowColor(tableWidget, i, QColor(200, 200, 200));
	}
    }
}
/*
void BagWidget::setRowColor(int row, const QColor &color)
{
}
*/
void BagWidget::initTable()
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
}
void BagWidget::buy(const ConfiguratorAbstractRecord &rec)
{
//    qDebug("BagWidget::buy");
//    qDebug()<<rec.toString();
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
//    qDebug("BagWidget::slotBagUpdate");
//    qDebug()<<QString("BagWidget::slotBagUpdate")<<rec.toString();
    
    QString kks = rec.record.value(ftKKS);
    int id = rec.record.value(ftID).toInt();
    int pos = findRec(id, kks);
    if (pos < 0)
    {
	m_data.records.append(rec);
	QStringList list = recToRow(m_data.records.last());
	LStatic::addTableRow(tableWidget, list);
    }
    else
    {
	m_data.records[pos].copy(rec);
	QStringList list = recToRow(m_data.recAt(pos));
	LStatic::setTableRow(pos, tableWidget, list);
    }
//    return;
    
//    updateColors();
//    m_search->exec();
//    LStatic::resizeTableContents(tableWidget);

    if (rec.record.value(ftCount).toInt() < 0)
	QMessageBox::critical(this, tr("Critical error"), tr("Paper count < 0 for company - %1").arg(rec.record.value(ftCompany)));

}
void BagWidget::refresh()
{
    updateColors();
    LStatic::resizeTableContents(tableWidget);
    
    m_search->exec();
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



