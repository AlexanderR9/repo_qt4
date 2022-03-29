 #include "generaldatawidget.h"
 #include "lcommonsettings.h"
 #include "lstatic.h"
 #include "cfdenums.h"
 #include "generaldatadialog.h"

 #include <QPushButton>
 #include <QDialogButtonBox>
 #include <QMessageBox>
 #include <QDir>
 #include <QFile>
 #include <QDebug>
 #include <QDomElement>
 #include <QAction>
 #include <QContextMenuEvent>
 #include <QMenu>
 #include <QTextStream>
 #include <QTextCodec>
 #include <QScrollBar>
 #include <QTableWidgetItem>
 #include <QPixmap>
 #include <QColor>



/////////// ConfiguratorGeneralDataWdiget /////////////////////////////
ConfiguratorGeneralDataWdiget::ConfiguratorGeneralDataWdiget(QWidget *parent)
	:QWidget(parent),
	 m_search(NULL),
	 m_sortOrder(0)
{
	setupUi(this);
	//if (!configuratorSettings.widgetsState.generalData.isEmpty()) restoreGeometry(configuratorSettings.widgetsState.generalData);
	//else resize(800,600);

	searchLineEdit->clear();
	listWidget->clear();
	LStatic::fullClearTable(tableWidget);
	tableWidget->verticalHeader()->hide();

	m_search = new LSearch(searchLineEdit, this);
	m_search->addTable(tableWidget, countLabel);
	m_search->exec();

	init();
	createContextActions();


	connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotTableChanged(QListWidgetItem*)));
	connect(tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(slotSort(int)));
	connect(tableWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrolling()));
	connect(tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(slotContextEdit()));

}
void ConfiguratorGeneralDataWdiget::slotScrolling()
{
	LStatic::resizeTableContents(tableWidget);
}
/*
void ConfiguratorGeneralDataWdiget::slotShiftRows()
{
	if (!sender()) return;

	QList<int> sel_rows = LStatic::selectedRows(tableWidget);
	if (sel_rows.isEmpty()) return;

	int move_pos = 0;
	if (sender()->objectName() == QString("button_up")) move_pos = -1;
	else if (sender()->objectName() == QString("button_down")) move_pos = 1;
	if (move_pos == 0) return;


	disconnect(tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotTableSelectionChanged()));

	tableWidget->clearSelection();
	int n = tableWidget->rowCount();

	for (int i=0; i<n; i++)
	{
	    int cur_index = (move_pos > 0) ? n-i-1 : i;
	    if (sel_rows.contains(cur_index))
	    {
		m_data.records.move(cur_index, cur_index+move_pos);
		LStatic::shiftTableRow(tableWidget, cur_index, move_pos);
	    }
	}

	LStatic::resizeTableContents(tableWidget);

	tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
	for (int i=0; i<sel_rows.count(); i++)
		tableWidget->selectRow(sel_rows.at(i) + move_pos);
	tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	slotTableSelectionChanged();

	connect(tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotTableSelectionChanged()));
}
*/
/*
void ConfiguratorGeneralDataWdiget::slotTableSelectionChanged()
{
	QList<int> sel_rows = LStatic::selectedRows(tableWidget);
	if (sel_rows.isEmpty())
	{
		buttonBox->button(QDialogButtonBox::Yes)->setEnabled(false);
		buttonBox->button(QDialogButtonBox::No)->setEnabled(false);
		return;
	}

	buttonBox->button(QDialogButtonBox::Yes)->setEnabled(!sel_rows.contains(0));
	buttonBox->button(QDialogButtonBox::No)->setEnabled(!sel_rows.contains(tableWidget->rowCount()-1));
}
*/
void ConfiguratorGeneralDataWdiget::createContextActions()
{
	int type = catAdd;
	QAction *act = new QAction(ConfiguratorEnums::interfaceTextByType(type), 0);
	connect(act, SIGNAL(triggered()), this, SLOT(slotContextAdd()));
	m_contextActions.insert(type, act);

	type = catEdit;
	act = new QAction(ConfiguratorEnums::interfaceTextByType(type), 0);
	connect(act, SIGNAL(triggered()), this, SLOT(slotContextEdit()));
	m_contextActions.insert(type, act);

	type = catClone;
	act = new QAction(ConfiguratorEnums::interfaceTextByType(type), 0);
	connect(act, SIGNAL(triggered()), this, SLOT(slotContextClone()));
	m_contextActions.insert(type, act);

	type = catDelete;
	act = new QAction(ConfiguratorEnums::interfaceTextByType(type), 0);
	connect(act, SIGNAL(triggered()), this, SLOT(slotContextDelete()));
	m_contextActions.insert(type, act);

}
bool ConfiguratorGeneralDataWdiget::visibleContextType(int context_type) const
{
	switch (context_type)
	{
		//case catEditFilling: return m_data.fields.contains(ftFilling);
		default: break;
	}

	return true;
}
void ConfiguratorGeneralDataWdiget::contextMenuEvent(QContextMenuEvent *event)
{
	if (m_data.invalid()) return;

	int x = event->pos().x();
	int y = event->pos().y();
	int x_min = tableWidget->pos().x() + tableBox->pos().x();
	int y_min = tableWidget->pos().y() + tableBox->pos().y();
	int x_max = x_min + tableWidget->width();
	int y_max = y_min + tableWidget->height();

	//qDebug()<<QString("ConfiguratorGeneralDataWdiget::contextMenuEvent - event->pos  x/y = %1/%2   tableWidget->pos x/y = %3/%4").arg(x).arg(y).arg(x_min).arg(y_min);
	if (x < x_min || x > x_max) return;
	if (y < y_min || y > y_max) return;

    QMenu menu(this);
    QMap<int, QAction*>::const_iterator it = m_contextActions.constBegin();
    while (it != m_contextActions.constEnd())
    {
    	if (visibleContextType(it.key()))
    		menu.addAction(it.value());
    	it++;
    }
    menu.exec(event->globalPos());

}
void ConfiguratorGeneralDataWdiget::init()
{
	m_data.reset();
	//if (configuratorSettings.path.generalData.isEmpty())
	QString path = lCommonSettings.paramValue("datapath").toString().trimmed();
	if (path.isEmpty())
	{
		QMessageBox::critical(this, MSGBOX_ERR_TITLE, QObject::tr("General data path is empty, \n set path in application settings."));
		enableControls(false);
		return;
	}

	QDir dir(path);
	if (!dir.exists())
	{
		QMessageBox::critical(this, MSGBOX_ERR_TITLE, QObject::tr("General data path not exist, \n set valid path in application settings."));
		enableControls(false);
		return;
	}

	initListWidget();
	//initButtonBox();
}
void ConfiguratorGeneralDataWdiget::initListWidget()
{
	QList<int> list = ConfiguratorEnums::generalDataTypes();
	for (int i=0; i<list.count(); i++)
	{
		if (GeneralDataFileReader::xmlFileByType(list.at(i)).isEmpty()) continue;
		QString s = ConfiguratorEnums::interfaceTextByType(list.at(i));
		if (s == ConfiguratorEnums::errorType()) continue;
		QListWidgetItem *item = new QListWidgetItem(s, 0, list.at(i));
		listWidget->addItem(item);
	}

	tableListBox->setTitle(QObject::tr("Tables (%1)").arg(listWidget->count()));
}
void ConfiguratorGeneralDataWdiget::enableControls(bool b)
{
    searchLineEdit->setEnabled(b);
    //listWidget->setEnabled(b);
    tableWidget->setEnabled(b);
}
void ConfiguratorGeneralDataWdiget::loadDataToTable()
{
	QStringList s_list;

	// create headers
	for (int i=0; i<m_data.fields.count(); i++)
		s_list.append(ConfiguratorEnums::interfaceTextByType(m_data.fields.at(i)));
	LStatic::setTableHeaders(tableWidget, s_list);

	//create rows
	s_list.clear();
	for (int j=0; j<m_data.fields.count(); j++)
		s_list.append(QString());

	for (int i=0; i<m_data.count(); i++)
	{
		LStatic::addTableRow(tableWidget, s_list);
		updateTableRow(i, m_data.records.at(i));
	}

	updateTable();
}
void ConfiguratorGeneralDataWdiget::updateTableRow(int row, const ConfiguratorAbstractRecord &rec)
{
	if (row < 0 || row >= tableWidget->rowCount()) return;

	QPixmap pixmap(16, 16);


	for (int i=0; i<m_data.fields.count(); i++)
	{
		int ft = m_data.fields.at(i);
		if (ConfiguratorEnums::isFixField(ft))
		{
		    int g_type =  ConfiguratorEnums::generalTypeByFieldType(ft);
		    if (g_type > 0)
		    {
		    	ConfiguratorAbstractData g_data;
		    	GeneralDataFileReader::loadDataFormFile(g_type, g_data);
		    	if (!g_data.invalid())
			{
			    int rec_value = rec.value(ft).toInt();
			    if (rec_value == 0) tableWidget->item(row, i)->setText(ConfiguratorEnums::interfaceTextByType(0));
			    else
			    {
				const ConfiguratorAbstractRecord *g_rec = g_data.recByFieldValue(ftID, rec.value(ft));
				if (!g_rec) tableWidget->item(row, i)->setText(ConfiguratorEnums::errorType());
				else 
				{
				    if (ft == ftCouponCount) tableWidget->item(row, i)->setText(g_rec->value(ftCount));
				    else tableWidget->item(row, i)->setText(g_rec->value(ftName));
				}
			    }
			}
		    }
		    else tableWidget->item(row, i)->setText(ConfiguratorEnums::interfaceTextByType(rec.value(ft).toInt()));
		}
		else
		{
			tableWidget->item(row, i)->setText(rec.value(ft));
			if (ft == ftColor)
			{
				QColor c(rec.value(ftColor));
				if (c.isValid())
				{
					pixmap.fill(c);
					tableWidget->item(row, i)->setIcon(QIcon(pixmap));
				}
			}
		}
	}
}
void ConfiguratorGeneralDataWdiget::updateTable()
{
    tableWidget->resizeColumnsToContents();
    tableWidget->resizeRowsToContents();
    if (m_search) m_search->exec();
}
void ConfiguratorGeneralDataWdiget::slotTableChanged(QListWidgetItem *item)
{
	if (!item) return;

	LStatic::fullClearTable(tableWidget);
	m_data.reset();

	QString fname = GeneralDataFileReader::xmlFileByType(item->type()).trimmed();
	if (fname.isEmpty())
	{
		QMessageBox::critical(this, MSGBOX_ERR_TITLE, QObject::tr("Invalid general data type."));
		enableControls(false);
		return;
	}

	fname = QString("%1%2%3").arg(lCommonSettings.paramValue("datapath").toString()).arg(QDir::separator()).arg(fname);
	QFile f(fname);
	if (!f.exists())
	{
		QMessageBox::critical(this, MSGBOX_WARNING_TITLE, QObject::tr("File data not found - %1").arg(fname));
		m_data.fields.append(ftID);
		m_data.generalType = item->type();
	}
	else GeneralDataFileReader::loadDataFormFile(item->type(), m_data);

	if (m_data.invalid())
	{
		QMessageBox::critical(0, MSGBOX_ERR_TITLE, QObject::tr("Incorrect file data - %1").arg(f.fileName()));
		enableControls(false);
		return;
	}

	enableControls(true);
	loadDataToTable();
}
/*
void ConfiguratorGeneralDataWdiget::slotEditRecord()
{

}
*/
void ConfiguratorGeneralDataWdiget::slotSort(int col)
{
	tableWidget->sortItems(col, Qt::SortOrder(m_sortOrder));
	m_sortOrder = (m_sortOrder != 0) ? 0 : 1;
}
void ConfiguratorGeneralDataWdiget::slotApply()
{
    if (m_data.invalid())
    {
	QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("Not selected table data"));
	return;
    }

    QString question = QObject::tr("Are you sure want rewrite file %1?").arg(GeneralDataFileReader::xmlFileByType(m_data.generalType));
    if (QMessageBox::question(this, MSGBOX_REWRITEFILE_TITLE, question, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
    	QString err;
	GeneralDataFileReader::writeDataToFile(m_data, err);
	if (!err.isEmpty())	qWarning()<<err;
    }
}
void ConfiguratorGeneralDataWdiget::closeEvent(QCloseEvent*)
{
//	configuratorSettings.widgetsState.generalData = saveGeometry();

}




//context slots
void ConfiguratorGeneralDataWdiget::slotContextAdd()
{
	ConfiguratorAbstractRecord rec;
	m_data.setEmptyRecord(rec);
	GeneralDataRecordDialog dialog(catAdd, rec);
	dialog.exec();
	if (!dialog.isApply()) return;

	if (rec.value(ftID).toInt() <= 0)
		rec.record.insert(ftID, QString::number(m_data.maxID()+1));

	for (int i=0; i<m_data.fields.count(); i++)
	{
		if (!ConfiguratorEnums::isUniqueField(m_data.fields.at(i))) continue;
		if (m_data.hasFieldValue(m_data.fields.at(i), rec.value(m_data.fields.at(i))))
		{
			QString msg = QObject::tr("Field <%1>: value <%2> already exists in the table.").arg(ConfiguratorEnums::interfaceTextByType(m_data.fields.at(i))).arg(rec.value(m_data.fields.at(i)));
			QMessageBox::warning(this, MSGBOX_WARNING_TITLE, msg);
			return;
		}
	}

	QStringList row_list;
	rec.getValues(m_data.fields, row_list);
	LStatic::addTableRow(tableWidget, row_list);
	m_data.records.append(rec);
	updateTableRow(m_data.count()-1, rec);
	updateTable();

}
void ConfiguratorGeneralDataWdiget::slotContextEdit()
{
	QList<int> sel_rows = LStatic::selectedRows(tableWidget);
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

	int pos = m_data.indexOf(ftID, tableWidget->item(sel_rows.first(), 0)->text());
	if (pos < 0)
	{
		QMessageBox::critical(this, MSGBOX_ERR_TITLE, QObject::tr("Invalid record selected"));
		return;
	}

	ConfiguratorAbstractRecord rec;
	rec.copy(m_data.records.at(pos));
	GeneralDataRecordDialog dialog(catEdit, rec);
	dialog.exec();
	if (!dialog.isApply()) return;

	for (int i=0; i<m_data.fields.count(); i++)
	{
		if (!ConfiguratorEnums::isUniqueField(m_data.fields.at(i))) continue;
		if (m_data.hasFieldValue(m_data.fields.at(i), rec.value(m_data.fields.at(i)), pos))
		{
			QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("The value of the <%1> field must be unique").arg(ConfiguratorEnums::interfaceTextByType(m_data.fields.at(i))));
			return;
		}
	}

	m_data.records[pos].copy(rec);
	updateTableRow(sel_rows.first(), rec);
	updateTable();
}
void ConfiguratorGeneralDataWdiget::slotContextClone()
{
	QList<int> sel_rows = LStatic::selectedRows(tableWidget);
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

	int pos = m_data.indexOf(ftID, tableWidget->item(sel_rows.first(), 0)->text());
	if (pos < 0)
	{
		QMessageBox::critical(this, MSGBOX_ERR_TITLE, QObject::tr("Invalid record selected"));
		return;
	}

	ConfiguratorAbstractRecord rec;
	rec.copy(m_data.records.at(pos));
	rec.record.insert(ftID, QString::number(-1));
	GeneralDataRecordDialog dialog(catClone, rec);
	dialog.exec();
	if (!dialog.isApply()) return;

	if (rec.value(ftID).toInt() <= 0)
		rec.record.insert(ftID, QString::number(m_data.maxID()+1));

	for (int i=0; i<m_data.fields.count(); i++)
	{
		if (!ConfiguratorEnums::isUniqueField(m_data.fields.at(i))) continue;
		if (m_data.hasFieldValue(m_data.fields.at(i), rec.value(m_data.fields.at(i))))
		{
			QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("The value of the <%1> field must be unique").arg(ConfiguratorEnums::interfaceTextByType(m_data.fields.at(i))));
			return;
		}
	}

	QStringList row_list;
	rec.getValues(m_data.fields, row_list);
	LStatic::addTableRow(tableWidget, row_list);
	m_data.records.append(rec);
	updateTableRow(m_data.count()-1, rec);
	updateTable();

}
void ConfiguratorGeneralDataWdiget::slotContextDelete()
{
	QList<int> sel_rows = LStatic::selectedRows(tableWidget);
	if (sel_rows.isEmpty())
	{
		QMessageBox::warning(this, MSGBOX_WARNING_TITLE, QObject::tr("Not selected table data"));
		return;
	}

	QString question = QObject::tr("Are you sure you want to delete %1 records").arg(sel_rows.count());
	if (QMessageBox::question(this, MSGBOX_DELETEDATA_TITLE, question, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
	{
		for (int i=sel_rows.count(); i>0; i--)
		{
			tableWidget->removeRow(sel_rows.at(i-1));
			m_data.records.removeAt(sel_rows.at(i-1));
		}
	}
}



////////////////GeneralDataFileReader///////////////////////////////////////////
QString GeneralDataFileReader::xmlFileByType(int type)
{
    return ConfiguratorEnums::xmlFileByType(type);
}
void GeneralDataFileReader::loadDataFormFile(int type, ConfiguratorAbstractData &data)
{
	data.reset();

	QString fname = xmlFileByType(type).trimmed();
	if (fname.isEmpty())
	{
		qWarning()<<QString("invalid data type %1").arg(type);
		return;
	}

	QFile f(QString("%1%2%3").arg(lCommonSettings.paramValue("datapath").toString()).arg(QDir::separator()).arg(fname));
	if (!f.exists())
	{
		qWarning()<<QString("file data not found %1").arg(f.fileName());
		return;
	}

	QDomDocument dom;
	if (!dom.setContent(&f))
	{
		f.close();
		return;
	}
	f.close();

	QDomNode rootNode(dom.namedItem(ConfiguratorEnums::xmlRootNodeName()));
	if (rootNode.isNull())
	{
		qWarning("not found root node");
		return;
	}

	QDomNode fieldsNode(rootNode.namedItem(ConfiguratorEnums::xmlFieldsNodeName()));
	if (fieldsNode.isNull())
	{
		qWarning("not found fields node");
		return;
	}

	QDomNode recordsNode(rootNode.namedItem(ConfiguratorEnums::xmlRecordsNodeName()));
	if (recordsNode.isNull())
	{
		qWarning("not found records node");
		return;
	}


	//parse nodes
	loadFields(fieldsNode, data);
	loadRecords(recordsNode, data, type);

	//if (data.fields.contains(ftID))
	data.generalType = type;

}
void GeneralDataFileReader::loadFields(const QDomNode &node, ConfiguratorAbstractData &data)
{
	data.fields.clear();
	QList<int> all_fields = ConfiguratorEnums::fieldTypes();

	QDomNode childNode = node.firstChild();
	while(!childNode.isNull())
	{
		if (childNode.nodeName() == ConfiguratorEnums::xmlFieldNodeName())
		{
			int id = LStatic::getIntAttrValue(ConfiguratorEnums::xmlAttrNameByType(ftID), childNode);
			if (all_fields.contains(id)) data.fields.append(id);
		}
		childNode = childNode.nextSibling();
	}
}
void GeneralDataFileReader::loadRecords(const QDomNode &node, ConfiguratorAbstractData &data, int type)
{
	if (data.fields.isEmpty()) return;
	QDomNode childNode = node.firstChild();
	while(!childNode.isNull())
	{
		if (childNode.nodeName() == ConfiguratorEnums::xmlNodeNameByType(type))
		{
			ConfiguratorAbstractRecord rec;
			data.setEmptyRecord(rec);
			loadRecord(childNode.toElement(), rec);
			if (!rec.invalid()) data.records.append(rec);
		}

		childNode = childNode.nextSibling();
	}
}
void GeneralDataFileReader::loadRecord(const QDomElement &el, ConfiguratorAbstractRecord &rec)
{
	bool ok;
	QList<int> fields = rec.record.keys();
	if (fields.isEmpty()) {fields = ConfiguratorEnums::fieldTypes();}
	for (int i=0; i<fields.count(); i++)
	{
		QString attr_name = ConfiguratorEnums::xmlAttrNameByType(fields.at(i));
		if (!el.hasAttribute(attr_name)) continue;
		QString value = LStatic::getStringAttrValue(attr_name, el, QString());
		if (value.trimmed().isEmpty()) continue;

		if (ConfiguratorEnums::isIntField(fields.at(i)))
		{
			value.toInt(&ok);
			if (!ok)
			{
				qWarning()<<QString("ConfiguratorGeneralDataWdiget::loadRecord - %1=%2 invalid integer value").arg(attr_name).arg(value);
				continue;
			}
		}

		rec.record.insert(fields.at(i), value);
	}

/*
	if (!rec.record.contains(ftID))
	{
		rec.reset();
		return;
	}
*/

}
void GeneralDataFileReader::addRecordFieldsToNode(const ConfiguratorAbstractRecord &rec, QDomElement &node)
{
	if (node.isNull()) return;

	QList<int> fields = rec.fields();
	for (int i=0; i<fields.count(); i++)
	{
		QString attr_name = ConfiguratorEnums::xmlAttrNameByType(fields.at(i)).trimmed();
		if (!attr_name.isEmpty() && attr_name != ConfiguratorEnums::errorType())
		{
			LStatic::setAttrNode(node, attr_name, rec.value(fields.at(i)));
		}
	}
}
void GeneralDataFileReader::fillRecordFieldsFromNode(ConfiguratorAbstractRecord &rec, const QDomNode &node)
{
	if (node.isNull()) return;

	QList<int> fields = ConfiguratorEnums::fieldTypes();
	for (int i=0; i<fields.count(); i++)
	{
		QString attr_name = ConfiguratorEnums::xmlAttrNameByType(fields.at(i)).trimmed();
		if (attr_name.isEmpty()) continue;
		if (!node.toElement().hasAttribute(attr_name)) continue;
		rec.record.insert(fields.at(i), LStatic::getStringAttrValue(attr_name, node));
	}
}
void GeneralDataFileReader::writeDataToFile(const ConfiguratorAbstractData &data, QString &err)
{
	err.clear();

	QString fname = xmlFileByType(data.generalType).trimmed();
	if (fname.isEmpty())
	{
		err = QObject::tr("Invalid data type %1").arg(data.generalType);
		return;
	}

	QFile f(QString("%1%2%3").arg(lCommonSettings.paramValue("datapath").toString()).arg(QDir::separator()).arg(fname));

	//fill dom
	QDomDocument dom;
	LStatic::createDomHeader(dom);
	QDomNode rootNode = dom.createElement(ConfiguratorEnums::xmlRootNodeName());
	QDomNode fieldsNode = dom.createElement(ConfiguratorEnums::xmlFieldsNodeName());
	QDomNode recordsNode = dom.createElement(ConfiguratorEnums::xmlRecordsNodeName());
	dom.appendChild(rootNode);
	rootNode.appendChild(fieldsNode);
	rootNode.appendChild(recordsNode);

	//fill nodes
	writeFields(fieldsNode, data);
	writeRecords(recordsNode, data);

	//write file
    if (!f.open(QIODevice::WriteOnly))
    {
		err = QObject::tr("Error open file for writing %1").arg(f.fileName());
        return ;
    }

    QTextStream stream(&f);
    stream.setCodec("UTF-8");
    stream << dom.toString();
    f.close();

}
void GeneralDataFileReader::writeFields(QDomNode &node, const ConfiguratorAbstractData &data)
{
	if (node.isNull()) return;

	QDomDocument dom;
	for (int i=0; i<data.fields.count(); i++)
	{
		QDomElement el = dom.createElement(ConfiguratorEnums::xmlFieldNodeName());
		LStatic::setAttrNode(el, ConfiguratorEnums::xmlAttrNameByType(ftID), QString::number(data.fields.at(i)));
		node.appendChild(el);
	}
}
void GeneralDataFileReader::writeRecords(QDomNode &node, const ConfiguratorAbstractData &data)
{
	if (node.isNull()) return;

	QDomDocument dom;
	for (int i=0; i<data.count(); i++)
	{
		QDomElement el = dom.createElement(ConfiguratorEnums::xmlNodeNameByType(data.generalType));
		writeRecord(el, data.records.at(i));
		node.appendChild(el);
	}
}
void GeneralDataFileReader::writeRecord(QDomElement &el, const ConfiguratorAbstractRecord &rec)
{
	QList<int> fields = rec.record.keys();
	for (int i=0; i<fields.count(); i++)
	{
		switch (fields.at(i))
		{
			default:
			{
				LStatic::setAttrNode(el, ConfiguratorEnums::xmlAttrNameByType(fields.at(i)), rec.value(fields.at(i)));
				break;
			}
		}
	}
}




