INCLUDEPATH *= $$PWD \
            $$PWD/images 


####### pri includes ################
include($$PWD/math/simplemath.pri)
include($$PWD/static/static.pri)


# Input
HEADERS += $$PWD/lmainwidget.h \
	    $$PWD/lchildwidget.h \
	    $$PWD/lchart.h \
	    $$PWD/lcommonsettings.h \
	    $$PWD/lsimpleobj.h


SOURCES += $$PWD/lmainwidget.cpp \
	    $$PWD/lchart.cpp \
	    $$PWD/lcommonsettings.cpp \
	    $$PWD/lsimpleobj.cpp
                            
RESOURCES += $$PWD/icons.qrc


