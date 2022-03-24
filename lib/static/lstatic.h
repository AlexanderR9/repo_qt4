 #ifndef LSTATIC_H
 #define LSTATIC_H


 #include <QList>
 #include <QColor>
 #include <QTextEdit>
 #include <QGroupBox>
 #include <QDomElement>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QToolButton;
class QListWidget;
class QTableWidget;
class QLabel;
class QTimer;


///////////LSearch//////////////////////////
class LSearch: public QObject
{
	Q_OBJECT
public:
	struct LStructSearch
	{
		LStructSearch() :table(NULL), list(NULL), label(NULL), caption(QString()) {}
		LStructSearch(QLabel *l, QTableWidget *tw = NULL, QListWidget *lw = NULL, QString s = QString())
			:table(tw), list(lw), label(l), caption(s) {}

		QTableWidget 		*table;
		QListWidget  		*list;
		QLabel				*label;
		QString				 caption;

	};

	LSearch(const QLineEdit*, QObject *parent = NULL);
	virtual ~LSearch() {}

	void exec();
	void addTable(QTableWidget *tw, QLabel *l = NULL, QString s = QString());
	void addList(QListWidget *lw, QLabel *l = NULL, QString s = QString());
	void updateText();
	void setDelay(int);

	inline void setColIndex(int i) {m_colIndex = i;}


protected:
	const QLineEdit *m_edit;
	QTimer	*m_timer;
	QList<LStructSearch> 	m_structs;
	int	m_colIndex;
	int m_delay;
	bool m_timeout;

	int visibleRows(int) const;
	int rowCount(int) const;
	void setText(int);

protected slots:
	void slotSearch(const QString&);
	void slotTimer();


};


///////////LStatic//////////////////////////
class LStatic: public QObject
{
	Q_OBJECT
public:
	static QString curTime();
        static int defIconSize() {return 40;}


	//// for QTableWidget funcs
	static void addTableRow(QTableWidget*, const QStringList&, int align = Qt::AlignCenter, QColor cf = Qt::black, QColor cb = Qt::white);
	static void insertTableRow(int, QTableWidget*, const QStringList&, int align = Qt::AlignCenter, QColor cf = Qt::black, QColor cb = Qt::white);
	static void setTableRow(int, QTableWidget*, const QStringList&, int align = Qt::AlignCenter, QColor c = Qt::black);
	static void createTableItem(QTableWidget*, int, int, const QString&, int flags = Qt::ItemIsEnabled, int align = Qt::AlignCenter, QColor c = Qt::black);
	static void setTableHeaders(QTableWidget*, const QStringList&, int orintation = Qt::Horizontal);
	static void removeAllRowsTable(QTableWidget*);
	static void fullClearTable(QTableWidget*);
	static QList<int> selectedRows(QTableWidget*);
	static QList<int> selectedCols(QTableWidget*);
	static void createAllItems(QTableWidget*, int align = Qt::AlignCenter);
	static void clearAllItemsText(QTableWidget*);
	static void setTableRowColor(QTableWidget*, int, const QColor &c);
	static void resizeTableContents(QTableWidget*);	
	static void shiftTableRow(QTableWidget*, int row_index, int shift); //перемещает строку row_index на shift позиций, shift может быть меньше нуля

	//for strings
        //количество вхождений подстроки в строку
        static int subCount(const QString &s, const QString sub_s);
        //ищет позицию вхождения подстроки в строку, начиная С КОНЦА
        static int strIndexOfByEnd(const QString &s, const QString sub_s);
        //отрезает в строке n символов слева и возвращает новую результирующую строку
        static QString strTrimLeft(const QString &s, int n);
        //отрезает в строке n символов справа и возвращает новую результирующую строку
        static QString strTrimRight(const QString &s, int n);
        //ищет и возвращает подстроку в строке, которая находится между двумя заданными подстроками (например между "(" и ")")
        static QString strBetweenStr(const QString &s, QString s1, QString s2);
	//преобразует русский текст в unicode через заданный кодек, после чего его можно вывести на экран или записать в файл
	static QString fromCodec(const QString&, QString codec = "utf8");
	//преобразует QColor в строку типа (r; g; b)
	static QString fromColor(const QColor&, QString split_symbol = ";");
	//преобразует строку в QColor, если строка не корректна, то вернет QColor(0, 0, 0)
	static QColor strToColor(const QString&, QString split_symbol = ";");
	//преобразует QByteArray в строку типа: AA BB 12 FD ....., line_size задает через сколько байт вставлять '\n'
	static QString baToStr(const QByteArray&, int line_size = 8);


	static void setAttrNode(QDomElement&, QString a1, QString v1, QString a2 = QString(), QString v2 = QString(),
	    QString a3 = QString(), QString v3 = QString(), QString a4 = QString(), QString v4 = QString(),
	    QString a5 = QString(), QString v5 = QString());

	static int getIntAttrValue(const QString&, const QDomNode&, int defValue = -99); //get integer value of attribute, params: attr_name, node, default_value
	static QString getStringAttrValue(const QString&, const QDomNode&, QString defValue = QString()); //get string value of attribute, params: attr_name, node, default_value
        static void createDomHeader(QDomDocument&); // for create first section xml node



	
};


///////////LSplash//////////////////////////
class LSplash: public QWidget
{
    Q_OBJECT
public:
    LSplash(QWidget *parent = 0);
    virtual ~LSplash() {}

    void startDelay(const QString &s = QString());
    void stopDelay();

protected:
    QLabel *m_label;


};




#endif



