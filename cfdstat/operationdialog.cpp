 #include "operationdialog.h"
 #include "cfdenums.h"
 #include "generaldatawidget.h"
 #include "lcommonsettings.h"


 #include <QDebug>
 #include <QDate>
 #include <QMessageBox>
 #include <QComboBox>

int OperationDialogBase::company_index = -1;

/////////////OperationDialogBase/////////////////////////
OperationDialogBase::OperationDialogBase(ConfiguratorAbstractRecord &rec, QWidget *parent)
	:LSimpleDialog(parent),
	 m_record(rec)
{
    setObjectName("operation_dialog_base");

}
void OperationDialogBase::load()
{
    if (company_index < 0) return;

    QString key = ConfiguratorEnums::xmlAttrNameByType(ftCompany);
    const SimpleWidget *sw = widgetByKey(key);
    if (sw) 
    {
	if (company_index < sw->comboBox->count())
	{
	    sw->comboBox->setCurrentIndex(company_index);
	}
    }
}
void OperationDialogBase::save()
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftCompany);
    const SimpleWidget *sw = widgetByKey(key);
    if (sw) company_index = sw->comboBox->currentIndex();
}
void OperationDialogBase::getCompanyInfo(int c_id, CompanyInfo &data)
{
    ConfiguratorAbstractData g_data;
    GeneralDataFileReader::loadDataFormFile(gdCompany, g_data);
    if (g_data.invalid()) {qWarning("OperationDialogBase::getCompanyInfo: ERR - invalid load company general data"); return;}
    for (int j=0; j<g_data.count(); j++)
    {
        if (g_data.recAtValue(j, ftID).toInt() == c_id)
	{
	    data.rec.copy(g_data.records.at(j));
	    break;
	}
    }

    GeneralDataFileReader::loadDataFormFile(gdCFD, g_data);
    if (g_data.invalid()) {qWarning("OperationDialogBase::getCompanyInfo: ERR - invalid load CFD general data"); return;}
    for (int j=0; j<g_data.count(); j++)
        if (g_data.recAtValue(j, ftCompany).toInt() == c_id) {data.has_cfd = true; break;}
    
    GeneralDataFileReader::loadDataFormFile(gdBond, g_data);
    if (g_data.invalid()) {qWarning("OperationDialogBase::getCompanyInfo: ERR - invalid load bonds general data"); return;}
    for (int j=0; j<g_data.count(); j++)
        if (g_data.recAtValue(j, ftCompany).toInt() == c_id) 
	    data.bond_kks.insert(g_data.recAtValue(j, ftID).toInt(), g_data.recAtValue(j, ftKKS));
}
void OperationDialogBase::placeSubWidgets()
{
    QList<int> fields = fieldsByType();
    for (int i=0; i<fields.count(); i++)
    {
	int ft = fields.at(i);
	QString name = ConfiguratorEnums::interfaceTextByType(ft);
	QString key = ConfiguratorEnums::xmlAttrNameByType(ft);

	if (ConfiguratorEnums::isDoubleField(ft))
	{
	    addSimpleWidget(name, sdtDoubleLine, key, 2); 
	    continue;
	}
        if (ConfiguratorEnums::isFixField(ft))
        {
            addSimpleWidget(name, sdtStringCombo, key);
            QVariantList list;
            QVariantList user_data;
            ConfiguratorAbstractData g_data;
            GeneralDataFileReader::loadDataFormFile(ConfiguratorEnums::generalTypeByFieldType(ft), g_data);
            if (g_data.invalid()) break;
            for (int j=0; j<g_data.count(); j++)
            {
                list.append(g_data.recAtValue(j, ftName));
                user_data.append(g_data.recAtValue(j, ftID));
            }
            setComboList(key, list, user_data);
            continue;
        }


	switch(ft)
	{
	    case ftKKS: 
	    case ftName: 
	    case ftPaperType: 
	    {
	        addSimpleWidget(name, sdtStringCombo, key);
	        break;
	    }
	    case ftDateOperation: 
	    {
	        addSimpleWidget(name, sdtString, key);
	        break;
	    }
	    case ftCount: 
	    {
	        addSimpleWidget(name, sdtIntCombo, key);
	        break;
	    }
	    default: break;
        }
    }
}
void OperationDialogBase::init()
{
    placeSubWidgets();
    setCaptionsWidth(150);
    addVerticalSpacer();
    setExpandWidgets();
    initPageByData();
    load();
    slotCompanyChanged();
}
void OperationDialogBase::finishedKKS()
{
    if (m_record.hasField(ftKKS))
    {
	if (isCompanyType())  
	    m_record.record.insert(ftKKS, QString());
    }
}
void OperationDialogBase::slotApply()
{
    QString err;
    checkPageValues(err);

    if (!err.isEmpty())
    {
        QMessageBox::critical(this, MSGBOX_ERR_TITLE, err);
        return;
    }

    fillResultRecord();
    finishedKKS();
    LSimpleDialog::slotApply();
}
void OperationDialogBase::slotCompanyChanged()
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftKKS);
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) return;
    if (!sw->comboBox) return;
    sw->comboBox->clear();

    cur_company.reset();
    getCompanyInfo(currentCompany(), cur_company);
//    qDebug()<<cur_company.toStr();

    prepareKKSList();
    slotPaperTypeChanged();
}
void OperationDialogBase::slotPaperTypeChanged()
{
//    qDebug("OperationDialogBase::slotPaperTypeChanged()");
    updateEnabled();
    slotGetCurrentPrice();
}
void OperationDialogBase::slotGetCurrentPrice()
{
//    qDebug("OperationDialogBase::slotGetCurrentPrice()");

    double cp = -1;
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPrice1);
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::slotGetCurrentPrice(): ERR - sw is null, key=%1").arg(key); return;}

    setWidgetValue(key, cp);    
    if (!sw->edit->isEnabled()) return;
    
    QString kks;
    if (!isCompanyType())
    {
	key = ConfiguratorEnums::xmlAttrNameByType(ftKKS);
	sw = widgetByKey(key);
	if (!sw) {qWarning()<<QString("OperationDialogBase::slotGetCurrentPrice(): ERR - sw is null, key=%1").arg(key); return;}
	kks = sw->comboBox->currentText();
    }

    ConfiguratorAbstractRecord rec;
    rec.record.insert(ftID, QString::number(cur_company.id()));
    rec.record.insert(ftKKS, kks);

    emit signalGetCurrentPrice(rec, cp);
    key = ConfiguratorEnums::xmlAttrNameByType(ftPrice1);
    setWidgetValue(key, cp);
}
void OperationDialogBase::fillResultRecord()
{
    m_record.serial = ConfiguratorAbstractData::nextSerial();
    QList<int> fields = fieldsByType();
    for (int i=0; i<fields.count(); i++)
    {
        int ft = fields.at(i);
        QString key = ConfiguratorEnums::xmlAttrNameByType(ft);
	m_record.record.insert(ft, widgetValue(key).toString().trimmed());	
	if (ft == ftCompany) 
	{
	    const SimpleWidget *sw = widgetByKey(key);
	    int index = sw->comboBox->currentIndex();
	    int user_data = sw->comboBox->itemData(index).toInt();
	    m_record.record.insert(ft, QString::number(user_data));	
	}
    }    
}
bool OperationDialogBase::isCompanyType() const
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPaperType);
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::isCompanyType()(): ERR -  widgetByKey(%1) is NULL").arg(key); return false;} 
    return (sw->comboBox->currentIndex() == 0);
}
int OperationDialogBase::currentCompany() const
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftCompany);
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::currentCompany(): ERR -  widgetByKey(%1) is NULL").arg(key); return -1;} 
    int index = sw->comboBox->currentIndex();
    return sw->comboBox->itemData(index).toInt();
}
void OperationDialogBase::initPaperTypeList()
{
    QVariantList vlist;
    vlist.append(ConfiguratorEnums::interfaceTextByType(gdCFD));
    vlist.append(ConfiguratorEnums::interfaceTextByType(gdBond));
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPaperType);
    setComboList(key, vlist);	

    const SimpleWidget *sw = widgetByKey(key);
    if (sw)
    {
//	qDebug("sw ok of ftPaperType");
	connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPaperTypeChanged()));
    }

    key = ConfiguratorEnums::xmlAttrNameByType(ftKKS);
    if (sw)
	connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGetCurrentPrice()));

}
void OperationDialogBase::initCompanyList()
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftCompany);
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::initCompanyWidget(): ERR -  widgetByKey(%1) is NULL").arg(key); return;} 

    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCompanyChanged()));
}
void OperationDialogBase::prepareKKSList()
{
//    qDebug("OperationDialog::preparePaperType()");
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPaperType);
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::preparePaperType(): ERR -  widgetByKey(%1) is NULL").arg(key); return;}

    disconnect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPaperTypeChanged()));
    sw->comboBox->setCurrentIndex(0);
    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPaperTypeChanged()));

    key = ConfiguratorEnums::xmlAttrNameByType(ftKKS);
    sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::preparePaperType(): ERR -  widgetByKey(%1) is NULL").arg(key); return;}

    QList<int> id_bonds(cur_company.bond_kks.keys());
    for (int i=0; i<id_bonds.count(); i++)
        sw->comboBox->addItem(cur_company.bond_kks.value(id_bonds.at(i)), id_bonds.at(i));

    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGetCurrentPrice()));
}
void OperationDialogBase::updateEnabled()
{
    bool b = false;
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftKKS);
    const SimpleWidget *sw = widgetByKey(key);
    if (sw)
    {
	b = (sw->comboBox->count() > 0);
	sw->comboBox->setEnabled(!isCompanyType() && b);
    }

    key = ConfiguratorEnums::xmlAttrNameByType(ftCouponSize);
    sw = widgetByKey(key);
    if (sw) sw->edit->setEnabled(!isCompanyType() && b);

    key = ConfiguratorEnums::xmlAttrNameByType(ftPrice1);
    sw = widgetByKey(key);
    if (!sw) {qWarning()<<QString("OperationDialogBase::preparePaperType(): ERR -  widgetByKey(%1) is NULL").arg(key); return;}
    if (isCompanyType()) sw->edit->setEnabled(cur_company.has_cfd);
    else sw->edit->setEnabled(!cur_company.bond_kks.isEmpty());
}





/////////////UpdatePricesDialog/////////////////////////
UpdatePricesDialog::UpdatePricesDialog(ConfiguratorAbstractRecord &rec, QWidget *parent)
	:OperationDialogBase(rec, parent)
{
    setObjectName("update_price_dialog");
    setWindowTitle("Update prices");

}
QList<int> UpdatePricesDialog::fieldsByType() const
{
    QList<int> list;
    list << ftCompany << ftPaperType << ftKKS << ftPrice1 << ftDateOperation;
    return list;
}
void UpdatePricesDialog::initPageByData()
{
    initCompanyList();
    initPaperTypeList();

    QVariantList vlist;
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftDateOperation);
    setWidgetValue(key, QDate::currentDate().toString(DATE_MASK));
}
void UpdatePricesDialog::checkPageValues(QString &err)
{
    bool ok;
    err.clear();

    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPrice1);
    QString value = widgetValue(key).toString().trimmed();
    double d = value.toDouble(&ok);
    if (!ok || d <= 0) {err = tr("Invalid page value [%1]").arg(key); return;}
    
    key = ConfiguratorEnums::xmlAttrNameByType(ftDateOperation);
    value = widgetByKey(key)->edit->text().trimmed();
    QDate dt = QDate::fromString(value, DATE_MASK);
    if (!dt.isValid() || dt.isNull()) {err = tr("Invalid page value [%1]").arg(key); return;}
    if (dt > QDate::currentDate()) {err = tr("Invalid page value [%1]").arg(key); return;}
}

/////////////EditPricesDialog/////////////////////////
EditPricesDialog::EditPricesDialog(ConfiguratorAbstractRecord &rec, QWidget *parent)
    :UpdatePricesDialog(rec, parent)
{
    setObjectName("edit_price_dialog");
    setWindowTitle("Editing prices");

}
void EditPricesDialog::init()
{
//    qDebug("EditPricesDialog::init()");
    OperationDialogBase::init();
    
//    qDebug() << m_record.toString();
    recToPage();
}
void EditPricesDialog::recToPage()
{
    int field = ftCompany;
    int id = m_record.record.value(field).toInt();
    QString key = ConfiguratorEnums::xmlAttrNameByType(field);
    const SimpleWidget *sw = widgetByKey(key);
    int pos = sw->comboBox->findData(id);
    if (pos < 0) return;
    sw->comboBox->setCurrentIndex(pos);
    sw->comboBox->setEnabled(false);

    field = ftKKS;
    QString kks = m_record.record.value(field);

    field = ftPaperType;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    sw = widgetByKey(key);
    pos = kks.isEmpty() ? 0 : 1; 
    sw->comboBox->setCurrentIndex(pos);
    sw->comboBox->setEnabled(false);
    
    if (!kks.isEmpty())
    {
//	qDebug()<<QString("kks = %1").arg(kks);
	field = ftKKS;
	key = ConfiguratorEnums::xmlAttrNameByType(field);
	sw = widgetByKey(key);
	pos = sw->comboBox->findText(kks);
	if (pos < 0) return;
	sw->comboBox->setCurrentIndex(pos);
    }

    field = ftPrice1;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    double d = m_record.record.value(field).toDouble();
    setWidgetValue(key, d);

    field = ftDateOperation;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    setWidgetValue(key, m_record.record.value(field));
    
}




/////////////OperationDialog/////////////////////////
OperationDialog::OperationDialog(int type, ConfiguratorAbstractRecord &rec, QWidget *parent)
	:OperationDialogBase(rec, parent),
	 m_type(type)
{
    resize(500, 400);
    switch(m_type)
    {
        case opBuy: 
        {
    	    setWindowTitle(QObject::tr("Buy operation")); 
	    setObjectName("buy_dialog");
	    break;
	}
	case opSell: 
	{
	    setWindowTitle(QObject::tr("Sell operation")); 
	    setObjectName("sell_dialog");
	    break;
	}
	default: 
	{
	    setWindowTitle(QObject::tr("Unknown operation")); 
	    setObjectName("unknown_dialog");
	    return;
	}
    }
}
void OperationDialog::connectSignalsForCalc()
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPrice1);
    const SimpleWidget *sw = widgetByKey(key);
    if (sw)
	connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCalcSum()));

    key = ConfiguratorEnums::xmlAttrNameByType(ftCouponSize);
    sw = widgetByKey(key);
    if (sw)
	connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCalcSum()));

    key = ConfiguratorEnums::xmlAttrNameByType(ftCommission);
    sw = widgetByKey(key);
    if (sw)
	connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCalcSum()));

    key = ConfiguratorEnums::xmlAttrNameByType(ftCount);
    sw = widgetByKey(key);
    if (sw)
	connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCalcSum()));


    if (m_type == opSell)
    {
	key = ConfiguratorEnums::xmlAttrNameByType(ftNalogSize);
	sw = widgetByKey(key);
	if (sw)
	    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCalcSum()));
    }
}
void OperationDialog::init()
{
    OperationDialogBase::init();
    m_record.reset();


    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPrice);
    setWidgetValue(key, QString("-1"));

    key = ConfiguratorEnums::xmlAttrNameByType(ftNalogSize);
    setWidgetValue(key, QString("0.0"));

    connectSignalsForCalc();
}
void OperationDialog::slotCalcSum()
{
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftPrice);
    setWidgetValue(key, QString("-1"));

    if (!sender()) return;
    QString sender_name = sender()->objectName().toLower().trimmed();
//    qDebug()<<QString("OperationDialog::slotCalcSum():  sender - %1").arg(sender_name);
    
    bool ok;
    key = ConfiguratorEnums::xmlAttrNameByType(ftPrice1);
    double p1 = widgetValue(key).toDouble(&ok);	
    if (!ok || p1 <= 0) return;

    key = ConfiguratorEnums::xmlAttrNameByType(ftCount);
    int count = widgetValue(key).toInt(&ok);	
    if (!ok || count < 1) return;

    double price_all = p1*count;
//    int precision = lCommonSettings.paramValue("precision").toInt();    
    double cs_commission = lCommonSettings.paramValue("commission").toDouble();    

    double commission = 0;
    key = ConfiguratorEnums::xmlAttrNameByType(ftCommission);
    if (sender_name.contains(key))
    {
	double commission = widgetValue(key).toDouble(&ok);	
	if (!ok || commission <= 0) return;
    }
    else
    {
	commission = price_all*cs_commission/100;
	setWidgetValue(key, commission);
    }
    
    double coupons = 0;
    if (!isCompanyType())
    {
        key = ConfiguratorEnums::xmlAttrNameByType(ftCouponSize);
	coupons = widgetValue(key).toDouble(&ok);	
	if (!ok || coupons < 0) return;
    }

    if (m_type == opSell)
    {
        key = ConfiguratorEnums::xmlAttrNameByType(ftNalogSize);
	double nalog = widgetValue(key).toDouble(&ok);	
	if (!ok || nalog < 0) {qWarning("OperationDialog::slotCalcSum() ERR: invalid nalog value"); return;}
	price_all -= (commission + nalog);
    }
    else price_all += (commission + coupons*count);

    key = ConfiguratorEnums::xmlAttrNameByType(ftPrice);
    setWidgetValue(key, price_all);

}
QList<int> OperationDialog::fieldsByType() const
{
    QList<int> list;
    switch(m_type)
    {
        case opBuy: 
        {
	    list << ftCompany << ftPaperType << ftKKS << ftCouponSize << ftPrice1 << ftCount << ftCommission << ftPrice << ftDateOperation;
	    break;
	}
	case opSell: 
	{
	    list << ftCompany << ftPaperType << ftKKS << ftCouponSize << ftPrice1 << ftCount << ftCommission << ftNalogSize << ftPrice << ftDateOperation;
	    break;
	}
	default: break;
    }
    return list;
}
void OperationDialog::initPageByData()
{
    initCompanyList();
    initPaperTypeList();

    QVariantList vlist;
    for (int i=1; i<=30; i++) vlist.append(i);
    vlist.append(int(35));
    vlist.append(int(40));
    vlist.append(int(45));
    vlist.append(int(50));
    vlist.append(int(60));
    vlist.append(int(70));
    vlist.append(int(100));
    vlist.append(int(1000));
    vlist.append(int(2000));
    vlist.append(int(3000));
    vlist.append(int(4000));
    vlist.append(int(5000));
    vlist.append(int(10000));
    vlist.append(int(20000));
    vlist.append(int(30000));
    QString key = ConfiguratorEnums::xmlAttrNameByType(ftCount);
    setComboList(key, vlist);	

    key = ConfiguratorEnums::xmlAttrNameByType(ftDateOperation);
    setWidgetValue(key, QDate::currentDate().toString(DATE_MASK));
    key = ConfiguratorEnums::xmlAttrNameByType(ftCouponSize);
    setWidgetValue(key, 0);
}
void OperationDialog::checkPageValues(QString &err)
{
    err.clear();

    bool ok;
    QList<int> fields = fieldsByType();
    for (int i=0; i<fields.count(); i++)
    {
        int ft = fields.at(i);
        QString key = ConfiguratorEnums::xmlAttrNameByType(ft);
        if (ConfiguratorEnums::isDoubleField(ft))
        {
	    QString value = widgetByKey(key)->edit->text().trimmed();
	    double d = value.toDouble(&ok);
	    if (!ok || d < 0) {err = tr("Invalid page value [%1]").arg(key); return;}
	}
    }

    QString key = ConfiguratorEnums::xmlAttrNameByType(ftDateOperation);
    QString value = widgetByKey(key)->edit->text().trimmed();
    QDate dt = QDate::fromString(value, "dd.MM.yyyy");
    if (!dt.isValid() || dt.isNull()) {err = tr("Invalid page value [%1]").arg(key); return;}
    if (dt > QDate::currentDate()) {err = tr("Invalid page value [%1]").arg(key); return;}

}



/////////////OperationEditDialog/////////////////////////
OperationEditDialog::OperationEditDialog(ConfiguratorAbstractRecord &rec, QWidget *parent)
    :OperationDialog(rec.record.value(ftTypeOperation).toInt(), rec, parent)
{
    
    setWindowTitle(QString("%1 (editing record)").arg(windowTitle()));

}
void OperationEditDialog::init()
{
//    qDebug("OperationEditDialog::init()");
    OperationDialogBase::init();
    recToPage();
    connectSignalsForCalc();
}
void OperationEditDialog::recToPage()
{
    int field = ftCompany;
    int id = m_record.record.value(field).toInt();
    QString key = ConfiguratorEnums::xmlAttrNameByType(field);
    const SimpleWidget *sw = widgetByKey(key);
    int pos = sw->comboBox->findData(id);
    if (pos < 0) return;
    sw->comboBox->setCurrentIndex(pos);
    sw->comboBox->setEnabled(false);


    field = ftKKS;
    QString kks = m_record.record.value(field);

    field = ftPaperType;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    sw = widgetByKey(key);
    pos = kks.isEmpty() ? 0 : 1; 
    sw->comboBox->setCurrentIndex(pos);
    sw->comboBox->setEnabled(false);
    
    if (!kks.isEmpty())
    {
//	qDebug()<<QString("kks = %1").arg(kks);
	field = ftKKS;
	key = ConfiguratorEnums::xmlAttrNameByType(field);
	sw = widgetByKey(key);
	pos = sw->comboBox->findText(kks);
	if (pos < 0) return;
	sw->comboBox->setCurrentIndex(pos);
    }
    
    field = ftCount;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    sw = widgetByKey(key);
    pos = m_record.record.value(field).toInt() - 1;
    sw->comboBox->setCurrentIndex(pos);


    field = ftCouponSize;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    double d = m_record.record.value(field).toDouble();
    setWidgetValue(key, d);

    field = ftPrice1;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    d = m_record.record.value(field).toDouble();
    setWidgetValue(key, d);

    field = ftCommission;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    d = m_record.record.value(field).toDouble();
    setWidgetValue(key, d);

    field = ftPrice;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    d = m_record.record.value(field).toDouble();
    setWidgetValue(key, d);

    field = ftDateOperation;
    key = ConfiguratorEnums::xmlAttrNameByType(field);
    setWidgetValue(key, m_record.record.value(field));
}








