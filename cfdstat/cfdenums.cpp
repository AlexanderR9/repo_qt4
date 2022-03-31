 #include "cfdenums.h"

 #include <QDebug>


/// static funcs class ConfiguratorEnums
QString ConfiguratorEnums::imageByContextType(int type)
{
    switch(type)
    {
    	case catAdd:					return QString("list-add.svg");
	case catEdit:					return QString("edit.svg");
	case catClone:					return QString("edit-copy.svg");
	case catDelete:					return QString("remove.svg");
	case catUp:					return QString("go-up.svg");
	case catLoad:
	case catDown:					return QString("go-down.svg");
	case catClear:					return QString("edit-clear.svg");

	default: break;
    }
    return QString();
}
bool ConfiguratorEnums::isFixField(int type)
{
    switch (type)
    {
	case ftRating:
//	case ftDivDelay:
	case ftCurrency:
	case ftCountry:
	case ftBranch:
	case ftCouponCount:
	case ftCompany: return true;
	default: break;
    }
    return false;
}
QList<int> ConfiguratorEnums::operationTypes()
{
    QList<int> list;
    list.append(opSell);
    list.append(opBuy);
    list.append(opCouponReceive);
    list.append(opDivReceive);
    list.append(opCouponAndDiv);
    return list;
}
QList<int> ConfiguratorEnums::fieldTypes()
{
	QList<int> list;
	list.append(ftID);
	list.append(ftName);
	list.append(ftShortName);
	list.append(ftKKS);
	list.append(ftColor);
	list.append(ftCurrency);
	list.append(ftCommission);
	list.append(ftDateOffer);
	list.append(ftDateCoupon);
	list.append(ftCouponCount);
	list.append(ftCouponSize);
	list.append(ftADiv);
	list.append(ftNalogSize);
	list.append(ftDivSize);
	list.append(ftNalogP);
	list.append(ftRating);
	list.append(ftPDiv);
	list.append(ftDivDelay);
	list.append(ftDebt);
	list.append(ftMarket);
	list.append(ftCount);
	list.append(ftPrice1);
	list.append(ftPrice);
	list.append(ftNote);
	list.append(ftPaperType);
	list.append(ftDateOperation);
	list.append(ftTypeOperation);
	list.append(ftCompany);
	list.append(ftCountry);
	list.append(ftBranch);
	list.append(ftPayedSize);
	list.append(ftSumSize);
	list.append(ftDifference);
	list.append(ftImage);

	return list;
}
QList<int> ConfiguratorEnums::generalDataTypes()
{
	QList<int> list;
	list.append(gdRating);
	list.append(gdCurrency);
	list.append(gdCountry);
	list.append(gdBranch);
	list.append(gdCouponCount);
	list.append(gdCompany);
	list.append(gdCFD);
	list.append(gdBond);
	list.append(gdDivCalendar);
	return list;
}
QList<int> ConfiguratorEnums::invisibleGeneralDataTypes()
{
	QList<int> list;
	return list;
}
bool ConfiguratorEnums::isUniqueField(int type)
{
	switch (type)
	{
		case ftID:
		case ftName:
		case ftKKS:
		case ftShortName: return true;
		default: break;
	}
	return false;
}
bool ConfiguratorEnums::isIntField(int type)
{
	switch (type)
	{
		case ftCount:
		case ftDivDelay:
		case ftTypeOperation:
		case ftID:
		case ftRating:
		case ftCouponCount: return true;
		default: break;
	}
	return false;
}
bool ConfiguratorEnums::isDoubleField(int type)
{
	switch (type)
	{
		case ftDebt:
		case ftPDiv:
		case ftMarket:
		case ftCommission:
		case ftADiv:
		case ftDivSize:
		case ftPrice1:
		case ftPrice:
		case ftPayedSize:
		case ftSumSize:
		case ftDifference:
		case ftNalogSize:
		case ftNalogP:
		case ftCouponSize: return true;
		default: break;
	}
	return false;
}
QString ConfiguratorEnums::interfaceTextByType(int type)
{
	switch (type)
	{
		case 0:			return QObject::tr("None");

		// fields
		case ftID: 		return QObject::tr("ID");
		case ftCount: 		return QObject::tr("Count");
		case ftName: 		return QObject::tr("Name");
		case ftShortName: 	return QObject::tr("Ticker");
		case ftKKS: 		return QObject::tr("KKS");
		case ftColor: 		return QObject::tr("Color");
		case ftCurrency: 	return QObject::tr("Currency");
		case ftCommission: 	return QString("Commission size");
		case ftDateOffer: 	return QString("Offer date");
		case ftDateCoupon:	return QString("Coupon date");
		case ftCouponCount:	return QString("Coupon count");
		case ftCouponSize:	return QString("Coupon size");
		case ftADiv: 		return QString("Div, %");
		case ftNalogSize:	return QString("Nalog size");
		case ftDivSize: 	return QString("Div size");
		case ftNalogP: 		return QString("Nalog, %");
		case ftRating: 		return QString("Rating");
		case ftDebt:		return QString("Debt factor");
		case ftPDiv:		return QString("Div payout, %");
		case ftDivDelay:	return QString("Div delay, days");
		case ftMarket:		return QString("Marcket, B");
		case ftPrice1:		return QString("Price (1 pcs.)");
		case ftPrice:		return QString("Price");
		case ftNote:		return QString("Note");
		case ftPaperType:	return QString("Paper type");
		case ftDateOperation:	return QString("Date of operation");
		case ftTypeOperation:	return QString("Operation type");
		case ftCompany:		return QObject::tr("Company");
		case ftCountry:		return QString("Country");
		case ftImage:		return QString("Image");
		case ftBranch:		return QString("Branch");
		case ftPayedSize:	return QString("Payed size");
		case ftSumSize:		return QString("Sum size");
		case ftDifference:	return QString("Difference");


		// general data
		case gdCompany:		return QObject::tr("Company");
		case gdRating:		return QObject::tr("Rating");
		case gdCFD:		return QObject::tr("CFD");
		case gdBond:		return QObject::tr("Bonds");
		case gdPrices:		return QObject::tr("Paper prices");
		case gdCurrency:	return QObject::tr("Currency");
		case gdCountry:		return QObject::tr("Country");
		case gdBranch:		return QObject::tr("Branch");
    		case gdCouponCount:	return QObject::tr("Coupon count");	
    		case gdDivCalendar:	return QObject::tr("Div calendar");	

		//context action type
		case catAdd:					return QObject::tr("add");
		case catEdit:					return QObject::tr("edit");
		case catClone:					return QObject::tr("clone");
		case catDelete:					return QObject::tr("delete");
		case catHide:					return QObject::tr("hide");
		case catHideOther:				return QObject::tr("hide other");
		case catExpand:					return QObject::tr("expand");
		case catCollapse:				return QObject::tr("collapse");
		case catUp:					return QObject::tr("up");
		case catDown:					return QObject::tr("down");
		case catClear:					return QObject::tr("Clear");
		case catLoad:					return QObject::tr("Load config");

		//operations
		case opSell: return QObject::tr("SELL");
		case opBuy: return QObject::tr("BUY");
		case opCouponReceive: return QObject::tr("COUPON");
		case opDivReceive: return QObject::tr("DIV");
		case opCouponAndDiv: return QObject::tr("COUPON & DIV");


		default: break;
	}
	return errorType();
}
QString ConfiguratorEnums::defFieldValue(int type)
{
	switch (type)
	{
		case ftID: 	return QString("-1");
		case ftTypeOperation: 	return QString("301");
		case ftCurrency: 	return QString("rub");
		case ftCouponCount: 	return QString("2");
		case ftRating: 	return QString("2");
		case ftDebt: 	return QString("0.1");
		case ftMarket: 	return QString("0.1");
		case ftPDiv: 	return QString("50");
		case ftColor:	return QString("#000000");


		default: break;
	}
	return QString();
}
QString ConfiguratorEnums::xmlAttrNameByType(int type)
{
	switch (type)
	{
		case ftID: 		return QString("id");
		case ftName:		return QString("name");
		case ftShortName: 	return QString("short_name");
		case ftKKS: 		return QString("kks");
		case ftColor: 		return QString("color");
		case ftCurrency: 	return QString("currency");
		case ftCommission: 	return QString("commission");
		case ftDateOffer: 	return QString("offer_date");
		case ftDateCoupon: 	return QString("coupon_date");
		case ftCouponCount: 	return QString("coupon_count");
		case ftCouponSize: 	return QString("coupon_size");
		case ftADiv: 		return QString("div");
		case ftDivDelay:	return QString("div_delay");
		case ftNalogSize: 	return QString("nalog_size");
		case ftDivSize:		return QString("div_size");
		case ftNalogP: 		return QString("nalog_p");
		case ftRating: 		return QString("rating");
		case ftDebt:		return QString("debt");
		case ftPDiv:		return QString("pdiv");
		case ftMarket:		return QString("market");
		case ftCount:		return QString("count");
		case ftPrice1:		return QString("price1");
		case ftPrice:		return QString("price");
		case ftNote:		return QString("note");
		case ftPaperType:	return QString("paper_type");
		case ftDateOperation:	return QString("date_operation");
		case ftTypeOperation:	return QString("type_operation");
		case ftCompany:		return QString("company");
		case ftCountry:		return QString("country");
		case ftImage:		return QString("image");
		case ftBranch:		return QString("branch");
		case ftPayedSize:	return QString("payed_size");
		case ftSumSize:		return QString("sum_size");
		case ftDifference:	return QString("difference");

		default: break;
	}
	return errorType();
}
int ConfiguratorEnums::typeByXmlNodeName(const QString &node_name)
{
	QList<int> list = ConfiguratorEnums::generalDataTypes();
	for (int i=0; i<list.count(); i++)
	{
		QString s = ConfiguratorEnums::xmlNodeNameByType(list.at(i)).trimmed();
		if (s.isEmpty() || s == ConfiguratorEnums::errorType()) continue;
		if (s == node_name) return list.at(i);
	}
	return -1;
}
int ConfiguratorEnums::typeByXmlAttrName(const QString &attr_name)
{
	QList<int> list = ConfiguratorEnums::fieldTypes();
	for (int i=0; i<list.count(); i++)
	{
		QString s = ConfiguratorEnums::xmlAttrNameByType(list.at(i)).trimmed();
		if (s.isEmpty() || s == ConfiguratorEnums::errorType()) continue;
		if (s == attr_name) return list.at(i);
	}
	return -1;
}
QString ConfiguratorEnums::xmlNodeNameByType(int type)
{
    switch (type)
    {
    	case gdRating:		return QString("rating");
    	case gdCFD:		return QString("cfd");
	case gdBond:		return QString("bond");
    	case gdBag:		return QString("bag");
	case gdHistory:		return QString("history");
	case gdPrices:		return QString("prices");
	case gdCompany:		return QString("company");
	case gdCurrency:	return QString("currency");
	case gdCountry:		return QString("country");
	case gdBranch:		return QString("branch");
    	case gdCouponCount:	return QString("coupon_count");	
    	case gdDivCalendar:	return QString("div_calendar");	
	default: break;
    }
    return errorType();
}
QString ConfiguratorEnums::xmlFileByType(int type)
{
    QString s = xmlNodeNameByType(type);
    if (s == errorType()) return QString();
    return QString("%1.xml").arg(s);
}
int ConfiguratorEnums::generalTypeByFieldType(int type)
{
    switch (type)
    {
	case ftRating: 		return gdRating;
	case ftCountry: 	return gdCountry;
	case ftCurrency: 	return gdCurrency;
	case ftBranch: 		return gdBranch;
	case ftCouponCount: 	return gdCouponCount;
	case ftCompany: 	return gdCompany;
	default: break;
    }
    return -1;
}












