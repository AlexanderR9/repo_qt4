 #include "generaldatadialog.h"
 #include "lcommonsettings.h"
 #include "lstatic.h"
 #include "cfdenums.h"
 #include "datastructs.h"
 #include "generaldatawidget.h"


 #include <QPushButton>
 #include <QDialogButtonBox>
 #include <QDebug>
 #include <QComboBox>
 #include <QLineEdit>
 #include <QMessageBox>
 #include <QIntValidator>
 #include <QCheckBox>


/////////////GeneralDataRecordDialog/////////////////////////
GeneralDataRecordDialog::GeneralDataRecordDialog(int type, ConfiguratorAbstractRecord &rec, QWidget *parent)
	:LSimpleDialog(parent),
	 m_type(type),
	 m_record(rec)
{
	resize(500, 400);
	setBoxTitle(QObject::tr("Feilds values"));

	switch(m_type)
	{
		case catAdd: {setWindowTitle(QObject::tr("Adding record!")); break;}
		case catEdit: {setWindowTitle(QObject::tr("Editing record!")); break;}
		case catClone: {setWindowTitle(QObject::tr("Cloning record!")); break;}
		default: {ConfiguratorEnums::errorType(); return;}
	}


	init();
	setCaptionsWidth(150);
    addVerticalSpacer();
	setExpandWidgets();
	toPage();

}
void GeneralDataRecordDialog::init()
{
	QList<int> fields = m_record.record.keys();
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

	    if (ConfiguratorEnums::isFixField(fields.at(i)))
	    {
			addSimpleWidget(name, sdtStringCombo, key);
			QVariantList list;
			QVariantList user_data;
			ConfiguratorAbstractData g_data;
			GeneralDataFileReader::loadDataFormFile(ConfiguratorEnums::generalTypeByFieldType(ft), g_data);
			if (g_data.invalid()) break;
			for (int j=0; j<g_data.count(); j++)
			{
				if (ft == ftCouponCount) list.append(g_data.recAtValue(j, ftCount));
				else list.append(g_data.recAtValue(j, ftName));
				user_data.append(g_data.recAtValue(j, ftID));
			}
			setComboList(key, list, user_data);
			continue;
	    }


	    switch(ft)
	    {
			case ftCount:
			case ftID:
			{
				addSimpleWidget(name, sdtIntLine, key);
				break;
			}

			case ftNote:
			case ftImage:
			case ftKKS:
			case ftDateOffer:
			case ftDateCoupon:
			case ftDateOperation:
			case ftName:
			case ftShortName: {addSimpleWidget(name, sdtString, key); break;}

			case ftDivDelay:
			{
				addSimpleWidget(name, sdtIntCombo, key);
				QVariantList list;
				list << 1 << 3 << 5 << 7 << 14 << 21 << 30 << 45 << 60;
				setComboList(key, list);
				setWidgetValue(key, 7);
				break;
			}
			default: break;
	    }

	}
}
void GeneralDataRecordDialog::addNoneItem(QComboBox *cb)
{
	if (!cb) return;
	cb->addItem(ConfiguratorEnums::interfaceTextByType(0));
}
void GeneralDataRecordDialog::toPage()
{
	QList<int> fields = m_record.record.keys();
	for (int i=0; i<fields.count(); i++)
	{
	    int ft = fields.at(i);
	    QVariant value = m_record.value(ft);
	    QString key = ConfiguratorEnums::xmlAttrNameByType(ft);
	    switch (m_type)
	    {
		case catClone:
		case catEdit:
		{
		    if (ConfiguratorEnums::isFixField(fields.at(i)))
		    {
		        const SimpleWidget *sw = widgetByKey(key);
			if (sw)
			{
			    int pos = sw->comboBox->findData(m_record.value(ft).toInt());
			    if (pos >= 0) sw->comboBox->setCurrentIndex(pos);
			}

		    }
		    else setWidgetValue(key, value);
		    break;
		}
		case catAdd:
		{
		    setWidgetValue(key, ConfiguratorEnums::defFieldValue(ft));
		    break;
		}
		    default: break;
	    }
	}
}
void GeneralDataRecordDialog::fromPage(QString &err)
{
	err.clear();
	QList<int> fields = m_record.record.keys();
	for (int i=0; i<fields.count(); i++)
	{
	    int ft = fields.at(i);
	    QString key = ConfiguratorEnums::xmlAttrNameByType(ft);
	    QVariant value = widgetValue(key);
	    m_record.record.insert(ft, value.toString());

	    if (ConfiguratorEnums::isFixField(ft))
	    {
			const SimpleWidget *sw = widgetByKey(key);
			if (sw)
				m_record.record.insert(ft, QString::number(sw->comboBox->itemData(sw->comboBox->currentIndex()).toInt()));
	    }
	}
}
void GeneralDataRecordDialog::closeEvent(QCloseEvent*)
{
    //configuratorSettings.widgetsState.generalRecord = saveGeometry();
}
void GeneralDataRecordDialog::slotApply()
{
	QString err;
	fromPage(err);

	if (!err.isEmpty())
	{
		QMessageBox::critical(this, MSGBOX_ERR_TITLE, err);
		return;
	}

	LSimpleDialog::slotApply();
}


