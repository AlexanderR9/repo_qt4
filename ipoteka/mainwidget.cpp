 #include "mainwidget.h"
 #include "qpalette.h"
 #include "qsettings.h"

 #define COMPANY        "my_apps"
 #define PROJNAME       "ipoteka"


MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    move(100,100);
    resize(900, 700);

    QIntValidator *iv = new QIntValidator(0, 100000, this);
    QDoubleValidator *dv = new QDoubleValidator(0, 100, 2, this);
    creditPercentComboBox->setValidator(dv);
    priceComboBox->setValidator(iv);
    firsPaymentPercentComboBox->setValidator(dv);
    firsPaymentSumComboBox->setValidator(iv);
    creditSizeComboBox->setValidator(iv);

    clearTable();
    load();

    slotChangeMorePayment();
    slotChangeCreditParameters();
    resultTableWidget->resizeColumnsToContents();



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    connect(creditPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
//    connect(insurancePercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    connect(insurancePercentComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(clearTable()));
    connect(yearsComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(clearTable()));
    connect(priceComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(clearTable()));
    connect(priceComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));

    connect(firsPaymentPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    connect(firsPaymentSumComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    connect(creditSizeComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));

    connect(morePeriodMonthComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(clearTable()));
    connect(morePaymentSumComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(clearTable()));
    connect(maxMonthPaymentCheckBox, SIGNAL(stateChanged(int)), this, SLOT(clearTable()));
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    connect(creditPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeCreditParameters()));
//    connect(insurancePercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeCreditParameters()));
    connect(insurancePercentComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotChangeCreditParameters()));
    connect(yearsComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotChangeCreditParameters()));
    connect(priceComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotChangeCreditParameters()));
    connect(priceComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeCreditParameters()));

    connect(firsPaymentPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeFPPercent()));
    connect(firsPaymentSumComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeFPSum()));
    connect(creditSizeComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeSumCredit()));

    connect(morePeriodMonthComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotChangeMorePayment()));
    connect(morePaymentSumComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotChangeMorePayment()));
    connect(maxMonthPaymentCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotChangeMorePayment()));
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    connect(calcButton, SIGNAL(clicked()), this, SLOT(slotCalc()));

}
void MainWidget::load()
{
    qDebug("MainWidget::load()");
    QSettings settings(COMPANY, PROJNAME);

    creditPercentComboBox->setEditText(settings.value("creditPercentComboBox", QString("13.5")).toString());
    //insurancePercentComboBox->setEditText(settings.value("insurancePercentComboBox", QString("0.0")).toString());
    insurancePercentComboBox->setCurrentIndex(settings.value("insurancePercentComboBox", 0).toInt());
    priceComboBox->setEditText(settings.value("priceComboBox", QString("2100")).toString());
    yearsComboBox->setCurrentIndex(settings.value("yearsNumber", 4).toInt());

    fpPercentRadioButton->setChecked(settings.value("fpPercentRadioButton", true).toBool());
    fpSumRadioButton->setChecked(settings.value("fpSumRadioButton", false).toBool());
    fpCreditSizeRadioButton->setChecked(settings.value("fpCreditSizeRadioButton", false).toBool());
    firsPaymentPercentComboBox->setEditText(settings.value("firsPaymentPercentComboBox", QString("25")).toString());
    firsPaymentSumComboBox->setEditText(settings.value("firsPaymentSumComboBox", QString("200")).toString());
    creditSizeComboBox->setEditText(settings.value("creditSizeComboBox", QString("500")).toString());

    morePeriodMonthComboBox->setCurrentIndex(settings.value("morePeriodMonthComboBox", 0).toInt());
    morePaymentSumComboBox->setCurrentIndex(settings.value("morePaymentSumComboBox", 0).toInt());

    QByteArray ba = settings.value("widget_geometry", QByteArray()).toByteArray();
    if (!ba.isEmpty()) restoreGeometry(ba);
    else {resize(800, 700); move(100, 100);}
}
void MainWidget::save()
{
    qDebug("MainWidget::save()");
    QSettings settings(COMPANY, PROJNAME);
    settings.setValue("widget_geometry", saveGeometry());

    settings.setValue("creditPercentComboBox", creditPercentComboBox->currentText());
//    settings.setValue("insurancePercentComboBox", insurancePercentComboBox->currentText());
    settings.setValue("insurancePercentComboBox", insurancePercentComboBox->currentIndex());
    settings.setValue("priceComboBox", priceComboBox->currentText());
    settings.setValue("yearsNumber", yearsComboBox->currentIndex());

    settings.setValue("fpPercentRadioButton", fpPercentRadioButton->isChecked());
    settings.setValue("fpSumRadioButton", fpSumRadioButton->isChecked());
    settings.setValue("fpCreditSizeRadioButton", fpCreditSizeRadioButton->isChecked());
    settings.setValue("firsPaymentPercentComboBox", firsPaymentPercentComboBox->currentText());
    settings.setValue("firsPaymentSumComboBox", firsPaymentSumComboBox->currentText());
    settings.setValue("creditSizeComboBox", creditSizeComboBox->currentText());

    settings.setValue("morePeriodMonthComboBox", morePeriodMonthComboBox->currentIndex());
    settings.setValue("morePaymentSumComboBox", morePaymentSumComboBox->currentIndex());
}
void MainWidget::slotCalc()
{
//    qDebug("slot calc!");
    clearTable();
    if (!icalc.isValidState()) return;
    icalc.recalc();
    int n = icalc.recordsCount();
    if (n <= 0) return;

    for (int i=0; i<n; i++)
	addRow(icalc.recordAt(i));

    double price = icalc.price();
    double send_money = icalc.firstPaymentSum()+icalc.lastRecord().sum;
    QString s1(QString("%1 %2").arg(QString::fromUtf8("всего выплачено")).arg(QString::number(send_money, 'f', 1)));
    QString s2(QString("%1 %2").arg(QString::fromUtf8("переплата")).arg(QString::number(send_money - price, 'f', 1)));

    if (icalc.insuracePayed() > 0) 	
	s2 = QString("%1(%2 %3)").arg(s2).arg(QString::fromUtf8("из них страховка")).arg(QString::number(icalc.insuracePayed(), 'f', 1));

    QString s3(QString("%1 %2").arg(QString::fromUtf8("срок выплаты")).arg(icalc.allPeriod()));
    setResultLabel(QString("%1,  %2,  %3").arg(s1).arg(s2).arg(s3));
}
void MainWidget::slotChangeFPPercent()
{
    bool ok;
    double d = firsPaymentPercentComboBox->currentText().toDouble(&ok);
    if (ok) icalc.setFirstPaymentPercent(d);
    fillFirstPaymentParameters();
}
void MainWidget::slotChangeFPSum()
{
    bool ok;
    double d = firsPaymentSumComboBox->currentText().toDouble(&ok);
    if (ok) icalc.setFirstPaymentSum(d);
    fillFirstPaymentParameters();
}
void MainWidget::slotChangeSumCredit()
{
    bool ok;
    double d = creditSizeComboBox->currentText().toDouble(&ok);
    if (ok) icalc.setCreditSize(d);
    fillFirstPaymentParameters();
}
void MainWidget::clearTable()
{
    setResultLabel();
    int k = resultTableWidget->rowCount();
    if (k <= 0) return;
    for (int i=0; i<k; i++)
	resultTableWidget->removeRow(0);
    
}
void MainWidget::slotChangeCreditParameters()
{
    qDebug("credit parameters changed!");
    icalc.resetParameters();
    setCreditParameters();
    calcFirstPaymentByCreditParameters();    
    fillFirstPaymentParameters();
    
}
void MainWidget::slotChangeMorePayment()
{
    bool ok;
    icalc.setIsMaxMonthPayment(maxMonthPaymentCheckBox->isChecked());
    morePeriodMonthComboBox->setEnabled(!maxMonthPaymentCheckBox->isChecked());
    morePaymentSumComboBox->setPalette(QPalette());
    calcButton->setEnabled(true);
    double d = morePaymentSumComboBox->currentText().toDouble(&ok);
    if (!ok) return;
    icalc.setMorePaymentSum(d);
    

    if (maxMonthPaymentCheckBox->isChecked())
    {
	icalc.setMorePaymentReriod(1);
	double pm = paymentLineEdit->text().toDouble(&ok);
	if (!ok) {morePaymentSumComboBox->setEditText("?"); return;}
	if (d < pm) 
	{
	    icalc.setMorePaymentSum(-1);
	    QPalette palette(QColor(255,0,0));
	    morePaymentSumComboBox->setPalette(palette);
	    calcButton->setEnabled(false);
	}
    }
    else
    {
	uint u = morePeriodMonthComboBox->currentText().toUInt(&ok);
	if (!ok) return;
	icalc.setMorePaymentReriod(u);
    }
}
void MainWidget::addRow(const IRecord &ir)
{
    int k = resultTableWidget->rowCount();
    resultTableWidget->insertRow(k);
    
    QTableWidgetItem *item0 = new QTableWidgetItem(ir.month);
    item0->setFlags(Qt::ItemIsEnabled);
    item0->setTextAlignment(Qt::AlignCenter);
    item0->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 0, item0);

    QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(ir.payment, 'f', PRECISION));
    item1->setFlags(Qt::ItemIsEnabled);
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 1, item1);

    QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(ir.inDebt, 'f', PRECISION));
    item2->setFlags(Qt::ItemIsEnabled);
    item2->setTextAlignment(Qt::AlignCenter);
    item2->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 2, item2);

    QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(ir.morePayment, 'f', PRECISION));
    item3->setFlags(Qt::ItemIsEnabled);
    item3->setTextAlignment(Qt::AlignCenter);
    item3->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 3, item3);

    QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(ir.curDebt, 'f', PRECISION));
    item4->setFlags(Qt::ItemIsEnabled);
    item4->setTextAlignment(Qt::AlignCenter);
    item4->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 4, item4);

    QTableWidgetItem *item5 = new QTableWidgetItem(QString::number(ir.insurance, 'f', PRECISION));
    item5->setFlags(Qt::ItemIsEnabled);
    item5->setTextAlignment(Qt::AlignCenter);
    item5->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 5, item5);

    QTableWidgetItem *item6 = new QTableWidgetItem(QString::number(ir.sum, 'f', PRECISION));
    item6->setFlags(Qt::ItemIsEnabled);
    item6->setTextAlignment(Qt::AlignCenter);
    item6->setBackground(QBrush(ir.color));
    resultTableWidget->setItem(k, 6, item6);
    
}
void MainWidget::setCreditParameters()
{
    bool ok;
    double d;
    d = creditPercentComboBox->currentText().toDouble(&ok);
    if (!ok) return;
    else icalc.setCreditPercent(d);

    d = insurancePercentComboBox->currentText().toDouble(&ok);
    if (!ok) return;
    else icalc.setInsuranceSum(d);

    uint u = yearsComboBox->currentText().toUInt(&ok);
    if (!ok) return;
    else icalc.setYears(u);

    d = priceComboBox->currentText().toDouble(&ok);
    if (!ok) return;
    else icalc.setPrice(d);
}
void MainWidget::calcFirstPaymentByCreditParameters()
{
    bool ok;
    double d;
    if (fpPercentRadioButton->isChecked())
    {
	d = firsPaymentPercentComboBox->currentText().toDouble(&ok);
	if (!ok) return;
	else icalc.setFirstPaymentPercent(d);
	if (!icalc.isValidState()) return;
    }
    else if (fpSumRadioButton->isChecked())
    {
	d = firsPaymentSumComboBox->currentText().toDouble(&ok);
	if (!ok) return;
	else icalc.setFirstPaymentSum(d);
	if (!icalc.isValidState()) return;
    }
    else
    {
	d = creditSizeComboBox->currentText().toDouble(&ok);
	if (!ok) return;
	else icalc.setCreditSize(d);    
	if (!icalc.isValidState()) return;
    }
}
void MainWidget::fillFirstPaymentParameters()
{
    disconnect(firsPaymentPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    disconnect(firsPaymentSumComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    disconnect(creditSizeComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    disconnect(firsPaymentPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeFPPercent()));
    disconnect(firsPaymentSumComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeFPSum()));
    disconnect(creditSizeComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeSumCredit()));

    firsPaymentPercentComboBox->setEditText(QString::number(icalc.firstPaymentPercent(), 'f', 1));
    firsPaymentSumComboBox->setEditText(QString::number(icalc.firstPaymentSum(), 'f', 0));
    creditSizeComboBox->setEditText(QString::number(icalc.creditSize(), 'f', 0));

    if (icalc.isValidState()) paymentLineEdit->setText(QString::number(icalc.monthPayment(), 'f', PRECISION));
    else paymentLineEdit->setText("err");

    slotChangeMorePayment();

    connect(firsPaymentPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    connect(firsPaymentSumComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    connect(creditSizeComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearTable()));
    connect(firsPaymentPercentComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeFPPercent()));
    connect(firsPaymentSumComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeFPSum()));
    connect(creditSizeComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotChangeSumCredit()));
}

