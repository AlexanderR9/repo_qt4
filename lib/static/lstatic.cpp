 #include "lstatic.h"

 #include <QLineEdit>
 #include <QComboBox>
 #include <QLabel>
 #include <QGridLayout>
 #include <QDialogButtonBox>
 #include <QPushButton>
 #include <QListWidget>
 #include <QTableWidget>
 #include <QDebug>
 #include <QTime>
 #include <QTest>
 #include <QTableWidgetItem>
 #include <QTimer>
 #include <QTextCodec>




/////////////LSearch/////////////////////////
LSearch::LSearch(const QLineEdit *edit, QObject *parent)
	:QObject(parent),
	m_edit(edit),
	m_timer(NULL),
	m_colIndex(-1),
	m_delay(-1),
	m_timeout(false)
{
	if (!m_edit) return;

	connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));

}
void LSearch::setDelay(int v)
{
	m_timeout = false;
	if (v < 100) return;
	m_delay = v;
	if (m_timer) return;

	m_timer = new QTimer(this);
	m_timer->setInterval(m_delay);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void LSearch::slotTimer()
{
	m_timer->stop();
	m_timeout = true;
	exec();
}
void LSearch::slotSearch(const QString &s)
{
	if (m_timer)
	{
		if (m_timeout)
		{
			m_timeout = false;
		}
		else
		{
			if (m_timer->isActive()) m_timer->stop();
			m_timer->start();
			return;
		}
	}

	//// try search
	for (int i=0; i<m_structs.count(); i++)
	{
		int rows = rowCount(i);
		if (m_structs.at(i).table)
		{
			int cols = m_structs.at(i).table->columnCount();
			for (int row=0; row<rows; row++)
			{
				m_structs.at(i).table->hideRow(row);
				for (int col=0; col<cols; col++)
				{
					if (m_structs.at(i).table->item(row, col)->text().contains(s))
					{
						m_structs.at(i).table->showRow(row);
						break;
					}
				}
			}
			m_structs.at(i).table->resizeColumnsToContents();
		}
		else if (m_structs.at(i).list)
		{
			for (int row=0; row<rows; row++)
			{
				if (m_structs.at(i).list->item(row)->text().contains(s))
					m_structs.at(i).list->item(row)->setHidden(false);
				else m_structs.at(i).list->item(row)->setHidden(true);
			}
		}

		setText(i);
	}

	emit signalSearched();

//	qDebug("LSearch end");
}
void LSearch::exec()
{
	if (m_edit)
		slotSearch(m_edit->text());
}
void LSearch::updateText()
{
	for (int i=0; i<m_structs.count(); i++)
	{
		setText(i);
		if (m_structs.at(i).table)
			m_structs.at(i).table->resizeColumnsToContents();
	}
}
void LSearch::addTable(QTableWidget *tw, QLabel *l, QString s)
{
	if (!tw) return;
	m_structs.append(LStructSearch(l, tw, NULL, s));
	tw->resizeColumnsToContents();
	tw->resizeRowsToContents();
}
void LSearch::addList(QListWidget *lw, QLabel *l, QString s)
{
	if (!lw) return;
	m_structs.append(LStructSearch(l, NULL, lw, s));
}
int LSearch::visibleRows(int struct_index) const
{
	if (struct_index < 0 || struct_index >= m_structs.count()) return -1;

	int n = 0;
	int rows = rowCount(struct_index);

	if (m_structs.at(struct_index).table)
	{
		for (int i=0; i<rows; i++)
			if (!m_structs.at(struct_index).table->isRowHidden(i)) n++;
	}
	else if (m_structs.at(struct_index).list)
	{
		for (int i=0; i<rows; i++)
			if (!m_structs.at(struct_index).list->isRowHidden(i)) n++;
	}

	return n;
}
int LSearch::rowCount(int struct_index) const
{
	if (struct_index < 0 || struct_index >= m_structs.count()) return -1;
	if (m_structs.at(struct_index).table)
		return m_structs.at(struct_index).table->rowCount();
	if (m_structs.at(struct_index).list)
		return m_structs.at(struct_index).list->count();
	return -1;
}
void LSearch::setText(int struct_index)
{
	if (struct_index < 0 || struct_index >= m_structs.count()) return;
	if (!m_structs.at(struct_index).label) return;
	QString s = QString("%1: %2/%3").arg(tr("Record number")).arg(visibleRows(struct_index)).arg(rowCount(struct_index));
	if (!m_structs.at(struct_index).caption.isEmpty()) s = QString("%1. %2").arg(m_structs.at(struct_index).caption).arg(s);
	m_structs.at(struct_index).label->setText(s);
}


/////////////LSplash/////////////////////////
LSplash::LSplash(QWidget *parent)
    :QWidget(parent, Qt::SplashScreen),
     m_label(NULL)
{
    if (layout()) delete layout();
    setLayout(new QVBoxLayout(0));

    m_label = new QLabel("Loading");
    layout()->addWidget(m_label);

    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    m_label->setFont(font);
    QPalette plt;
    plt.setColor(QPalette::WindowText, Qt::blue);
    m_label->setPalette(plt);

    stopDelay();
}
void LSplash::startDelay(const QString &s)
{
    QString caption = s.isEmpty() ? tr("Loading...........") : s;
    m_label->setText(caption);
    show();
    QTest::qWait(100);
}
void LSplash::stopDelay()
{
    hide();
    QTest::qWait(100);
}



/////////////LStatic/////////////////////////
void LStatic::insertTableRow(int index, QTableWidget *table, const QStringList &list, int align, QColor cf, QColor cb)
{
    if (!table) return;
    if (index < 0 || index > table->rowCount()) return;
    if (table->columnCount() != list.count())
    {
	qWarning()<<QString("LStatic::addTableRow - ERR: col count(%1) != list count(%2)").arg(table->columnCount()).arg(list.count());
	return;
    }

    if (table->rowCount() == index)
    {
	addTableRow(table, list, align, cf, cb);
	return;
    }

    table->insertRow(index);
    setTableRow(index, table, list, align, cf, cb);
}
QString LStatic::baToStr(const QByteArray &ba, int line_size)
{
    QString s;
    if (ba.isEmpty()) return QString("ByteArray is empty!");

    int cur_len = 0;
    for (int i=0; i<ba.count(); i++)
    {
        QString s16 = QString::number(ba.at(i), 16);
        if (s16.isEmpty()) continue;
        if (s16.length() > 2) s16 = s16.right(2);
        if (s16.length() == 1) s16 = s16.prepend("0");

        s.append(s16);
        s.append(QString("  "));
        cur_len++;

        if (line_size > 0 && cur_len == line_size)
        {
            s.append(QString("\n"));
            cur_len = 0;
        }
    }
    return s;
}
void LStatic::addTableRow(QTableWidget *table, const QStringList &list, int align, QColor cf, QColor cb)
{
    if (!table) return;
    if (table->columnCount() != list.count())
    {
	qWarning()<<QString("LStatic::addTableRow - ERR: col count(%1) != list count(%2)").arg(table->columnCount()).arg(list.count());
	return;
    }

    table->setRowCount(table->rowCount()+1);
    for (int i=0; i<table->columnCount(); i++)
    {
	QTableWidgetItem *item = new QTableWidgetItem(list.at(i));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (align >= 0) item->setTextAlignment(align);
	item->setForeground(QBrush(cf));
	item->setBackground(QBrush(cb));
	table->setItem(table->rowCount()-1, i, item);
    }
}
void LStatic::setTableRow(int row, QTableWidget *table, const QStringList &list, int align, QColor c, QColor cb)
{
    if (!table) return;

    if (table->columnCount() != list.count())
    {
	qWarning()<<QString("LStatic::setTableRow - ERR: col count(%1) != list count(%2)").arg(table->columnCount()).arg(list.count());
	return;
    }

    if (row < 0 || row >= table->rowCount())
    {
	qWarning()<<QString("LStatic::setTableRow - ERR: invalid row index: %1").arg(row);
	return;
    }

    for (int i=0; i<table->columnCount(); i++)
    {
	QTableWidgetItem *item = table->item(row, i);
	if (!item) 
	{
	    item = new QTableWidgetItem(list.at(i));
	    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	    table->setItem(row, i, item);
	}

	item->setText(list.at(i));
	if (align >= 0) item->setTextAlignment(align);
	item->setForeground(QBrush(c));
	item->setBackground(QBrush(cb));
    }
}
void LStatic::createTableItem(QTableWidget *table, int row, int col, const QString &text, int flags, int align, QColor c)
{
    if (!table) return;

    if (table->columnCount() <= col || col < 0)
    {
	qWarning()<<QString("LStatic::createTableItem - ERR: cols count(%1) , param col (%2)").arg(table->columnCount()).arg(col);
	return;
    }

    if (table->rowCount() <= row || row < 0)
    {
	qWarning()<<QString("LStatic::createTableItem - ERR: rows count(%1) , param row (%2)").arg(table->rowCount()).arg(row);
	return;
    }

    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setFlags(Qt::ItemFlags(flags));
    item->setTextAlignment(align);
    item->setTextColor(c);
    table->setItem(row, col, item);
}
void LStatic::resizeTableContents(QTableWidget *table)
{
    if (!table) return;
    table->resizeColumnsToContents();
    table->resizeRowsToContents();

}
void LStatic::setTableRowColor(QTableWidget *table, int row, const QColor &c)
{
//    qDebug()<<QString("LStatic::setTableRowColor  row=%0/%1, color=%2").arg(table->rowCount()).arg(row).arg(c.red());
    if (!table) return;
    if (row < 0 || row >= table->rowCount()) return;
    for (int i=0; i<table->columnCount(); i++)
    {
	QTableWidgetItem *item = table->item(row, i);
	if (item) item->setBackground(QBrush(c));
    }
}
void LStatic::clearAllItemsText(QTableWidget *table)
{
    if (!table) return;
    for (int i=0; i<table->rowCount(); i++)
	for (int j=0; j<table->columnCount(); j++)
	{
	    QTableWidgetItem *item = table->item(i, j);
	    if (item) item->setText(QString());
	}
}
void LStatic::createAllItems(QTableWidget *table, int align)
{
    if (!table) return;
    table->clearContents();

    for (int i=0; i<table->rowCount(); i++)
	for (int j=0; j<table->columnCount(); j++)
	{
	    QTableWidgetItem *item = new QTableWidgetItem(QString());
	    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	    if (align >= 0) item->setTextAlignment(align);
	    table->setItem(i, j, item);
	}
}
void LStatic::setTableHeaders(QTableWidget *table, const QStringList &list, int orintation)
{
    if (!table || list.isEmpty()) return;
    
        
    if (orintation == Qt::Horizontal)
    {
	table->setColumnCount(list.count());
        table->setHorizontalHeaderLabels(list);
    }
    else
    {
	table->setRowCount(list.count());
        table->setVerticalHeaderLabels(list);
    }


}
void LStatic::fullClearTable(QTableWidget *table)
{
    if (!table) return;
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
}
void LStatic::removeAllRowsTable(QTableWidget *table)
{
    if (!table) return;
    int n = table->rowCount();
    for (int i=0; i<n; i++)	table->removeRow(0);
    table->setRowCount(0);
}
void LStatic::shiftTableRow(QTableWidget *table, int row_index, int shift)
{
    if (!table || shift == 0 || row_index < 0 || row_index >= table->rowCount()) return;
    int new_index = row_index + shift;

    QList<QTableWidgetItem*> save_items;
    for (int j=0; j<table->columnCount(); j++)
	save_items.append(table->takeItem(row_index, j));
    table->removeRow(row_index);


    if (new_index < 0) new_index = 0;
    if (new_index > table->rowCount()) new_index = table->rowCount();
    table->insertRow(new_index);

    for (int j=0; j<table->columnCount(); j++)
	table->setItem(new_index, j, save_items.at(j));
}
/*
void LStatic::outTime(QString s)
{
    if (s.isEmpty()) qDebug()<<curTime();
    else qDebug()<<QString("%1..........%2").arg(s).arg(curTime());
}
*/
QString LStatic::curTime()
{
    return QTime::currentTime().toString("hh:mm:ss.zzz");
}
QList<int> LStatic::selectedRows(QTableWidget *table)
{
    QList<int> list;
    if (!table) return list;

    QList<QTableWidgetItem*> items(table->selectedItems());
    for (int i=0; i<items.count(); i++)
    {
    	if (!list.contains(items.at(i)->row()))
	list.append(items.at(i)->row());
    }

    return list;
}
QList<int> LStatic::selectedCols(QTableWidget *table)
{
    QList<int> list;
    if (!table) return list;

    QList<QTableWidgetItem*> items(table->selectedItems());
    for (int i=0; i<items.count(); i++)
    {
	int col = items.at(i)->column();
    	if (!list.contains(col))
	    list.append(col);
    }

    return list;
}
int LStatic::subCount(const QString &s, const QString sub_s)
{
    int n = 0;
    int start_pos = 0;
    for (;;)
    {
        int pos = s.indexOf(sub_s, start_pos);
        if (pos < 0) break;

        n++;
        start_pos = pos + 1;
        if (start_pos >= s.length()) break;
    }
    return n;
}
int LStatic::strIndexOfByEnd(const QString &s, const QString sub_s)
{
    int last_pos = -1;
    int start_pos = 0;            
    for (;;)
    {
        int pos = s.indexOf(sub_s, start_pos);
        if (pos < 0) break;

        last_pos = pos;
        start_pos = pos + 1;
        if (start_pos >= s.length()) break;
    }
    return last_pos;
}
QString LStatic::fromColor(const QColor &color, QString split_symbol)
{
    return QString("(%1%2 %3%4 %5)").arg(color.red()).arg(split_symbol).arg(color.green()).arg(split_symbol).arg(color.blue());
}
QColor LStatic::strToColor(const QString &str, QString split_symbol)
{
    qDebug()<<QString("str");
    QColor color(0, 0, 0);

    int pos1 = str.indexOf("(");
    int pos2 = str.indexOf(")");
    if (pos1 < 0 || pos1 >= pos2) return color;

    qDebug("2");
    QString s = str.mid(pos1+1, pos2-pos1-1).trimmed();
    QStringList list = s.split(split_symbol);
    if (list.count() < 3) return color;
    
    qDebug()<<s;
    qDebug("3");
    bool ok;
    int r = list.at(0).toInt(&ok);
    if (!ok || r < 0 || r > 255) return color;
    int g = list.at(1).toInt(&ok);
    if (!ok || g < 0 || g > 255) return color;
    int b = list.at(2).toInt(&ok);
    if (!ok || b < 0 || b > 255) return color;

    qDebug()<<QString("r=%1  g=%2  b=%3").arg(r).arg(g).arg(b);

    return QColor(r, g, b);
}
QString LStatic::strTrimLeft(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.right(s.length()-n);
}
QString LStatic::strTrimRight(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.left(s.length()-n);
}
QString LStatic::strBetweenStr(const QString &s, QString s1, QString s2)
{
    if (s.isEmpty() || s1.isEmpty() || s2.isEmpty()) return QString();
    int pos1 = s.indexOf(s1);
    int pos2 = s.indexOf(s2);
    if (pos1 < 0 || pos2 < 0 || pos2 <= pos1) return QString();
    return s.mid(pos1+1, pos2-pos1-1);
}
QString LStatic::fromCodec(const QString &s, QString codec_name)
{
    if (!codec_name.isEmpty())
    {
        QTextCodec *codec = QTextCodec::codecForName(codec_name.toLatin1());
        if (codec) return codec->toUnicode(s.toLatin1());
        else qWarning("err codec");
    }
    return s;
}
void LStatic::setAttrNode(QDomElement &node, QString a1, QString v1, QString a2, QString v2, QString a3, QString v3, QString a4, QString v4, QString a5, QString v5)
{
    if (node.isNull()) return;
    if (a1.trimmed().isEmpty()) return;
    node.setAttribute(a1.trimmed(), v1.trimmed());
    if (a2.trimmed().isEmpty()) return;
    node.setAttribute(a2.trimmed(), v2.trimmed());
    if (a3.trimmed().isEmpty()) return;
    node.setAttribute(a3.trimmed(), v3.trimmed());
    if (a4.trimmed().isEmpty()) return;
    node.setAttribute(a4.trimmed(), v4.trimmed());
    if (a5.trimmed().isEmpty()) return;
    node.setAttribute(a5.trimmed(), v5.trimmed());
}
int LStatic::getIntAttrValue(const QString &attr_name, const QDomNode &node, int defValue)
{
    if (attr_name.trimmed().isEmpty() || node.isNull()) return defValue;
    if (!node.toElement().hasAttribute(attr_name.trimmed()))  return defValue;
    QString s_value = node.toElement().attribute(attr_name.trimmed());

    bool ok;
    int value = s_value.toInt(&ok);
    if (!ok) return defValue;
    return value;
}
QString LStatic::getStringAttrValue(const QString &attr_name, const QDomNode &node, QString defValue)
{
    if (attr_name.trimmed().isEmpty() || node.isNull()) return defValue;
    if (!node.toElement().hasAttribute(attr_name.trimmed()))  return defValue;
    return node.toElement().attribute(attr_name.trimmed());
}
void LStatic::createDomHeader(QDomDocument &dom)
{
    dom.appendChild(dom.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
}



