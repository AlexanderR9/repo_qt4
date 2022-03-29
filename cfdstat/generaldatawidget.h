 #ifndef GENERALDATA_WIDGET_H
 #define GENERALDATA_WIDGET_H

 #include "ui_generaldatawidget.h"
 #include "datastructs.h"



class LSearch;
class QListWidgetItem;
class QDomNode;
class QDomElement;
class QAction;
class QContextMenuEvent;


////////////////ConfiguratorGeneralDataWdiget///////////////////////////////////////////
class ConfiguratorGeneralDataWdiget : public QWidget, public Ui::ConfiguratorGeneralDataWidget
{
    Q_OBJECT
public:
    ConfiguratorGeneralDataWdiget(QWidget *parent = 0);
    virtual ~ConfiguratorGeneralDataWdiget() {}

protected:
    LSearch		*m_search;
    ConfiguratorAbstractData	 m_data;
    QMap<int, QAction*>		 m_contextActions;
    int				 m_sortOrder;


	virtual void closeEvent(QCloseEvent*);
	void init();
	void enableControls(bool);
	void initListWidget();
	void loadDataToTable();
	void updateTable();
	void updateTableRow(int, const ConfiguratorAbstractRecord&);

	//context menu funcs
	void createContextActions();
	void contextMenuEvent(QContextMenuEvent*);
	bool visibleContextType(int) const;


protected slots:
	void slotTableChanged(QListWidgetItem*);
	void slotSort(int);
	void slotScrolling();

	//context slots
	void slotContextAdd();
	void slotContextEdit();
	void slotContextClone();
	void slotContextDelete();

public slots:
	void slotApply();


};


////////////////GeneralDataFileReader///////////////////////////////////////////
class GeneralDataFileReader
{
public:
    static QString xmlFileByType(int);
    static void loadDataFormFile(int, ConfiguratorAbstractData&);
    static void writeDataToFile(const ConfiguratorAbstractData&, QString&);

    static void addRecordFieldsToNode(const ConfiguratorAbstractRecord&, QDomElement&);
    static void fillRecordFieldsFromNode(ConfiguratorAbstractRecord&, const QDomNode&);


protected:
    static void loadFields(const QDomNode&, ConfiguratorAbstractData&);
    static void loadRecords(const QDomNode&, ConfiguratorAbstractData&, int);
    static void loadRecord(const QDomElement&, ConfiguratorAbstractRecord&);

    static void writeFields(QDomNode&, const ConfiguratorAbstractData&);
    static void writeRecords(QDomNode&, const ConfiguratorAbstractData&);
    static void writeRecord(QDomElement&, const ConfiguratorAbstractRecord&);



};



 #endif


