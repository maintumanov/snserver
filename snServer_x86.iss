; Имя приложения
#define   Name       "snServer"
; Версия приложения
#define   Version    "0.9.86.3"
; Фирма-разработчик
#define   Publisher  "SignalNet"

[Setup]
AppId={{228F6294-B721-4424-983A-2D39C03B19B9}
AppName={#Name}
AppVersion={#Version}
DefaultDirName={pf}\SignalNet\server
DefaultGroupName=SignalNet
UninstallDisplayIcon=favicon.ico
SetupIconFile=snServerDistributive.ico
Compression=lzma2
SolidCompression=yes
OutputDir=D:\SynologyDrive\snDistributive\
OutputBaseFilename=SetupServer {#Version} x86
AllowNoIcons=yes
;AppPublisherURL=http://signalnet.com
AppPublisher=SignalNet, LLC.
CreateUninstallRegKey=yes 

[Languages]
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"

[Files]
Source: "..\build-snServer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\snserver.exe"; DestDir: "{app}"

Source: "C:\Qt\5.15.2\mingw81_32\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\libstdc++-6.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\libwinpthread-1.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\Qt5Core.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\Qt5Network.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\Qt5SerialPort.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\Qt5Multimedia.dll"; DestDir: "{app}"
Source: "C:\Qt\5.15.2\mingw81_32\bin\Qt5Gui.dll"; DestDir: "{app}"

;ssl
;Source: "C:\Qt\5.15.2\mingw81_32\bin\libeay32.dll"; DestDir: "{app}"
;Source: "C:\Qt\5.15.2\mingw81_32\bin\ssleay32.dll"; DestDir: "{app}"

Source: "C:\Qt\5.15.2\mingw81_32\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"
Source: "C:\Qt\5.15.2\mingw81_32\plugins\mediaservice\dsengine.dll"; DestDir: "{app}\mediaservice"
Source: "C:\Qt\5.15.2\mingw81_32\plugins\mediaservice\qtmedia_audioengine.dll"; DestDir: "{app}\mediaservice"
Source: "C:\Qt\5.15.2\mingw81_32\plugins\audio\qtaudio_windows.dll"; DestDir: "{app}\audio"
Source: "C:\Qt\5.15.2\mingw81_32\plugins\bearer\qgenericbearer.dll"; DestDir: "{app}\bearer"
Source: "C:\Qt\5.15.2\mingw81_32\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\imageformats"
Source: "C:\Qt\5.15.2\mingw81_32\plugins\imageformats\qsvg.dll"; DestDir: "{app}\imageformats"


[Icons]
Name: "{group}\Server"; Filename: "{app}\snServer.exe"
