 #ifndef MAINFORM_H
 #define MAINFORM_H 

 #include "lmainwidget.h"
 #include "lchildwidget.h"
 #include "ui_mainwidget.h"


class QTabWidget;
class QListWidget;
class QLabel;
class QSplitter;

enum BStages {bStoped = 0, bRunning, bPaused};


struct Bond
{
    Bond() {reset();}
    
    int week; //номер недели
    int was_coupons; //количество раз всех выплат

    void reset() {week = -1;  was_coupons = 0;}
    void nextPay() {was_coupons++;}
};

struct BondsState
{
    BondsState() {reset();}

    QList<Bond> bonds;

    int coupon_size;
    int add_week_bonds;
    
    int cur_period;
    int cur_week;
    int space_sum;
    

    
    void reset() {coupon_size = 0; add_week_bonds = 0; cur_period = 0; cur_week = 0; space_sum = 0; bonds.clear();}
    int allBonds() const {return bonds.count();}
    int bondsCountByWeek(int w) const //вернуть количество бондов по номеру недели
    {
	int x = 0;
	for (int i=0; i<bonds.count(); i++) 
	    if (bonds.at(i).week == w) x++;
	return x;
    }
    void addBond(int w) {Bond bond; bond.week = w; bonds.append(bond);}
    int currentWeekPay() const {return bondsCountByWeek(cur_week)*coupon_size;} //текущие выплаты на неделе
    void nextPays() //увеличить счетчик выплат у всех бондов с текущей недели
    {
	for (int i=0; i<bonds.count(); i++) 
	    if (bonds.at(i).week == cur_week) bonds[i].nextPay();
    }
    int allPaysNumber() const //вернуть текущее количетсво всех выплат по всем бондам
    {
	int x = 0;
	for (int i=0; i<bonds.count(); i++) 
	    x += bonds.at(i).was_coupons;
	return x;
    }



};


// MainWidget
class BoundsCentraWidget: public LChildWidget, public Ui::mainWidget
{
    Q_OBJECT
public:
    BoundsCentraWidget(QWidget *parent = 0);
    virtual ~BoundsCentraWidget() {}

    void save(QSettings&);
    void load(QSettings&);
    
    inline int stage() const {return m_stage;}
    inline void setStage(int a) {m_stage = a; updateByStage();}

protected:
    QSplitter *m_splitter1;
    QSplitter *m_splitter2;
    QSplitter *m_splitter3;
    QTimer *m_timer;
    int m_stage;
    bool need_clear;
    BondsState m_state;
    
    void initStateTable();
    void initSplitters();


    void updateByStage();
    void enableControls(bool);
    void startTest();
    void stopTest();
    void resetTables();
    void updateTables(int, int, int);

    void setNextWeek(); //увеличивает счетчик недели/периода в m_state
//    void getWeekPays(); //получить очередные купоны
    void addNextBonds(); //добавить новые бонды в контейнер
    int periodWeeks() const; //количество недель в периоде
    int minWeekBonds() const; //номер недели с минимальным количеством бондов 


protected slots:
    void slotNextWeek();

signals:
    void signalFinish();
    void signalNextWeek(int, int);

};



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);


protected:
    BoundsCentraWidget *m_boundsWidget;

    QString projectName() const {return "bonds";}
    virtual void load();
    virtual void save();

    void initActions();
    void initWidgets();
    void initCommonSettings();

    QString mainTitle() const;
    void updateButtons();
    void stop();
    void start();
    void pause();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotStop() {stop();}
    void slotUpdateWindowTitle(int, int);

/*
    virtual void slotAppSettingsChanged(QStringList);
    void slotDataChanged(const QStringList&);

signals:
    void signalExec(int);

*/
    
};
 

 #endif






