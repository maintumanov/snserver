#-------------------------------------------------
#
# Project created by QtCreator 2012-11-20T11:01:38
# Modyfed 23.01.2023
#-------------------------------------------------

QT       += core network serialport multimedia
QT       -= gui

TARGET = snserver
TEMPLATE = app
CONFIG   += console serialport
CONFIG   -= app_bundle
CONFIG += resources_big

SOURCES += main.cpp\
    qsnbsshapes.cpp \
    qsnweb.cpp \
    qsnwebadaptermqtt.cpp \
    qsnwebadaptermqttinput.cpp \
    qsnwebadaptermqttoutput.cpp \
    qsnwebadaptersynch.cpp \
    qsnwebitemabsencetime.cpp \
    qsnwebitemenergydevice.cpp \
    qsnwebitemmessage.cpp \
    qsnwebitempresencetime.cpp \
    qsnwebitemscheduleaction.cpp \
    qsnwebitemyeelightdevice.cpp \
    qsnwebitemyeelightscenario.cpp \
    qsnwebpage.cpp \
    qsnwebpage404.cpp \
    qsnwebpageabout.cpp \
    qsnwebpagealert.cpp \
    qsnwebpagecontrolpanel.cpp \
    qsnwebpagedaytime.cpp \
    qsnwebpagedoorbell.cpp \
    qsnwebpageenergy.cpp \
    qsnwebpagegraph.cpp \
    qsnwebpagegsm.cpp \
    qsnwebpagekeenetic.cpp \
    qsnwebpagelog.cpp \
    qsnwebpagelogconnector.cpp \
    qsnwebpagemail.cpp \
    qsnwebpagemessages.cpp \
    qsnwebpagenarodmon.cpp \
    qsnwebpagenighttime.cpp \
    qsnwebpagenotification.cpp \
    qsnwebpageowntracks.cpp \
    qsnwebpagepresence.cpp \
    qsnwebpagepzem.cpp \
    qsnwebpageschedule.cpp \
    qsnwebpagesecurityalarm.cpp \
    qsnwebpagesnirf.cpp \
    qsnwebpagesnirfmqtt.cpp \
    qsnwebpagesyslogconnector.cpp \
    qsnwebpageusers.cpp \
    qsnwebpagewifisniffer.cpp \
    qsnwebpageyeelight.cpp \
    qsnwebsocket.cpp \
    qsnwebcontainer.cpp \
    qsnwebwidgetcard.cpp \
    qsnwebwidgetcardbutton.cpp \
    qsnwebwidgetcardindicator.cpp \
    qsnwebwidgetcardindicatorcomp.cpp \
    qsnwebwidgetcardindicatorcompoutputs.cpp \
    qsnwebwidgetcardindicatormqtt.cpp \
    qsnwebwidgetcardindicatormqttalert.cpp \
    qsnwebwidgetcardindicatormqttstate.cpp \
    qsnwebwidgetcardindicatormsg.cpp \
    qsnwebwidgetcardindicatoroutput.cpp \
    qsnwebwidgetcardindicatorsnirf.cpp \
    qsnwebwidgetcardselector.cpp \
    qsnwebwidgetcardselectoritem.cpp \
    qsnwebwidgetcardswitch.cpp \
    qsnwebwidgetcardswitchmqtt.cpp \
    qsnwebwidgetcardswitchrm.cpp \
    qsnwebwidgetcardthermostatcontrol.cpp \
    qsnwebwidgetcardthermostatindicator.cpp \
    qsnwebwidgetcardtitle.cpp \
    qsnwebwidgetcardtitleswitch.cpp \
    qsnwebwidgetcardtwobuttons.cpp \
    qsnwebwidgetrmcodeinput.cpp \
    qsncanv5.cpp \
    qsninterface.cpp \
    qsnshapes.cpp \
    qsnwebadapter.cpp \
    qsnwebadaptercanv5.cpp \
    qsnwebauthorization.cpp \
    qsnwebadaptertcpserver.cpp \
    qsniotable.cpp \
    qsndb.cpp \
    qsnwebwidgetstartpage.cpp \
    qsnuartcan.cpp \
    qsnwebadapteruartcan.cpp \
    qsnring.cpp \
    qsnwebadapterring.cpp \
    qsnwebadapterudp.cpp \
    qsnudpclient.cpp \
    qsnlocations.cpp \
    qsnudpserver.cpp \
    qsntcpserver.cpp \
    qsnwebadapterudpserver.cpp \
    qsnwebsslserver.cpp \
    qsnwebsocketssl.cpp \
    qsnimageconvert.cpp \
    qsnregistry.cpp

HEADERS  += \
    qsnbsshapes.h \
    qsnweb.h \
    qsnwebadaptermqtt.h \
    qsnwebadaptermqttinput.h \
    qsnwebadaptermqttoutput.h \
    qsnwebadaptersynch.h \
    qsnwebitemabsencetime.h \
    qsnwebitemenergydevice.h \
    qsnwebitemmessage.h \
    qsnwebitempresencetime.h \
    qsnwebitemscheduleaction.h \
    qsnwebitemyeelightdevice.h \
    qsnwebitemyeelightscenario.h \
    qsnwebpage.h \
    qsnwebpage404.h \
    qsnwebpageabout.h \
    qsnwebpagealert.h \
    qsnwebpagecontrolpanel.h \
    qsnwebpagedaytime.h \
    qsnwebpagedoorbell.h \
    qsnwebpageenergy.h \
    qsnwebpagegraph.h \
    qsnwebpagegsm.h \
    qsnwebpagekeenetic.h \
    qsnwebpagelog.h \
    qsnwebpagelogconnector.h \
    qsnwebpagemail.h \
    qsnwebpagemessages.h \
    qsnwebpagenarodmon.h \
    qsnwebpagenighttime.h \
    qsnwebpagenotification.h \
    qsnwebpageowntracks.h \
    qsnwebpagepresence.h \
    qsnwebpagepzem.h \
    qsnwebpageschedule.h \
    qsnwebpagesecurityalarm.h \
    qsnwebpagesnirf.h \
    qsnwebpagesnirfmqtt.h \
    qsnwebpagesyslogconnector.h \
    qsnwebpageusers.h \
    qsnwebpagewifisniffer.h \
    qsnwebpageyeelight.h \
    qsnwebsocket.h \
    qsnwebcontainer.h \
    qsnwebwidgetbutton.h \
    qsnwebwidgetcard.h \
    qsnwebwidgetcardbutton.h \
    qsnwebwidgetcardindicator.h \
    qsnwebwidgetcardindicatorcomp.h \
    qsnwebwidgetcardindicatorcompoutputs.h \
    qsnwebwidgetcardindicatormqtt.h \
    qsnwebwidgetcardindicatormqttalert.h \
    qsnwebwidgetcardindicatormqttstate.h \
    qsnwebwidgetcardindicatormsg.h \
    qsnwebwidgetcardindicatoroutput.h \
    qsnwebwidgetcardindicatorsnirf.h \
    qsnwebwidgetcardselector.h \
    qsnwebwidgetcardselectoritem.h \
    qsnwebwidgetcardswitch.h \
    qsnwebwidgetcardswitchmqtt.h \
    qsnwebwidgetcardswitchrm.h \
    qsnwebwidgetcardthermostatcontrol.h \
    qsnwebwidgetcardthermostatindicator.h \
    qsnwebwidgetcardtitle.h \
    qsnwebwidgetcardtitleswitch.h \
    qsnwebwidgetcardtwobuttons.h \
    qsnwebwidgetpresence.h \
    qsnwebwidgetrmcodeinput.h \
    qsncanv5.h \
    qsninterface.h \
    qsnshapes.h \
    qsnwebadapter.h \
    qsnwebadaptercanv5.h \
    qsnwebauthorization.h \
    qsnwebwidgetlink.h \
    qsnwebadaptertcpserver.h \
    qsnglobalmodules.h \
    qsniotable.h \
    qsndb.h \
    qsnwebwidgetstartpage.h \
    qsnwebwidgetlabel.h \
    qsnwebwidgetchartline.h \
    qsnuartcan.h \
    qsnwebadapteruartcan.h \
    qsnring.h \
    qsnwebadapterring.h \
    qsnwebadapterudp.h \
    qsnudpclient.h \
    qsnlocations.h \
    qsnudpserver.h \
    qsntcpserver.h \
    qsnwebadapterudpserver.h \
    qsnwebsslserver.h \
    qsnwebsocketssl.h \
    qsnimageconvert.h \
    qsnregistry.h

RESOURCES += \
    qsnassets.qrc \
    qsnwebtranslate.qrc \
    qsnwebsounds.qrc \
    qsnwebsecurity.qrc

TRANSLATIONS = ServerLng_ru_RU.ts

win32 {
        RC_FILE += applico.rc
        OTHER_FILES += applico.rc
}
