 #include "datastructs.h"
 #include "cfdenums.h"

 #include <QStringList>
 #include <QDebug>

int ConfiguratorAbstractData::serial = 0;


/////////////// ConfiguratorAbstractRecord //////////////////////////
ConfiguratorAbstractRecord::ConfiguratorAbstractRecord(const ConfiguratorAbstractRecord &rec)
	:serial(rec.serial)
{
	copy(rec);
}
QString ConfiguratorAbstractRecord::value(int field, QString defValue) const
{
	if (record.contains(field)) return record.value(field);
	return defValue;
}
QString ConfiguratorAbstractRecord::toString() const
{
	QString s("Record info:");
	QList<int> keys = fields();
	for (int i=0; i<keys.count(); i++)
		s = QString("%1  %2=[%3]").arg(s).arg(ConfiguratorEnums::xmlAttrNameByType(keys.at(i))).arg(record.value(keys.at(i)));
	return s;
}
void ConfiguratorAbstractRecord::getValues(const QList<int> &fields, QStringList &list) const
{
	for (int i=0; i<fields.count(); i++)
		list.append(record.value(fields.at(i), QString()));
}
void ConfiguratorAbstractRecord::copy(const ConfiguratorAbstractRecord &rec, bool with_serial)
{
	record.clear();
	if (with_serial) serial = rec.serial;

	QList<int> keys(rec.record.keys());
	for (int i=0; i<keys.count(); i++)
		record.insert(keys.at(i), rec.record.value(keys.at(i)));
}
void ConfiguratorAbstractRecord::reset()
{
	record.clear();
	serial = -1;
}



/////////////// CompanyInfo //////////////////////////
QString CompanyInfo::toStr() const
{
    QString s = QString("/////////////Company: id=%1  name=[%2]  ticker=[%3]///////////////////").arg(rec.record.value(ftID)).arg(rec.record.value(ftName)).arg(rec.record.value(ftShortName));
    s.append("\n");
    s = QString("%1 has_cfd=%2").arg(s).arg(has_cfd?"true":"false");
    s.append("\n");
    
    if (bond_kks.isEmpty())
    {
	s = QString("%1 bonds not found!").arg(s);
	s.append("\n");
	return s;    
    }


    QList<int> id_bonds(bond_kks.keys());
    for (int i=0; i<id_bonds.count(); i++)
	s = QString("%1 i=%2  key=%3  kks=%4  \n").arg(s).arg(i+1).arg(id_bonds.at(i)).arg(bond_kks.value(id_bonds.at(i)));    

    return s;
}
int CompanyInfo::id() const
{
    return rec.record.value(ftID, QString("-1")).toInt();
}


/////////////// ConfiguratorAbstractData //////////////////////////
void ConfiguratorAbstractData::reset()
{
    generalType = -1;
    records.clear();
    fields.clear();
}
void ConfiguratorAbstractData::addOtherData(const ConfiguratorAbstractData &data)
{
    records.append(data.records);

}
bool ConfiguratorAbstractData::hasFieldValue(int field, const QString &value, int notCheckIndex) const
{
	return (indexOf(field, value, notCheckIndex) >= 0);
}
int ConfiguratorAbstractData::indexOf(int field, const QString &value, int notCheckIndex) const
{
	for (int i=0; i<count(); i++)
	{
		if (notCheckIndex == i) continue;

		if (records.at(i).record.contains(field))
			if (records.at(i).record.value(field) == value)
				return i;
	}
	return -1;
}
const ConfiguratorAbstractRecord* ConfiguratorAbstractData::recByFieldValue(int field, const QString &value) const
{
	for (int i=0; i<count(); i++)
		if (records.at(i).record.contains(field))
				if (records.at(i).record.value(field) == value)
					return &records.at(i);
	return NULL;
}
QString ConfiguratorAbstractData::recAtValue(int index, int field) const
{
	if (index < 0 || index >= count()) return QString();
	return records.at(index).value(field);

}
void ConfiguratorAbstractData::setRecordValue(int i, int field, const QString &value)
{
    if (i < 0 || i >= count()) return;
    records[i].record.insert(field, value);
    
    

}
void ConfiguratorAbstractData::replaceRecords(int index1, int index2)
{
    if (index1 < 0 || index1 >= count()) return;
    if (index2 < 0 || index2 >= count()) return;
    if (index1 == index2) return;

    ConfiguratorAbstractRecord rec(recAt(index1));
    records[index1].copy(recAt(index2));
    records[index2].copy(rec);
}
void ConfiguratorAbstractData::setFieldValueToRecords(int field, const QString &value)
{
	for (int i=0; i<count(); i++)
	{
		records[i].record.insert(field, value);
	}
}
void ConfiguratorAbstractData::setEmptyRecord(ConfiguratorAbstractRecord &rec)
{
	rec.reset();
	rec.serial = ConfiguratorAbstractData::nextSerial();
	for (int i=0; i<fields.count(); i++)
		rec.record.insert(fields.at(i), QString());
}
int ConfiguratorAbstractData::maxID(int field) const
{
	bool ok;
	int max_id = 0;
	for (int i=0; i<count(); i++)
	{
		int id = records.at(i).value(field).toInt(&ok);
		if (!ok) continue;
		if (id > max_id) max_id = id;
	}
	return max_id;
}



