#ifndef MQ_H
#define MQ_H
 
#include "lsimpleobj.h"
 
#include <QString>
#include <QColor>

struct mq_attr;
class QByteArray;

 
//класс для работы с одним экземпляром очереди posix 
class MQ : public LSimpleObject
{
    Q_OBJECT
public:
    enum MQState {mqsDeinit = 221,  mqsOpened, mqsClosed, mqsCreated, mqsInvalid};

    MQ(const QString&, QObject *parent = NULL);
    virtual ~MQ() {}
    
    inline QString name() const {return mq_name;}
    inline int size() const {return m_size;} 
    inline bool invalid() const {return (m_handle <= 0);}
    inline int handle() const {return m_handle;}
    inline bool isOpened() const {return (m_state == mqsOpened || m_state == mqsCreated);}
    
    
    QString strMode() const;
    QString strState() const;
    QString strStatus() const;
    QString strAttrs() const;
    QColor colorStatus() const;
    void updateAttrs();

    void tryOpen(int, bool&);
    void tryClose(bool&);
    void tryCreate(int, bool&);
    void tryDestroy(bool&);
    void trySendMsg(const QByteArray&, bool&);
    void tryReadMsg(QByteArray&); //в случае ошибки ba будет пустой

    	
protected:
    QString  	mq_name;   	
    int 		m_size;		//текущий размер очереди
    int			m_handle;	//дескриптор очереди posix
    int 		m_state;	// MQState enum element
    int 		m_mode; 	//IODevice enum element
    mq_attr 	*m_attrs; 	//current attributes
    
private:
    const char* charName() const; 
    int mqModeByMode() const;
    
};
 
#endif
 
  
