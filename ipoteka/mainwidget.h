 #ifndef MAINWIDGET_H
 #define MAINWIDGET_H

 #include "qwidget.h"
 #include "icalc.h"
 #include "ui_mainwidget.h"    

class MainWidget : public QWidget, public Ui::MainWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = NULL);
    virtual ~MainWidget() {}

protected:
    void addRow(const IRecord&);
    void calcFirstPaymentByCreditParameters();
    void fillFirstPaymentParameters();
    void setCreditParameters();
    void setResultLabel(const QString s = QString()) {resultLabel->setText(QString("%1: %2").arg(QString::fromUtf8("Итог")).arg(s));}
    
    void save();
    void load();
    void closeEvent(QCloseEvent*) {save();}

protected slots:
    void clearTable();
    void slotChangeCreditParameters();
    void slotChangeMorePayment();
    void slotChangeFPPercent();
    void slotChangeFPSum();
    void slotChangeSumCredit();
    void slotCalc();

    
private:
    ICalc icalc;
    
    
};





 #endif

