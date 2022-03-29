 #ifndef GENERALDATA_DIALOG_H
 #define GENERALDATA_DIALOG_H

 #include <QDialog>
 #include "lsimpledialog.h" 

class ConfiguratorSearch;
struct ConfiguratorAbstractRecord;




///////////////////GeneralDataRecordDialog/////////////////////////////
class GeneralDataRecordDialog : public LSimpleDialog
{
    Q_OBJECT
public:
    GeneralDataRecordDialog(int, ConfiguratorAbstractRecord&, QWidget *parent = 0);
    virtual ~GeneralDataRecordDialog() {}


protected:
    int m_type;
    ConfiguratorAbstractRecord &m_record;

    virtual void closeEvent(QCloseEvent*);
    void init();
    void toPage();
    void fromPage(QString&);
    void addNoneItem(QComboBox*);

protected slots:
    virtual void slotApply();
    //void slotBufferPathChanged(const QString&);


};


#endif


