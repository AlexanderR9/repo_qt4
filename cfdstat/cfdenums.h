 #ifndef CFD_ENUMS_H
 #define CFD_ENUMS_H

 #include <QList>
 #include <QString>
 #include <QObject>
 #include <QStringList>


//после добавления элемента в данное множество необходимо дописать функции:
//generalDataTypes, interfaceTextByType, xmlNodeNameByType, GeneralDataFileReader::xmlFileByType
enum ConfiguratorGeneralData {gdCFD = 1, gdBond, gdBag, gdHistory, gdPrices, gdCompany, gdCurrency, gdCountry, gdBranch, gdCouponCount, gdRating, gdDivCalendar};

//после добавления элемента в данное множество необходимо дописать функции:
//fieldTypes, isIntField, interfaceTextByType, defFieldValue, xmlAttrNameByType, isUniqueField
enum ConfiguratorFieldType {ftID = 51, ftName, ftShortName, ftKKS, ftCurrency, ftColor, ftCommission, ftDateOffer, ftDateCoupon, ftCouponSize, 
				ftCouponCount, ftADiv, ftNalogSize, ftRating, ftDebt, ftMarket, ftPDiv, ftCount, ftPrice1, ftPrice, 
				ftPaperType, ftDateOperation, ftTypeOperation, ftNote, ftCompany, ftCountry, ftBranch, ftPayedSize, ftDifference, ftDivDelay,
				ftDivSize, ftNalogP, ftSumSize};

//после добавления элемента в данное множество необходимо дописать функции: interfaceTextByType
enum ContextActionType {catAdd = 151, catEdit, catClone, catDelete, catHide, catHideOther, catExpand, catCollapse, catDown, catUp, catLoad, catClear};


enum OperationType {opBuy = 301, opSell, opCouponReceive, opDivReceive, opCouponAndDiv};




// static funcs
class ConfiguratorEnums
{
public:
	static QList<int> paramDataTypes();
	static QList<int> fieldTypes();
	static QList<int> generalDataTypes();
	static QList<int> operationTypes();

	static QList<int> invisibleGeneralDataTypes();
	static QList<int> widgetTypes();
	static QList<int> groupChildsItemTypes();
	static bool isUniqueField(int);
	static bool isIntField(int);
	static bool isDoubleField(int);
	static bool isFixField(int);
//	static QList<int> fixFieldValues(int);
	static bool needTranslateField(int);
	static QString interfaceTextByType(int);
	static QString defFieldValue(int);
	static QString imageByContextType(int);
	static QString xmlAttrNameByType(int);
	static QString xmlNodeNameByType(int);
	static QString xmlRootNodeName() {return QString("table");}
	static QString xmlFieldsNodeName() {return QString("fields");}
	static QString xmlRecordsNodeName() {return QString("records");}
	static QString xmlRecordNodeName() {return QString("record");}
	static QString xmlFieldNodeName() {return QString("field");}
	static QString xmlTrAttrName() {return QString("tr");}
	static QString xmlInputAttrName() {return QString("input");}
	static QString errorType() {return QString("???");}
	static int typeByXmlNodeName(const QString&);
	static int typeByXmlAttrName(const QString&);
	static int generalTypeByFieldType(int);
	static bool hasNoneItem(int);
	static QString xmlFileByType(int);
	static double errLimitValue() {return -9999;}

};


 #define MSGBOX_ERR_TITLE		QObject::tr("Error!")
 #define MSGBOX_WARNING_TITLE		QObject::tr("Warning!")
 #define MSGBOX_DELETEDATA_TITLE	QObject::tr("Deleting data!")
 #define MSGBOX_REWRITEFILE_TITLE	QObject::tr("Rewrite file!")
 #define MSGBOX_READEDFILE_TITLE	QObject::tr("Readed file!")
 #define MSGBOX_ADD_TITLE		QObject::tr("Add!")
 #define DATE_MASK			QString("dd.MM.yyyy")



 #endif



