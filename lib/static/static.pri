
QT += widgets testlib xml

INCLUDEPATH *= $$PWD

FORMS += $$PWD/lsimpledialog.ui


# Input
HEADERS += $$PWD/lfile.h \
        $$PWD/lsimpledialog.h \
        $$PWD/lstatic.h \
        $$PWD/lprotocol.h


SOURCES += $$PWD/lfile.cpp \
        $$PWD/lsimpledialog.cpp \
        $$PWD/lstatic.cpp \
        $$PWD/lprotocol.cpp




