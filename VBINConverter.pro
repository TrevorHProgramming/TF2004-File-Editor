QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Source/Models/Antioch2.cpp \
    Source/Textures/BMPandPNGConversion.cpp \
    Source/Main/BinChanger.cpp \
    Source/Databases/Database.cpp \
    Source/Databases/DatabaseItems.cpp \
    DistanceCalculator.cpp \
    Source/Textures/ITF.cpp \
    Source/Models/LevelGeo.cpp \
    Source/Models/Mesh.cpp \
    Source/Audio/ToneLibrary.cpp \
    Source/Models/VBIN.cpp \
    Source/Main/main.cpp \
    Source/Main/mainwindow.cpp \
    Source/Main/openfile.cpp \
    stolenunswizzler.cpp

HEADERS += \
    Headers/Models/Antioch2.h \
    Headers/Main/BinChanger.h \
    Headers/Databases/Database.h \
    DistanceCalculator.h \
    Headers/Models/LevelGeo.h \
    Headers/Models/Mesh.h \
    Headers/Audio/ToneLibraries.h \
    Headers/Textures/itf.h \
    Headers/Main/mainwindow.h \
    Headers/Models/vbin.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

