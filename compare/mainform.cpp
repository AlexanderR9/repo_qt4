 #include "mainform.h"
 #include "lstatic.h"
 #include "lfile.h"
 #include "lcommonsettings.h"

 #include <QDebug>
 #include <QtCore/qmath.h>
 #include <QSettings>
 #include <QSplitter>
#include <QFileInfo>
#include <QDir>


//////// MainForm ////////////////////////
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_centralWidget(NULL)
{
    setObjectName("main_form");
    move(150, 50);
    resize(800, 500);

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atSScale);
    addAction(LMainWidget::atRemove);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
	m_centralWidget = new MainWidget(this);
	this->addWidget(m_centralWidget, 0, 0);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=1; i<=6; i++)
        combo_list.append(QString::number(i));

    QString key = QString("precision");
    lCommonSettings.addParam(QString("Precision values"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, 2);

    key = "host1_rpmqa";
    lCommonSettings.addParam(QString("host1 path"), LSimpleDialog::sdtFilePath, key);
    key = "host2_rpmqa";
    lCommonSettings.addParam(QString("host2 path"), LSimpleDialog::sdtFilePath, key);


    key = "host1_dir";
    lCommonSettings.addParam(QString("host1 dir of struct files"), LSimpleDialog::sdtDirPath, key);
    key = "host2_dir";
    lCommonSettings.addParam(QString("host2 dir of struct files"), LSimpleDialog::sdtDirPath, key);


    key = "find_type";
    lCommonSettings.addParam(QString("What looking for"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Only folders" << "Only files";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, 2);

    key = "is_bin";
    lCommonSettings.addParam(QString("Binary files"), LSimpleDialog::sdtBool, key);


}
void MainForm::load()
{
    LMainWidget::load();
    QSettings settings(companyName(), projectName());
    m_centralWidget->load(settings);
}
void MainForm::save()
{
    LMainWidget::save();
    QSettings settings(companyName(), projectName());
    m_centralWidget->save(settings);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {startCompare(); break;}
        case LMainWidget::atLoadData: {loadData(); break;}
        case LMainWidget::atRemove: {m_centralWidget->removeRows(); break;}
        case LMainWidget::atSScale: {getDirStructs(); break;}



        default: break;
    }
}
void MainForm::startCompare()
{
    m_centralWidget->compareRPMS();
}
void MainForm::loadData()
{
	QString path2 = lCommonSettings.paramValue("host1_rpmqa").toString().trimmed();
	QString path3 = lCommonSettings.paramValue("host2_rpmqa").toString().trimmed();
	m_centralWidget->setBinary(lCommonSettings.paramValue("is_bin").toBool());
	m_centralWidget->readFiles(path2, path3);
}
void MainForm::getDirStructs()
{
	QString path2 = lCommonSettings.paramValue("host1_dir").toString().trimmed();
	QString path3 = lCommonSettings.paramValue("host2_dir").toString().trimmed();
	bool is_files = lCommonSettings.paramValue("find_type").toString().contains("file");
	m_centralWidget->getDirStructs(path2, path3, is_files);
}


//MainWidget
MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent),
	 m_search2(NULL),
	 m_search3(NULL),
	h_splitter(NULL),
    is_binary(false)
{
    setupUi(this);

    if (layout()) delete layout();
    setLayout(new QHBoxLayout(0));


    h_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(h_splitter);
    h_splitter->addWidget(box2);
    h_splitter->addWidget(box3);

    m_search2 = new LSearch(lineEdit2, this);
    m_search2->addTable(table2, label2);
    m_search3 = new LSearch(lineEdit3, this);
    m_search3->addTable(table3, label3);



    initTable();
}
void MainWidget::initTable()
{
    LStatic::fullClearTable(table2);
    LStatic::fullClearTable(table3);
    LStatic::setTableHeaders(table2, headerLabels(), Qt::Horizontal);
    LStatic::setTableHeaders(table3, headerLabels(), Qt::Horizontal);
	table2->hideColumn(2);
	table3->hideColumn(2);

}
QStringList MainWidget::headerLabels()
{
    QStringList list;
    list.append("N");
    list.append("Value");
    list.append("Size, b");
    return list;
}
void MainWidget::compareBin()
{
    if (host1_rpms.isEmpty() || host2_rpms.isEmpty()) return;
    int n = ((host1_rpms.count() > host2_rpms.count()) ? host1_rpms.count() : host2_rpms.count());

    int n_err = 0;
    for (int i=0; i<n; i++)
    {
	if (host1_rpms.at(i) != host2_rpms.at(i))
	{
	    table2->item(i, 1)->setTextColor(Qt::red);
	    n_err++;
	}
    }

    QString label_text = QString("Records/diffs: %1/%2").arg(n).arg(n_err);
    label2->setText(label_text);

}
void MainWidget::compareRPMS()
{
	if (is_binary) {compareBin(); return;}
	compareRPMS(host1_rpms, host2_rpms, table2);
	compareRPMS(host2_rpms, host1_rpms, table3);
}
void MainWidget::compareRPMS(const QStringList &own, const QStringList &other, QTableWidget *own_table)
{
	LStatic::removeAllRowsTable(own_table);
	int n1 = 0;
	int n2 = 0;
	for (int i=0; i<own.count(); i++)
	{
		QString s = own.at(i).trimmed();
		if (s.isEmpty()) continue;
		if (other.contains(s)) continue;

		n1++;
		QStringList row;
		row << QString::number(n1) << s << QString();
		LStatic::addTableRow(own_table, row, Qt::AlignCenter, Qt::blue);
	}
	for (int i=0; i<other.count(); i++)
	{
		QString s = other.at(i).trimmed();
		if (s.isEmpty()) continue;
		if (own.contains(s)) continue;

		n2++;
		QStringList row;
		row << QString::number(n2) << s << QString();
		LStatic::addTableRow(own_table, row, Qt::AlignCenter, Qt::red);
	}

	QString label_text = QString("Exist/No: %1/%2").arg(n1).arg(n2);
	if (own_table->objectName().contains("2")) label2->setText(label_text);
	else label3->setText(label_text);

	LStatic::resizeTableContents(own_table);
	own_table->setColumnWidth(0, 100);

}
void MainWidget::readFiles(QString f2, QString f3)
{
	readFile(f2, host1_rpms);
	readFile(f3, host2_rpms);
	fillTable(host1_rpms, table2);
	fillTable(host2_rpms, table3);
	table2->hideColumn(2);
	table3->hideColumn(2);
}
void MainWidget::getDirStructs(QString path2, QString path3, bool is_files)
{
	getFolders(path2.trimmed(), host1_rpms);
	getFolders(path3.trimmed(), host2_rpms);
	if (is_files)
	{
		getFiles(path2, host1_rpms);
		getFiles(path3, host2_rpms);
	}
	fillTable(host1_rpms, table2);
	fillTable(host2_rpms, table3);
	if (is_files)
	{
		setFileSizeColumn(path2, table2);
		setFileSizeColumn(path3, table3);
	}

	qDebug()<<QString("done");
}
void MainWidget::setFileSizeColumn(const QString &root_path, QTableWidget *t)
{
	t->showColumn(2);

	int n = t->rowCount();
	for (int i=0; i<n; i++)
	{
		QFileInfo fi(QString("%1%2%3").arg(root_path).arg(QDir::separator()).arg(t->item(i, 1)->text()));
		if (fi.isFile()) t->item(i, 2)->setText(QString::number(fi.size()));
		else
		{
			t->item(i, 2)->setText("???");
			t->item(i, 2)->setTextColor(Qt::red);
		}
	}
}
void MainWidget::getFiles(const QString &root_path, QStringList &data)
{
	if (data.isEmpty()) return;

	QStringList dir_list(data);
	data.clear();
	for (int i=0; i<dir_list.count(); i++)
	{
		QString full_path = root_path;
		QString s = dir_list.at(i).trimmed();
		if (s == ".") s.clear();
		else full_path = QString("%1%2%3").arg(root_path).arg(QDir::separator()).arg(s);

		QFileInfo fi(full_path);
		if (!fi.isDir()) continue;


		QDir dir(full_path);
		QStringList file_childs(dir.entryList(QDir::Files));
		//qDebug()<<QString("getFiles: cur_dir [%1], file childs %2").arg(s).arg(file_childs.count());
		for (int j=0; j<file_childs.count(); j++)
		{
			QString fs = file_childs.at(j).trimmed();
			if (fs.isEmpty() || fs == "." || fs =="..") continue;
			if (!s.isEmpty())
				fs = QString("%1%2%3").arg(s).arg(QDir::separator()).arg(fs);
			QFileInfo fi(QString("%1%2%3").arg(root_path).arg(QDir::separator()).arg(fs));
			if (fi.isFile()) data.append(fs);
			else qWarning()<<QString("not file [%1]").arg(QString("%1%2%3").arg(root_path).arg(QDir::separator()).arg(fs));
		}
	}
}
void MainWidget::nextDir(const QString &root_path, QString child, QStringList &list)
{
	qDebug()<<QString("nextDir: [%1]").arg(child);
	QDir dir(QString("%1%2%3").arg(root_path).arg(QDir::separator()).arg(child));
	if (child.isEmpty()) list.append(".");

	int n = list.count();
	QStringList dir_childs(dir.entryList(QDir::Dirs));
	//qDebug()<<QString("current path: [%1],  childs dirs %2").arg(dir.path()).arg(dir_childs.count());
	for (int i=0; i<dir_childs.count(); i++)
	{
		QString s = dir_childs.at(i).trimmed();
		if (s.isEmpty() || s == "." || s =="..") continue;
		if (!child.isEmpty()) s = QString("%1%2%3").arg(child).arg(QDir::separator()).arg(s);

		QFileInfo fi(QString("%1%2%3").arg(root_path).arg(QDir::separator()).arg(s));
		if (fi.isDir()) list.append(s);
	}
	if (n == list.count()) return;

	for (int i=n; i<list.count(); i++)
		nextDir(root_path, list.at(i), list);
}
void MainWidget::getFolders(const QString &path, QStringList &data)
{
	data.clear();
	QFileInfo fi(path);
	if (!fi.isDir())
	{
		qWarning()<<QString("getFiles : ERROR path is not dir: [%1]").arg(path);
		return;
	}

	QString child_path;
	nextDir(path, child_path, data);
}


void MainWidget::fillTable(const QStringList &data, QTableWidget *table)
{
	table->verticalHeader()->hide();
	LStatic::removeAllRowsTable(table);

	int n = 0;
	for (int i=0; i<data.count(); i++)
	{
		QString s = data.at(i).trimmed();
		if (s.isEmpty()) continue;

		n++;
		QStringList row;
		row << QString::number(n) << s << QString();
		LStatic::addTableRow(table, row);
	}
	LStatic::resizeTableContents(table);
	table->setColumnWidth(0, 70);
	table->setColumnWidth(2, 70);
	if (table->objectName().contains("2")) label2->setText(QString("Count: %1").arg(n));
	else label3->setText(QString("Count: %1").arg(n));
}
void MainWidget::readBinFile(const QString &fname, QStringList &data)
{
    qDebug()<<QString("MainWidget::readBinFile - [%1]").arg(fname);

    QByteArray ba;
    QString err = LFile::readFileBA(fname, ba);
    if (!err.isEmpty())
    {
	qWarning()<<QString("ERROR host1 reading binary file: %1").arg(err);
	return;
    }
    
    data << QString("%1 bytes").arg(ba.size());
    if (ba.isEmpty()) return;
    

    int n_out = 4;
    while (!ba.isEmpty())
    {
	if (ba.size() < n_out) n_out = ba.size();
	data << LStatic::baToStr(ba.left(n_out), n_out);
	ba.remove(0, n_out);
    }
}
void MainWidget::readFile(const QString &fname, QStringList &data)
{
	data.clear();
	if (is_binary) {readBinFile(fname, data); return;}

	QString err = LFile::readFileSL(fname, data);
	if (!err.isEmpty())
	{
		qWarning()<<QString("ERROR host1 reading file: %1").arg(err);
		return;
	}

	for (int i=0; i<data.count(); i++)
	{
		QString s = data.at(i).trimmed();
		data[i] = s;
	}

}
void MainWidget::save(QSettings &settings)
{
	settings.setValue("mainform/splitter", h_splitter->saveState());
}						
void MainWidget::load(QSettings &settings)
{
	QByteArray ba = settings.value("mainform/splitter").toByteArray();
	if (!ba.isEmpty()) h_splitter->restoreState(ba);
}
void MainWidget::removeRows()
{
	removeRows(table2);
	removeRows(table3);
	m_search2->exec();
	m_search3->exec();
}
void MainWidget::removeRows(QTableWidget *t)
{
	QList<int> list(LStatic::selectedRows(t));
	if (list.isEmpty()) return;

	int n = list.count();
	for (int i=n-1; i>=0; i--)
	{
		t->removeRow(list.at(i));
	}
}




