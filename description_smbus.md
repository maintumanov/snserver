# Описание команд snBUS
`QSNContainer::service`
- `Command == 0` команда адаптерам на подключение/отключение
 - `Signal == 0` подключение
 - `Signal == 1` отключение
- `Command == 1` передача состояний адаптеров
 - `Signal == 0` подключено
 - `Signal == 1` отключено
- `Command == 2` ежесекундное событие обновления времени
- `Command == 3` передача сообщения, сообщение содержится в `info`
- `Command == 4` инициализация модулей с задержкой
- `Command == 5` событие изменения статуса одного из модулей
 - `Signal == 1` событие модуля дневное время
 - `Signal == 2` событие модуля ночное время
 - `Signal == 3` событие модуля отсутствие
 - `Signal == 8` событие модуля оповещения
 - `Signal == 9` событие модуля сигнализации
- `Command == 6` изменение адреса устройства (snInterface)
 - `Sender` содержит адрес устройства
- `Command == 7` извещение snInterface об изменении адреса устройства
 - `Sender` содержит адрес устройства
- `Command == 8` управление модулем оповещения
 - `Signal == 0` Отмена тревоги
- `Command == 9` управление модулем сигнализация
 - `Signal == 0` Отмена тревоги
 - `Signal == 1` Постановка на охрану
 - `Signal == 2` Постановка на охрану (тихо)
 - `Signal == 3` Снятие с охраны
 - `Signal == 4` Тревога
- `Command == 10` событие об изменении аккаунтов
- `Command == 11` событие MQTT
 - `Signal == 0` Событие изменения Топика
 - `Signal == 1` запрос состояния виджетов
 
 
``` 
 #define BUSSERV_CONNECT_CHANGE 0
#define BUSSERV_CONNECT_CHANGE_connect 0
#define BUSSERV_CONNECT_CHANGE_disconnect 1
#define BUSSERV_ADAPTER_STATE 1
#define BUSSERV_ADAPTER_STATE_connected 0
#define BUSSERV_ADAPTER_STATE_disconnected 1
#define BUSSERV_TIME_SEC_EVENT 2
#define BUSSERV_MESSAGE 3
#define BUSSERV_MODULE_INIT 4
#define BUSSERV_MODULE_STATE_CHANGE 5
#define BUSSERV_MODULE_STATE_CHANGE_day 1
#define BUSSERV_MODULE_STATE_CHANGE_night 2
#define BUSSERV_MODULE_STATE_CHANGE_absence 3
#define BUSSERV_MODULE_STATE_CHANGE_alert 8
#define BUSSERV_MODULE_STATE_CHANGE_alarm 9
#define BUSSERV_ADDR_CHANGE_BEGIN 6
#define BUSSERV_ADDR_CHANGED 7
#define BUSSERV_ALERT 8
#define BUSSERV_ALERT_cancel 0
#define BUSSERV_ALARM 9
#define BUSSERV_ALARM_cancel 0
#define BUSSERV_ALARM_arming 1
#define BUSSERV_ALARM_arming_quietly 2
#define BUSSERV_ALARM_disarming 3
#define BUSSERV_ALARM_alarm 4
#define BUSSERV_ACCAUNT_CHANGED 10
#define BUSSERV_MGTT 11
#define BUSSERV_MGTT_event 0
#define BUSSERV_MGTT_request_state 1
 ```
 
 