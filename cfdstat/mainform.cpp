 #include "mainform.h"
 #include "lcommonsettings.h"
 #include "historywidget.h"
 #include "lstatic.h"
 #include "cfdenums.h"
 #include "generaldatawidget.h"
 #include "bagwidget.h"
 #include "divstatwidget.h"
 #include "operationdialog.h"
 #include "divcalc.h"

 #include <QDebug>
 #include <QTimer>
 #include <QTabWidget>
 #include <QListWidget>
 #include <QLabel>
 #include <QSplitter>
 #include <QStackedWidget>


//////// MainForm ////////////////////////
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_stackedWidget(NULL),
	m_divsCalcObj(NULL)
{
    setObjectName("cfd_main_form");

}
void MainForm::initCalcDivObj()
{
	m_divsCalcObj = new DivCalc(operationsHistoryWidget()->operationsData(), this);

	QString err;
	m_divsCalcObj->readGeneralData(err);
    if (!err.isEmpty())
    {
    	QMessageBox::critical(this, "Divs calc object", err);
    	return;
    }

    m_divsCalcObj->updateDivInfo();
    if (m_divsCalcObj->isEmpty())
    {
    	qWarning()<<QString("MainForm::initCalcObj() WARNING - m_divs data is empty!");
    	return;
    }

    connect(divStatWidget(), SIGNAL(signalGetDivsData(ConfiguratorAbstractData*&)), m_divsCalcObj, SLOT(slotSetDivsData(ConfiguratorAbstractData*&)));
    connect(divStatWidget(), SIGNAL(signalGetWaitDays(QList<quint8>&)), m_divsCalcObj, SLOT(slotGetWaitDays(QList<quint8>&)));
    connect(operationsHistoryWidget(), SIGNAL(signalGetDivsData(ConfiguratorAbstractData*&)), m_divsCalcObj, SLOT(slotSetDivsData(ConfiguratorAbstractData*&)));
    connect(bagWidget(), SIGNAL(signalGetDivsData(ConfiguratorAbstractData*&)), m_divsCalcObj, SLOT(slotSetDivsData(ConfiguratorAbstractData*&)));


}
void MainForm::initActions()
{
    addAction(LMainWidget::atData);
    addAction(LMainWidget::atBag);
    addAction(LMainWidget::atChart);
    addAction(LMainWidget::atBScale);
    addAction(LMainWidget::atSave);
    addAction(LMainWidget::atBuy);
    addAction(LMainWidget::atSell);
    addAction(LMainWidget::atRefresh);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
    load();

    //создание страниц
    m_stackedWidget = new QStackedWidget(this);
    addWidget(m_stackedWidget, 0, 0, 1, 1);
    m_stackedWidget->addWidget(new ConfiguratorGeneralDataWdiget(this)); //0
    m_stackedWidget->addWidget(new BagWidget(this));			//1	
    m_stackedWidget->addWidget(new HistoryWidget(this));		//2	
    m_stackedWidget->addWidget(new DivStatWidget(this));		//3	

    connect(m_stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotWidgetChanged(int)));
    connect(bagWidget(), SIGNAL(signalNextOperation(int, const ConfiguratorAbstractRecord&)), operationsHistoryWidget(), SLOT(slotNextOperation(int, const ConfiguratorAbstractRecord&)));
    connect(operationsHistoryWidget(), SIGNAL(signalBagUpdate(const ConfiguratorAbstractRecord&)), bagWidget(), SLOT(slotBagUpdate(const ConfiguratorAbstractRecord&)));
    connect(divStatWidget(), SIGNAL(signalGetCompanyByID(int, QString&)), operationsHistoryWidget(), SLOT(slotSetCompanyByID(int, QString&)));
    connect(divStatWidget(), SIGNAL(signalGetCurrencyByID(int, QString&)), operationsHistoryWidget(), SLOT(slotSetCurrencyByID(int, QString&)));
    connect(operationsHistoryWidget(), SIGNAL(signalBagRefreshTable()), bagWidget(), SLOT(slotBagRefreshTable()));
    connect(bagWidget(), SIGNAL(signalGetOperationsHistory(ConfiguratorAbstractData*&)), operationsHistoryWidget(), SLOT(slotSetHistoryData(ConfiguratorAbstractData*&)));

    initCalcDivObj(); // в этот момент уже должна быть инициализирована страница HistoryWidget
    updateButtons();
}
void MainForm::updateButtons()
{
    switch(m_stackedWidget->currentIndex())
    {
		case 0:
		{
			getAction(atBuy)->setEnabled(false);
			getAction(atSell)->setEnabled(false);
			getAction(atSave)->setEnabled(true);
			getAction(atRefresh)->setEnabled(false);
			getAction(atClear)->setEnabled(false);
			break;
		}
		case 1:
		{
			getAction(atBuy)->setEnabled(true);
			getAction(atSell)->setEnabled(true);
			getAction(atSave)->setEnabled(false);
			getAction(atRefresh)->setEnabled(false);
			getAction(atClear)->setEnabled(false);
			break;
		}
		case 2:
		{
			getAction(atBuy)->setEnabled(true);
			getAction(atSell)->setEnabled(true);
			getAction(atSave)->setEnabled(true);
			getAction(atRefresh)->setEnabled(true);
			getAction(atClear)->setEnabled(true);
			break;
		}
		case 3:
		{
			getAction(atBuy)->setEnabled(false);
			getAction(atSell)->setEnabled(false);
			getAction(atSave)->setEnabled(false);
			getAction(atRefresh)->setEnabled(false);
			getAction(atClear)->setEnabled(true);
			break;
		}
		default: break;
    }
}
void MainForm::slotWidgetChanged(int index)
{
    Q_UNUSED(index);
    updateButtons();
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));

    QString key = QString("precision");
    lCommonSettings.addParam(QString("Precision values"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString("2"));

    key = QString("auto_scrolling");
    lCommonSettings.addParam(QString("Auto scrolling tables"), LSimpleDialog::sdtBool, key);

    key = QString("datapath");
    lCommonSettings.addParam(QString("XML data folder"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString("data"));

    key = QString("commission");
    lCommonSettings.addParam(QString("Commission of operations, %"), LSimpleDialog::sdtDoubleLine, key, 2);
    lCommonSettings.setDefValue(key, QString("0.3"));
    key = QString("kurs");
    lCommonSettings.addParam(QString("Kurs usd/rub"), LSimpleDialog::sdtDoubleLine, key, 2);
    lCommonSettings.setDefValue(key, QString("75.3"));

    combo_list.clear();
    combo_list << "2020" << "2021" << "2022" << "2023" << "2024";
    key = QString("first_div_date");
    lCommonSettings.addParam(QString("First divs. date"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString("2020"));

    key = QString("last_div_date");
    lCommonSettings.addParam(QString("Second divs. date"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString::number(QDate::currentDate().year()));

    key = QString("table_colors");
    lCommonSettings.addParam(QString("Apply tables colors"), LSimpleDialog::sdtBool, key);

    m_appDialogCaptionWidth = 200;
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
		case LMainWidget::atSettings: {actCommonSettings(); break;}
		case LMainWidget::atSave: {saveCurrentData(); break;}
		case LMainWidget::atData: {actGeneralData(); break;}
		case LMainWidget::atChart: {actHistory(); break;}
		case LMainWidget::atBag: {actBag(); break;}
		case LMainWidget::atBScale: {actDivStat(); break;}
		case LMainWidget::atClear: {actClearQueryHistory(); break;}
		case LMainWidget::atBuy: {actBuy(); break;}
		case LMainWidget::atSell: {actSell(); break;}
		case LMainWidget::atRefresh: {actUpdatePrices(); break;}

		default: break;
    }
}
void MainForm::actClearQueryHistory()
{
	operationsHistoryWidget()->clearQuery();
}
void MainForm::actUpdatePrices()
{
    ConfiguratorAbstractRecord result_record;
    UpdatePricesDialog d(result_record, this);
    connect(&d, SIGNAL(signalGetCurrentPrice(const ConfiguratorAbstractRecord&, double&)), operationsHistoryWidget(), SLOT(slotGetCurrentPrice(const ConfiguratorAbstractRecord&, double&)));
    d.init();
    d.exec();
    
    if (d.isApply())
    {
    	operationsHistoryWidget()->setPrice(result_record);
    }
}
void MainForm::actBuy()
{
    ConfiguratorAbstractRecord result_record;
    OperationDialog d(opBuy, result_record, this);
    connect(&d, SIGNAL(signalGetCurrentPrice(const ConfiguratorAbstractRecord&, double&)), operationsHistoryWidget(), SLOT(slotGetCurrentPrice(const ConfiguratorAbstractRecord&, double&)));
    d.init();
    d.exec();
    
    if (d.isApply())
    	bagWidget()->buy(result_record);
}
void MainForm::actSell()
{
    ConfiguratorAbstractRecord result_record;
    OperationDialog d(opSell, result_record, this);
    connect(&d, SIGNAL(signalGetCurrentPrice(const ConfiguratorAbstractRecord&, double&)), operationsHistoryWidget(), SLOT(slotGetCurrentPrice(const ConfiguratorAbstractRecord&, double&)));
    d.init();
    d.exec();

    if (d.isApply())
    	bagWidget()->sell(result_record);
}
void MainForm::saveCurrentData()
{
    if (!m_stackedWidget) return;

    switch(m_stackedWidget->currentIndex())
    {
		case 0: {generalDataWidget()->slotApply(); break;} //general data
		case 1: {break;} //bag
		case 2: {operationsHistoryWidget()->saveData(); break;} //history
		default: break;
    }
}
QString MainForm::mainTitle() const
{
    return QObject::tr("CFD statistic");
}
void MainForm::load()
{
    LMainWidget::load();
    QSettings settings(companyName(), projectName());
    if (!m_stackedWidget) return;

    m_stackedWidget->setCurrentIndex(settings.value(QString("%1/stacked_index").arg(objectName()), 0).toInt());
    for (int i=0; i<m_stackedWidget->count(); i++)
    {
        LChildWidget *cw = qobject_cast<LChildWidget*>(m_stackedWidget->widget(i));
        if (cw) cw->load(settings);
    }
}
void MainForm::save()
{
    LMainWidget::save();
    QSettings settings(companyName(), projectName());
    if (!m_stackedWidget) return;

    settings.setValue(QString("%1/stacked_index").arg(objectName()), m_stackedWidget->currentIndex());
    for (int i=0; i<m_stackedWidget->count(); i++)
    {
        LChildWidget *cw = qobject_cast<LChildWidget*>(m_stackedWidget->widget(i));
        if (cw) cw->save(settings);
    }
}
void MainForm::actGeneralData()
{
    m_stackedWidget->setCurrentIndex(0);
}
void MainForm::actBag()
{
    m_stackedWidget->setCurrentIndex(1);
}
void MainForm::actHistory()
{
    m_stackedWidget->setCurrentIndex(2);
}
void MainForm::actDivStat()
{
    m_stackedWidget->setCurrentIndex(3);
}




///////////////////private funcs////////////////////////////
ConfiguratorGeneralDataWdiget* MainForm::generalDataWidget() const
{
    if (m_stackedWidget->count() < 1) return NULL;
    return qobject_cast<ConfiguratorGeneralDataWdiget*>(m_stackedWidget->widget(0));
}
BagWidget* MainForm::bagWidget() const
{
    if (m_stackedWidget->count() < 2) return NULL;
    return qobject_cast<BagWidget*>(m_stackedWidget->widget(1));
}
HistoryWidget* MainForm::operationsHistoryWidget() const
{
    if (m_stackedWidget->count() < 3) return NULL;
    return qobject_cast<HistoryWidget*>(m_stackedWidget->widget(2));
}
DivStatWidget* MainForm::divStatWidget() const
{
    if (m_stackedWidget->count() < 4) return NULL;
    return qobject_cast<DivStatWidget*>(m_stackedWidget->widget(3));
}




