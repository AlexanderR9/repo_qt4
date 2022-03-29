 #ifndef DATASTRUCTS_H
 #define DATASTRUCTS_H


 #include <QMap>
 #include <QString>

class QStringList;


////////////////ConfiguratorAbstractRecord//////////////////////////
struct ConfiguratorAbstractRecord
{
	ConfiguratorAbstractRecord() :serial(-1) {reset();}
	ConfiguratorAbstractRecord(int a) :serial(a) {record.clear();}
	ConfiguratorAbstractRecord(const ConfiguratorAbstractRecord&);

	QMap<int, QString> record;
	int serial;

	QString value(int, QString defValue = QString()) const;
	void getValues(const QList<int>&, QStringList&) const;
	void copy(const ConfiguratorAbstractRecord&, bool with_serial = true);
	void reset();
	bool invalid() const {return (serial < 0 || record.isEmpty());}
	QList<int> fields() const {return record.keys();};
	bool hasField(int field) const {return fields().contains(field);}
	int size() const {return record.count();}
	bool isEmpty() const {return record.isEmpty();}

	//for out to debug func
	QString toString() const;

};


////////////////ConfiguratorAbstractData//////////////////////////
struct ConfiguratorAbstractData
{
	ConfiguratorAbstractData() :generalType(-1) {}
	ConfiguratorAbstractData(int t) :generalType(t) {}


	int generalType;
	QList<ConfiguratorAbstractRecord> records;
	QList<int> fields;

	static int serial;
	static int nextSerial() {serial++; return serial;}

	inline int count() const {return records.count();}
	inline void append(const ConfiguratorAbstractRecord &rec) {records.append(rec);}
	inline bool isEmpty() const {return records.isEmpty();}
	inline bool invalid() const {return (generalType < 0);}

	void reset();
	bool hasFieldValue(int, const QString&, int notCheckIndex = -1) const; //проверка на наличие записи в которой есть заданное поле с заданным значением
	const ConfiguratorAbstractRecord* recByFieldValue(int, const QString&) const;  //возвращает указатель на запись в которой есть заданное поле с заданным значением, если такой записи нет то вернет NULL
	QString recAtValue(int, int) const; // берется запись с заданным индексов и возвращается значение заданного поля, при ошибке пустая строка
	void setFieldValueToRecords(int, const QString&); //установка значения поля всем записям, если такого поля нет, то оно добавится
	void setEmptyRecord(ConfiguratorAbstractRecord&); //установка пустных значений всех полей в записи, а также присваевается следующий серийник ()
	int maxID(int field = 51) const; // возвращает максимальное значение поля ID из всех записей
	int indexOf(int, const QString&, int notCheckIndex = -1) const;  //возвращает индекс записи в которой есть заданое поле с заданным значением, если такой записи нет то вернет -1
	const ConfiguratorAbstractRecord& recAt(int i) const {return records.at(i);} 
	void setRecordValue(int, int, const QString&);
	void replaceRecords(int, int);
	void addOtherData(const ConfiguratorAbstractData&);

};

////////////////////////////////
struct CompanyInfo
{
    CompanyInfo() {reset();}

    ConfiguratorAbstractRecord rec;
    QMap<int, QString> bond_kks;
    bool has_cfd; 

    QString toStr() const;
    void reset() {has_cfd = false; bond_kks.clear(); rec.reset();}    
    int id() const;// {return rec.record.value(ftID, QString("-1")).toInt();}

};


 #endif



