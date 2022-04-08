 #ifndef CFT_MAINWIDGET_H
 #define CFT_MAINWIDGET_H

 #include "qwidget.h"
 #include "ui_mainwidget.h"
 #include "qfile.h"
 #include "qdebug.h"


 #define SYMBOL		"_"
 #define SET		"*"	
 #define SIMPLE		"X"	
 #define NUMERIC	"9"
 #define EMPTY		"[]"

 class QSplitter;


///////struct MaskInfo///////////////////////////
struct MaskInfo
{
    enum SymbolType  {SimpleType = 0, OneType, SetType, NumericType, EmptyType, UnknowType};

    MaskInfo(QString s) :orig_mask(s.trimmed())
    {
		if (orig_mask.isEmpty()) return;

		if (isEmptyType())
		{
			symbAddNext(EmptyType);
			return;
		}
		parseInputMask();
		parseTextParts();
    }

    QString orig_mask; // исходное значение маски, для парса (ABcv__[  02 -07]*fg)
    QMap<uint, SymbolType> symbols;// описание символов форматированой(реальной) маски. ключ очередной индекс маски
    QString format; // абстрактный вид маски типа X___X*X_[] , служит для сравнения с другими масками, символов "_" дожно быть столькоже сколько в оригинале, остальный в еденичном экземпляре
    QMap<int, QString> textParts; // описание обычных текстовых фрагментов, ключ - начальная позиция фрагмента, значение - сам фрагмент
    QList<int> numericInfo; // 4 значения: индекс фрагмента, размер фрагмента, начальное значение, сколько значений

    bool isNumeric() const {return !numericInfo.isEmpty();}
    bool isEmptyType() const {return (orig_mask == EMPTY);}
    void symbAddNext(SymbolType type) {symbols.insert(symbols.count(), type);}
    void parseInputMask()
    {
		for (int i=0; i<orig_mask.count(); i++)
		{
			if (orig_mask.mid(i, 1) == SYMBOL)
			{
				if (i>0 && symbols.values().last() == SetType) {symbols.clear(); return;}
				symbAddNext(OneType);
			}
			else if (orig_mask.mid(i, 1) == SET)
			{
				if (i>0 && symbols.values().last() == SetType) {symbols.clear(); return;}
				if (i>0 && symbols.values().last() == OneType) {symbols.clear(); return;}
				symbAddNext(SetType);
			}
			else if (orig_mask.mid(i, 1) == "[")
			{
				if (isNumeric()) {symbols.clear(); return;} // фрагмент типа NumericType может встречатся только один раз

				int pos2 =  orig_mask.indexOf("]");
				if (pos2<0 || (pos2-i)<4) {symbols.clear(); return;}
				QString ns(orig_mask.mid(i+1, pos2-i-1));
				int posd = ns.indexOf("-");
				if (posd<0) {symbols.clear(); return;}
				QString ns1(ns.left(posd).trimmed());
				QString ns2(ns.right(ns.length()-posd-1).trimmed());
				if (ns1.isEmpty() || ns2.isEmpty() || ns1.length() != ns2.length()) {symbols.clear(); return;}

				bool ok;
				int n1 = ns1.toInt(&ok); if (!ok) {symbols.clear(); return;}
				int n2 = ns2.toInt(&ok); if (!ok) {symbols.clear(); return;}
				if (n1<0 || n2<0 || n1>=n2) {symbols.clear(); return;}
				numericInfo.append(i);
				numericInfo.append(ns1.length());
				numericInfo.append(n1);
				numericInfo.append(n2-n1+1);

				for (int k=0; k<ns1.length(); k++) symbAddNext(NumericType);//symbols.insert(i+k, NumericType);
				i=pos2;
			}
			else symbAddNext(SimpleType);
		}
    }
    void parseTextParts()
    {
    	if (!isValid()) return;
    	int num_space = 0;
    	for (int i=0; i<symbols.keys().count(); i++)
    	{
    		switch(symbols.value(symbols.keys().at(i)))
    		{
				case SimpleType:
				{
					int j=i;
					while(symbols.value(symbols.keys().at(j)) == SimpleType)
					{
						j++;
						if (symbols.count() <= j) break;
					}
					format+=SIMPLE;
					textParts.insert(i, orig_mask.mid(i+num_space, j-i));
					i=j-1;
					break;
				}
				case OneType:
				{
					int j=i;
					while(symbols.value(symbols.keys().at(j)) == OneType)
					{
						format+=SYMBOL;
						j++;
					}
					i=j-1;
					break;
				}
				case SetType:
				{
					format+=SET;
					break;
				}
				case NumericType:
				{
					format+=NUMERIC;
					i+=numericInfo.at(1)-1;
					num_space = orig_mask.indexOf("]") - orig_mask.indexOf("[") + 1 - numericInfo.at(1);
					break;
				}

    			default: break;
    		}
    	}
    }
    bool isValid() const
    {
    	if (orig_mask.isEmpty() || symbols.count()==0) return false;
		if (symbols.values().first() == OneType || symbols.values().last() == OneType)  return false;
		if (symbols.values().first() == SetType || symbols.values().last() == SetType)  return false;
		return true;
    }	
    bool isSimple() const
    {
		if (!isValid()) return false;
		if (symbols.values().contains(OneType)) return false;
		if (symbols.values().contains(SetType)) return false;
		if (isNumeric()) return false;
		return true;
    }
    QString partMaskTextByPos(int pos) const //фрагмент из textParts который начинается с позиции pos в маске
    {
    	if (!isValid()) return QString();
    	if (!textParts.contains(pos)) QString();
    	return textParts.value(pos);
    }
    QString partMaskText(int i) const //фрагмент из textParts с порядковым номером i
    {
		if (!isValid()) return QString();
		if (i >= textParts.count() || i < 0) return QString();
		return textParts.values().at(i);
    }
    void out() const
    {
		qDebug("\n**** info mask ****");
		if (!isValid()) {qDebug()<<QString("invalid mask - %1").arg(orig_mask); return;}
		qDebug()<<QString("mask[%1]  format[%2]  parts[%3]  simple[%4]  numeric[%5]").arg(orig_mask).arg(format).arg(textParts.count()).
				arg(isSimple()?"yes":"no").arg(isNumeric()?"yes":"no");

		qDebug("\nParts:");
		for (int i=0; i<textParts.count(); i++)
				qDebug()<<QString("i=%1.  index=%2  len=%3  text=%4").arg(i).arg(textParts.keys().at(i)).arg(textParts.values().at(i).length()).arg(partMaskText(i));
		qDebug("\nSymbols:");
		for (int i=0; i<symbols.count(); i++)
				qDebug()<<QString("i=%1.  %2  %3").arg(i).arg(symbols.keys().at(i)).arg(symbols.values().at(i));

		if (isNumeric())
			qDebug()<<QString("\nNumeric info: index %1,  size %2,  fisrt value %3,  values number %4").arg(numericInfo.at(0)).arg(numericInfo.at(1)).arg(numericInfo.at(2)).arg(numericInfo.at(3));

		qDebug("*********************\n");
    } 


};



////////////// class MainWidget //////////////////////////////////////
class CFT_MainWidget : public QWidget, public  Ui::mainWidget
{
    Q_OBJECT
public:
	CFT_MainWidget(QWidget *parent = NULL);
    virtual ~CFT_MainWidget() {}

    void addProtocol(QString s = QString(), bool isErr = false);


protected slots:
	void slotDir();
	void slotFile();
    void slotExec();
    void slotClickedFile() {toEndCheckBox->setEnabled(modifFilenameCheckBox->isChecked());}

protected:
    void tryChange(QFile&, QString codec = "utf8");
    QMap<int, int> findOf(const QString&);
    void parseStringByMask(QMap<int, int>&, const MaskInfo&, const QString&);
    void changeString(QString&, const QMap<int, int>&);
    void changeFile(const QString&, QFile&, QString);
    void outFoundStrings(const QMap<int, int>&, const QString&);
    void compareMasks(const MaskInfo&, const MaskInfo&, bool &ok);

    //new funcs
    void load();
    void save();
    void closeEvent(QCloseEvent*) {save();}
    QString companyName() const {return QString("NRCKI");}
    QString projectName() const {return QString("change_file_text");}
    void initWidgets();
    QString curDir() const;
    ////////////////////////////////////////////////////////////////

    int n_files;
    int n_find;
    int n_files_find;
    int n_changed;

    QSplitter *h_splitter;
    QSplitter *v_splitter;

private:
    void testCheckMaskStruct();

};




 #endif



