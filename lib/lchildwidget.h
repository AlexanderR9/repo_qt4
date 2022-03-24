 #ifndef LCHILDWIDGET_H
 #define LCHILDWIDGET_H

 #include <QWidget>
 #include <QSettings>
 #include <QMessageBox>


// LChildWidget
class LChildWidget: public QWidget
{
    Q_OBJECT
public:
    LChildWidget(QWidget *parent = 0) :QWidget(parent), m_err(QString()) {}
    virtual ~LChildWidget() {}

    virtual void save(QSettings&) {}
    virtual void load(QSettings&) {}
    inline bool isErr() const {return !m_err.isEmpty();}
    void showErr() {QMessageBox::critical(this, name(), m_err); m_err.clear();}
    virtual QString name() const {return QObject::tr("Child widget name!!!");}
    virtual QString iconPath() const {return QString();}

public slots:
    virtual void slotExec(int) {}

protected:
    QString m_err;

};



 #endif



