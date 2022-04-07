 #include "mainform.h"
  #include "lcommonsettings.h"
  #include "lchildwidget.h"
  #include "lstatic.h"
 
 #include <QDebug>
 #include <QTimer>
 #include <QTabWidget>
 #include <QListWidget>
 #include <QLabel>
 #include <QSplitter>


//////// BoundsCentraWidget ////////////////////////
BoundsCentraWidget::BoundsCentraWidget(QWidget *parent)
    :LChildWidget(parent),
    m_splitter1(NULL),
    m_splitter2(NULL),
    m_splitter3(NULL),
    m_timer(NULL),
    m_stage(0),
    need_clear(true)
{
    setupUi(this);
    setObjectName("bounds_page");

    initSplitters();
    initStateTable();

    LStatic::fullClearTable(protocolTable);
    LStatic::fullClearTable(historyTable);

    m_timer = new QTimer(this);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotNextWeek()));
    

}
void BoundsCentraWidget::initSplitters()
{
    m_splitter1 = new QSplitter(Qt::Vertical, this);
    m_splitter2 = new QSplitter(Qt::Horizontal, this);
    m_splitter3 = new QSplitter(Qt::Horizontal, this);
    m_splitter1->addWidget(m_splitter2);
    m_splitter1->addWidget(m_splitter3);
    
    if (layout()) delete layout();
    setLayout(new QVBoxLayout(0));
    layout()->addWidget(m_splitter1);

    m_splitter2->addWidget(paramsBox);
    m_splitter2->addWidget(currentStateBox);
    m_splitter3->addWidget(protocolBox);
    m_splitter3->addWidget(historyBox);
}
void BoundsCentraWidget::setNextWeek()
{
    if (m_state.cur_period == 0)
    {
	m_state.cur_period = 1;
	m_state.cur_week = 1;
    }
    else
    {
	m_state.cur_week++;
	if (m_state.cur_week > periodWeeks())
	{
	    m_state.cur_week = 1;
	    m_state.cur_period++;
	}
    }
}
void BoundsCentraWidget::addNextBonds()
{
    int cur_pay = m_state.currentWeekPay();
    int cur_space = m_state.space_sum;
    int all_coupons = m_state.allPaysNumber();
    m_state.nextPays();

    int min_week = minWeekBonds();
    int n_weeks = periodWeeks();
    int add_n = addBondsWeekComboBox->currentText().toInt();
    if (cur_space > 1000) 
    {
	while (cur_space > 1000) {add_n++; cur_space -= 1000;}
    }
    m_state.space_sum = cur_space + cur_pay;

    switch(addingMetodComboBox->currentIndex())
    {
	case 0:
	{
	    int cur_week = min_week;
	    for (int i=0; i<add_n; i++)
	    {
		m_state.addBond(cur_week);
		cur_week++;
		if (cur_week > n_weeks) cur_week = 1;
	    }
		
	    break;
	}
	case 1:
	{
	    for (int i=0; i<add_n; i++)
		m_state.addBond(min_week);
	    break;
	}
	default: return;
    }

    updateTables(cur_pay, all_coupons, add_n);
}
void BoundsCentraWidget::updateTables(int cur_pay, int all_coupons, int added_bonds)
{
    int cw = m_state.cur_week;

    //state table
    QString str_week = QString("%1/%2").arg(cw).arg(m_state.cur_period);
    stateTable->item(0, 0)->setText(str_week);
    stateTable->item(0, 1)->setText(QString::number(m_state.space_sum));
    stateTable->item(0, 2)->setText(QString::number(m_state.allBonds()));
    QString last_pay = QString("%1/%2").arg(cur_pay/m_state.coupon_size).arg(cur_pay);
    stateTable->item(0, 3)->setText(last_pay);
    stateTable->item(0, 4)->setText(QString::number(all_coupons*m_state.coupon_size));

    //protocol table
    int n_weeks = periodWeeks();
    QStringList list;	
    for (int i=0; i<n_weeks; i++)
    {
	QString s = QString("%1").arg(m_state.bondsCountByWeek(i+1));
	list.append(s);
    }
    LStatic::addTableRow(protocolTable, list);

    //history table
    
    list.clear();
    list.append(QString::number(cw));
    list.append(last_pay);
    list.append(QString::number(added_bonds));
//    list.append(QString::number(m_state.bondsCountByWeek(cw)));
    LStatic::addTableRow(historyTable, list);
    

    
//////////////////////////////////////////////////
    if (lCommonSettings.paramValue("auto_scrolling").toBool())
    {
	protocolTable->scrollToBottom();
	historyTable->scrollToBottom();
    }

    LStatic::resizeTableContents(stateTable);
    LStatic::resizeTableContents(protocolTable);
    LStatic::resizeTableContents(historyTable);

}
void BoundsCentraWidget::slotNextWeek()
{
    qDebug("BoundsCentraWidget::slotNextWeek()");

    setNextWeek();
//    getWeekPays();
    addNextBonds();

    emit signalNextWeek(m_state.allBonds(), m_state.currentWeekPay());

    //	finish
    if (m_state.cur_week == periodWeeks() && m_state.cur_period == waitPeriodsComboBox->currentText().toInt())
	emit signalFinish();

}
int BoundsCentraWidget::periodWeeks() const
{
    int wy = weeksYearLineEdit->text().toInt();
    return int(wy/couponNumberComboBox->currentText().toInt());
}
int BoundsCentraWidget::minWeekBonds() const
{
    int min_week = 0;
    int min = -1;
    int pn = periodWeeks();
    for (int i=0; i<pn; i++)
    {
	int w = i+1;    
	int b_count = m_state.bondsCountByWeek(w);
	if (min < 0) {min = b_count; min_week = w;}
	else if (min > b_count) {min = b_count; min_week = w;}
    }
    return min_week;
}
void BoundsCentraWidget::initStateTable()
{
    LStatic::fullClearTable(stateTable);

    QStringList headers;
    headers.append("Value");
    LStatic::setTableHeaders(stateTable, headers, Qt::Horizontal);

    headers.clear();
    headers << "Current week (week/period)";
    headers << "Space sum";
    headers << "Bonds count";
    headers << "Last coupon size (bonds/sum)";
    headers << "All coupons sum";
    LStatic::setTableHeaders(stateTable, headers, Qt::Vertical);

    for (int i=0; i<stateTable->rowCount(); i++)
	for (int j=0; j<stateTable->columnCount(); j++)	
	    LStatic::createTableItem(stateTable, i, j, QString());

    LStatic::resizeTableContents(stateTable);
}
void BoundsCentraWidget::resetTables()
{
    for (int i=0; i<stateTable->rowCount(); i++)
	stateTable->item(i, 0)->setText(QString());

    LStatic::fullClearTable(protocolTable);
    LStatic::fullClearTable(historyTable);


    QStringList headers;
    int pn = periodWeeks();
    for (int i=0; i<pn; i++)
	headers.append(QString("Week %1").arg(i+1));
    LStatic::setTableHeaders(protocolTable, headers, Qt::Horizontal);

    headers.clear();
    headers << "Week" << "Sum" << "Added bonds";// << "Bonds week";
    LStatic::setTableHeaders(historyTable, headers, Qt::Horizontal);


    LStatic::resizeTableContents(stateTable);
    LStatic::resizeTableContents(protocolTable);
    LStatic::resizeTableContents(historyTable);

}
void BoundsCentraWidget::save(QSettings &settings)
{
    settings.setValue(QString("%1/couponSizeIndex").arg(objectName()), couponSizeComboBox->currentIndex());
    settings.setValue(QString("%1/couponNumberIndex").arg(objectName()), couponNumberComboBox->currentIndex());
    settings.setValue(QString("%1/addBondsWeekIndex").arg(objectName()), addBondsWeekComboBox->currentIndex());
    settings.setValue(QString("%1/addingMetodIndex").arg(objectName()), addingMetodComboBox->currentIndex());
    settings.setValue(QString("%1/waitPeriodsIndex").arg(objectName()), waitPeriodsComboBox->currentIndex());
    settings.setValue(QString("%1/timerIntervalIndex").arg(objectName()), timerIntervalComboBox->currentIndex());

    settings.setValue(QString("%1/splitter1").arg(objectName()), m_splitter1->saveState());
    settings.setValue(QString("%1/splitter2").arg(objectName()), m_splitter2->saveState());
    settings.setValue(QString("%1/splitter3").arg(objectName()), m_splitter3->saveState());
    
}
void BoundsCentraWidget::load(QSettings &settings)
{
    couponSizeComboBox->setCurrentIndex(settings.value(QString("%1/couponSizeIndex").arg(objectName()), 0).toInt());
    couponNumberComboBox->setCurrentIndex(settings.value(QString("%1/couponNumberIndex").arg(objectName()), 0).toInt());
    addBondsWeekComboBox->setCurrentIndex(settings.value(QString("%1/addBondsWeekIndex").arg(objectName()), 0).toInt());
    addingMetodComboBox->setCurrentIndex(settings.value(QString("%1/addingMetodIndex").arg(objectName()), 0).toInt());
    waitPeriodsComboBox->setCurrentIndex(settings.value(QString("%1/waitPeriodsIndex").arg(objectName()), 0).toInt());
    timerIntervalComboBox->setCurrentIndex(settings.value(QString("%1/timerIntervalIndex").arg(objectName()), 0).toInt());

    QByteArray ba = settings.value(QString("%1/splitter1").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) m_splitter1->restoreState(ba);
    ba = settings.value(QString("%1/splitter2").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) m_splitter2->restoreState(ba);
    ba = settings.value(QString("%1/splitter3").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) m_splitter3->restoreState(ba);

}
void BoundsCentraWidget::updateByStage()
{
    switch (m_stage)
    {
	case bStoped:
	{
	    enableControls(true);
	    stopTest();
	    break;
	}
	case bRunning:
	{
	    enableControls(false);
	    startTest();
	    break;
	}
	case bPaused:
	{
	    enableControls(false);
	    need_clear = false;
	    m_timer->stop();
	    break;
	}
	default: break;
    }
}
void BoundsCentraWidget::enableControls(bool b)
{
    couponSizeComboBox->setEnabled(b);
    couponNumberComboBox->setEnabled(b);
    addBondsWeekComboBox->setEnabled(b);
    addingMetodComboBox->setEnabled(b);
    waitPeriodsComboBox->setEnabled(b);
    timerIntervalComboBox->setEnabled(b);
}
void BoundsCentraWidget::startTest()
{
    if (need_clear)
    {
	m_timer->setInterval(timerIntervalComboBox->currentText().toInt());
	resetTables();

	m_state.reset();
	m_state.coupon_size = couponSizeComboBox->currentText().toInt();
	m_state.add_week_bonds = addBondsWeekComboBox->currentText().toInt();
    }

    m_timer->start();
}
void BoundsCentraWidget::stopTest()
{
    need_clear = true;;
    m_timer->stop();

}




//////// MainForm ////////////////////////
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_boundsWidget(NULL)
{
    setObjectName("main_form");

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atPause);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
    m_boundsWidget = new BoundsCentraWidget(this);
    addWidget(m_boundsWidget, 0, 0, 1, 1);

    connect(m_boundsWidget, SIGNAL(signalFinish()), this, SLOT(slotStop()));
    connect(m_boundsWidget, SIGNAL(signalNextWeek(int, int)), this, SLOT(slotUpdateWindowTitle(int, int)));

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
//    lCommonSettings.setDefValue(key);

    key = QString("auto_scrolling");
    lCommonSettings.addParam(QString("Auto scrolling tables"), LSimpleDialog::sdtBool, key);

/////////////////////////////////////////////////////////////////////////////////////////

}
void MainForm::updateButtons()
{
    switch(m_boundsWidget->stage())
    {
	case bStoped:
	{
	    getAction(atStop)->setEnabled(false);
	    getAction(atPause)->setEnabled(false);
	    getAction(atStart)->setEnabled(true);
	    getAction(atSettings)->setEnabled(true);
	    break;
	}
	case bRunning:
	{
	    getAction(atStop)->setEnabled(true);
	    getAction(atPause)->setEnabled(true);
	    getAction(atStart)->setEnabled(false);
	    getAction(atSettings)->setEnabled(false);
	    break;
	}
	case bPaused:
	{
	    getAction(atStop)->setEnabled(true);
	    getAction(atPause)->setEnabled(false);
	    getAction(atStart)->setEnabled(true);
	    getAction(atSettings)->setEnabled(false);
	    break;
	}
	default: break;
    }
}
void MainForm::stop()
{
    m_boundsWidget->setStage(bStoped);
    updateButtons();
}
void MainForm::start()
{
    m_boundsWidget->setStage(bRunning);
    updateButtons();
}
void MainForm::pause()
{
    m_boundsWidget->setStage(bPaused);
    updateButtons();
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
	case LMainWidget::atSettings: {actCommonSettings(); break;}
	case LMainWidget::atStart: {start(); break;}
	case LMainWidget::atPause: {pause(); break;} 
	case LMainWidget::atStop: {stop(); break;} 

	default: break;
    }
}
QString MainForm::mainTitle() const
{
    return QObject::tr("Bounds investing test");
}
void MainForm::slotUpdateWindowTitle(int total, int pay)
{
    QString s = QString("%1: total bonds %2,  week pay %3").arg(mainTitle()).arg(total).arg(pay);
    setWindowTitle(s);
}
void MainForm::load()
{
    LMainWidget::load();
    QSettings settings(companyName(), projectName());
    m_boundsWidget->load(settings);
}
void MainForm::save()
{
    LMainWidget::save();
    QSettings settings(companyName(), projectName());
    m_boundsWidget->save(settings);
}

