QT       += core gui
qt       += quick quick3d

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Antioch2.cpp \
    BMPandPNGConversion.cpp \
    BinChanger.cpp \
    Database.cpp \
    DatabaseItems.cpp \
    ITFtoBMP.cpp \
    LevelGeo.cpp \
    Mesh.cpp \
    ToneLibrary.cpp \
    VBINtoSTL.cpp \
    main.cpp \
    mainwindow.cpp \
    openfile.cpp \
    stolenunswizzler.cpp

HEADERS += \
    Antioch2.h \
    BinChanger.h \
    Database.h \
    DistanceCalculations.h \
    LevelGeo.h \
    Mesh.h \
    ToneLibraries.h \
    itf.h \
    mainwindow.h \
    vbin.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

