

INCLUDEPATH *= $$PWD

# include librt (linux)
DEPENDPATH += $$PWD/lib64
unix:!macx: LIBS += -L$$PWD/lib64/ -lrt


# Input
HEADERS += $$PWD/mq.h \
        $$PWD/mqworker.h

SOURCES += $$PWD/mq.cpp \
        $$PWD/mqworker.cpp


