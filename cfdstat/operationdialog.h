 #ifndef OPERATION_DIALOG_H
 #define OPERATION_DIALOG_H

 #include <QDialog>
 #include "lsimpledialog.h" 
 #include "datastructs.h"

struct ConfiguratorAbstractRecord;


///////////////////OperationDialogBase/////////////////////////////
class OperationDialogBase : public LSimpleDialog
{
    Q_OBJECT
public:
    OperationDialogBase(ConfiguratorAbstractRecord&, QWidget *parent = 0);
    virtual ~OperationDialogBase() {}

    virtual void init();

    static void getCompanyInfo(int, CompanyInfo&);

protected:
    CompanyInfo cur_company;

    ConfiguratorAbstractRecord& m_record;
    ConfiguratorAbstractData  m_countryData;
    ConfiguratorAbstractData  m_companyData;

    virtual void load();
    virtual void save();

    virtual void initPageByData() = 0;    
    virtual void checkPageValues(QString&) = 0;
    virtual QList<int> fieldsByType() const = 0;
    virtual void prepareKKSList(); //при смене компании заполнить список ккс

    virtual void fillResultRecord();
    virtual void placeSubWidgets();
    virtual void initCompanyList();
    virtual void initPaperTypeList();
    virtual void updateEnabled();
    virtual void finishedKKS();

    bool isCompanyType() const;
    int currentCompany() const;
    void readGeneralData();
    QString companyIcon(const QString&) const;
    void setCompanyIcons();


protected slots:
    virtual void slotApply();
    virtual void slotCompanyChanged();  
    virtual void slotGetCurrentPrice();
    virtual void slotPaperTypeChanged();
  
signals:
    void signalGetCurrentPrice(const ConfiguratorAbstractRecord&, double&);

private:
    static int company_index;

};

///////////////////OperationDialog/////////////////////////////
class OperationDialog : public OperationDialogBase
{
    Q_OBJECT
public:
    OperationDialog(int, ConfiguratorAbstractRecord&, QWidget *parent = 0);
    virtual ~OperationDialog() {}

    virtual void init();

protected:
    int m_type;

    void initPageByData();    
    QList<int> fieldsByType() const;
    void checkPageValues(QString&);
    virtual void connectSignalsForCalc();

protected slots:
    void slotCalcSum();

};

///////////////////UpdatePricesDialog/////////////////////////////
class UpdatePricesDialog : public OperationDialogBase
{
    Q_OBJECT
public:
    UpdatePricesDialog(ConfiguratorAbstractRecord&, QWidget *parent = 0);
    virtual ~UpdatePricesDialog() {}


protected:
    QList<int> fieldsByType() const;

    void checkPageValues(QString&);
    void initPageByData();

};

///////////////////EditPricesDialog/////////////////////////////
class EditPricesDialog : public UpdatePricesDialog
{
    Q_OBJECT
public:
    EditPricesDialog(ConfiguratorAbstractRecord&, QWidget *parent = 0);
    virtual ~EditPricesDialog() {}

    virtual void init();

protected:
    void load() {}
    void save() {}
    void recToPage();

};

///////////////////OperationDialog/////////////////////////////
class OperationEditDialog : public OperationDialog
{
    Q_OBJECT
public:
    OperationEditDialog(ConfiguratorAbstractRecord&, QWidget *parent = 0);
    virtual ~OperationEditDialog() {}

    virtual void init();

protected:
    void load() {}
    void save() {}
    void recToPage();

protected slots:
    virtual void slotGetCurrentPrice() {}

};



 #endif


