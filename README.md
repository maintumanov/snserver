# README #

* Программа web сервер для сети SignalNet
* Версия 0.9.53.0
* Dojo версия 1.14.0

### Настройка и компиляция ###

# для платформ x86 #
* клонировать репозиторий
* установить framework qt 5.3 и выше
* установить компилятор, например mingw
* или установить qt creator с framework в виде пакета
* скомпилировать проект и запустить

# Raspberry (arm) (ОС raspbian) - настройка

Для запуска настройки rasbian необходимо выполнить команду `sudo raspi-config`
после чего настроить SPI интерфейс: *Advanced Options* -> *A6 SPI* -> *Yes*

# Raspberry (arm) (ОС raspbian) - QT 4.8 и компиляция пректа

* Установить qt-sdk из репозитория
`$ sudo aptitude install qt-sdk`
* Клонировать QtSerialPort (Qt4) `git clone git://code.qt.io/qt/qtserialport.git`
* Перейти в директорию с исходным кодом QtSerialPort `cd qtserialport/src/serialport`
* Собрать и установить компонент
   `sudo qmake serialport.pro`
   `sudo make`
   `make install`
* После установки компонента QtSerialPort перейти в директорию с проектом snServer и собрать проект
   `qmake -project`
   `qmake Project.pro`
   `make`
* запустить `./webServer` или в фоне `./webServer &`

# Raspberry (arm) (ОС raspbian) - Установка QT5

* установить пакеты: `sudo apt-get install git apt-transport-https`
* Добавить в `/etc/apt/sources.list`:
~~~~
# Raspbian Buster
deb https://twolife.be/raspbian/ buster main
deb-src https://twolife.be/raspbian/ buster main
~~~~

~~~~
# Debian (stable)
deb http://twolife.be/debian/ stable main
deb-src http://twolife.be/debian/ stable main
~~~~

* Добавить ключ `sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-key E07B3CCBE73CD8EA6B65DA36CF13C21D2578B775`
Если не устанавливается ключь то нужно установить 'sudo apt install dirmngr'
* Обновить индекс пакетов: `sudo apt-get update`
* установить среду сборки: `sudo apt install build-essential`
* Установить библиотеки Qt5: `sudo apt-get install qt5-default qt5-qmake libegl1-mesa libgles2-mesa libqt5serialport5-dev` или `sudo apt-get install qtbase5-dev`
* Установить библиотеки Qt5 bluetooth `sudo apt-get install qtconnectivity5-dev libqt5bluetooth5`
* Установить библиотеки Qt5 multimedia `sudo apt-get install qtmultimedia5-dev libqt5multimedia5-plugins libqt5multimedia5`
* Установить pulseaudio `sudo apt-get install pulseaudio`
* Дополнительные пакеты `sudo apt-get install libgles2-mesa-dev libegl1-mesa-dev libwayland-dev libpulse-dev`
## Компиляция

* Выполнить `git clone https://maintumanov@bitbucket.org/signalnet/snserver.git`
* перейти в папку snserver `cd snserver`
* Разкоментировать в файле проекте `nano snServer.pro` строку *DEFINES += RASPBERRY*
* Собрать проект:  `qmake snServer.pro`   `make`
* запустить `./snerver` или в фоне `./snerver &`


# Raspberry (arm) (ОС raspbian) - Настройка и проверка UART
Для того, чтобы использовать выводы UART, сначала нужно отключить их применение по умолчанию, для этого нужно:

* Отредактировать файлы: */boot/cmdline.txt* и */etc/inittab*, сначала выполнить резервное копирование этих файлов `cp /boot/cmdline.txt /boot/cmdline.bak` и `cp /etc/inittab /etc/inittab.bak`
* Удалить строки **console=ttyAMA0,115200** и **kgdboc=ttyAMA0,115200** в файле */boot/cmdline.txt* с помощью редактора nano: `sudo nano /boot/cmdline.txt`
* Закомментировать последнюю строку в файле */etc/inittab*, для чего нужно поставить символ # перед **T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100** `sudo nano /etc/inittab`
* Перезагрузить систему: `sudo reboot`
* UART доступен на выводах RXD (GPIO15) и TXD (GPIO14)
* При настройке нужно указать порт **/dev/ttyAMA0**

У raspberry pi 3 UART программный, что бы он стабильно работал, необходимо включить UART с минимальной частотой ядра

* отредактировать в файле *sudo nano /boot/config.txt* параметр **enable_uart=1**
* название програмного UART интерфейса **/dev/ttyS0**

# Raspberry (arm) (ОС raspbian) - Настройка автозапуска сервера
Для автоматического запуска сервера, нужно создать скрипт, который будет работать как служба:

* Скопировать исполняемый файл сервер **snserver** в **/usr/bin/**;
* Создаем файл скрипт службы `sudo nano /etc/init.d/snserver`, куда нужно скопировать код скрипта
~~~~
# В этой строке укажите пользователя, под которым запускается snserver . Пользователь по умолчанию - pi
export USER='root'
eval cd ~$USER
# Проверяем параметр команды - start или stop
case "$1" in
  start)
    # Если параметр - start, то запускаем сервер
    su $USER -c '/usr/bin/snserver &'
    echo "Starting snserver for $USER "
    ;;
  stop)
    # Если параметр stop, то останавливаем сервер
    pkill snserver
    echo "snServer stopped"
    ;;
  *)
    echo "Usage: /etc/init.d/snserver {start|stop}"
    exit 1
    ;;
esac
exit 0
~~~~
* Теперь, после создания загрузочного файла, необходимо установить правильные атрибуты сделав файл исполняемым: `sudo chmod 755 /etc/init.d/snserver`, эту операцию необходимо повторять всякий раз, когда изменяется файл;

* Добавить файл в список автозагрузки: `sudo update-rc.d snserver defaults`.

# Запуск через cron

* запустить `sudo crontab -e`
* добавить запись `@reboot /usr/bin/snserver` или с отложенным запуском в 10 секунд `@reboot sleep 10 && /usr/bin/snserver`

# Raspberry (arm) (ОС raspbian) - Настройка подключения RTС часов на базе DS1307 или DS3231 
Самыми популярными RTC являются часы на основе чипов DS1307 и DS3231, рекомендуются часы на основе DS3231, так как они одни из самых точных.

* установить `sudo apt-get install i2c-tools`
* Добавить в файл `/etc/modules` записи `i2c-bcm2708 i2c-dev rtc_ds1307`
* перезагрузит
* проверить доступность модуля командой `lsmod`

Что бы зарегистрировать часы в системе, необходимо выполнить `sudo -s`, `echo ds1307 0x68 > /sys/class/i2c-adapter/i2c-1/new_device`, где **0x68** адрес часов на i2c шине.
Для работы с RTC в Linux существует утилита hwclock, которая может принимать следующие команды:

* `hwclock -r` — считать время, сохранённое в RTC
* `hwclock -s` — синхронизировать время системы с временем RTC
* `hwclock -w` — записать текущее системное время в RTC

Таким образом, после того как будет установлено точное время в системе при помощи утилиты date (например date --set=”20140125 09:17:00”), необходимо записать это время в RTC `hwclock -w`;
При каждой загрузке системы необходимо в качестве системного времени, устанавливать время, сохранённое в RTC, для этого нужно прописать в **/etc/rc.local** перед *exit0* следующие команды: 
~~~~
echo ds1307 0x68 > /sys/class/i2c-adapter/i2c-1/new_device
hwclock -s
~~~~

# Raspberry настройка отправки логов
Для начала можно настроить отправку всех логов на сервер. Создаем конфигурационный файл для rsyslog: nano /etc/rsyslog.d/all.conf
~~~~
*.* @@192.168.0.15:514
~~~~
* где 192.168.0.15 —  IP-адрес сервера логов. *.* — перенаправлять любой лог. Перезапускаем rsyslog:
~~~~
systemctl restart rsyslog
~~~~


