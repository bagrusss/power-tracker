<!--
   Двуязычный README: Русский / English
   Переключение — через HTML <details>
-->
<details open>
<summary><b>&#x1F1F7;&#x1F1FA; Русский</b></summary>

<br>

# Power Tracker — трекер энергопотребления на INA219/INA226/INA231 и ESP8266/ESP32

## Возможности
- Подключение до 16 датчиков INA219, INA226, INA231 по I2C (адреса 0x40–0x4F)
- Измерение тока (mA), напряжения (V), мощности (mW), накопление mAh и mWh
- Параллельные независимые измерения на нескольких датчиках
- OLED-дисплей SSD1306 128×64
- Веб-интерфейс (настройка, управление измерениями, файловый менеджер)
- API: start/stop/status/all
- OTA-обновление по воздуху
- Калибровка тока и напряжения по эталонным приборам

## Физика измерений

### Закон Ома

Основой всех измерений тока и напряжения является закон Ома:

$$
I = \frac{U}{R}
$$

где:
- $I$ — сила тока (А),
- $U$ — напряжение (В),
- $R$ — сопротивление (Ом).

В датчиках семейства INA ток измеряется косвенно: на токовом шунте известного сопротивления $R_{shunt}$ измеряется падение напряжения $U_{shunt}$, и ток вычисляется по закону Ома:

$$
I = \frac{U_{shunt}}{R_{shunt}}
$$

### Математическая модель измерительной цепи

Эквивалентная схема подключения датчика INA для измерения энергопотребления нагрузки:

```
    ┌──────────────────────────────────────────┐
    │              Источник питания             │
    │           (аккумулятор / БП)              │
    └────────────────────┬─────────────────────┘
                         │
                         │ I (ток цепи)
                         │
                    ┌────┴────┐
                    │ R_shunt │  ← токоизмерительный шунт (0.005 Ом)
                    │  (Ush)  │    на нём измеряется падение напряжения
                    └────┬────┘
                         │
                         │
                    ┌────┴────┐
                    │  IN+   │
                    │ ────── │  ← датчик INA (измеряет Ushunt и Uнагр)
                    │  IN-   │
                    └────┬────┘
                         │
                         │
                    ┌────┴────┐
                    │ R_нагр  │  ← нагрузка (телефон, планшет, ...)
                    │  (Uнагр)│
                    └────┬────┘
                         │
                         │
                    ┌────┴────┐
                    │   GND   │  ← общая земля
                    └─────────┘
```

**Математическая модель** описывается следующими соотношениями:

1. **Ток в цепи** — един для всей последовательной цепи (шунт + нагрузка):

   $$
   I = \frac{U_{shunt}}{R_{shunt}} = \frac{U_{нагр}}{R_{нагр}}
   $$

2. **Напряжение источника питания** равно сумме падений напряжений на шунте и нагрузке:

   $$
   U_{ист} = U_{shunt} + U_{нагр}
   $$

   Поскольку $R_{shunt} \ll R_{нагр}$ (шунт имеет сопротивление единицы-десятки мОм, а нагрузка — единицы-сотни Ом), падением напряжения на шунте можно пренебречь: $U_{shunt} \approx 0$, следовательно $U_{ист} \approx U_{нагр}$. Именно напряжение $U_{нагр}$ измеряет датчик INA между выводами GND и IN-.

3. **Мгновенная мощность**, потребляемая нагрузкой:

   $$
   P = U_{нагр} \times I
   $$

4. **Потреблённый заряд** (интеграл тока по времени):

   $$
   Q = \int_{t_0}^{t_1} I(t) \, dt \quad \text{[Кл]}
   $$

   $$
   Q_{mAh} = \frac{1000}{3600} \int_{t_0}^{t_1} I(t) \, dt \quad \text{[mAh]}
   $$

5. **Потреблённая энергия** (интеграл мощности по времени):

   $$
   E = \int_{t_0}^{t_1} U_{нагр}(t) \cdot I(t) \, dt \quad \text{[Дж]}
   $$

   $$
   E_{mWh} = \frac{1000}{3600} \int_{t_0}^{t_1} U_{нагр}(t) \cdot I(t) \, dt \quad \text{[mWh]}
   $$

Именно эти интегральные величины накапливает микроконтроллер в режиме Tracking, опрашивая датчик INA с заданным интервалом $\Delta t$ и выполняя дискретное суммирование.

### Электрический заряд и mAh

Электрический заряд $Q$ измеряется в кулонах (Кл). Один кулон — это заряд, переносимый током 1 А за 1 с:

$$
Q = I \times t
$$

где $t$ — время в секундах.

На практике для измерения ёмкости батарей удобнее использовать производную единицу **ампер-час (Ah)** и её дольную единицу **миллиампер-час (mAh)**:

$$
1 \text{ Ah} = 3600 \text{ Кл}
$$

$$
1 \text{ mAh} = 3.6 \text{ Кл}
$$

Поскольку ток в цепи непостоянен и меняется во времени, полный потреблённый заряд вычисляется как **интеграл тока по времени**:

$$
Q = \int_{t_0}^{t_1} I(t) \, dt
$$

В дискретном виде (для микроконтроллера с фиксированным интервалом опроса $\Delta t$):

$$
Q \approx \sum_{i=1}^{n} I_i \cdot \Delta t_i
$$

где $I_i$ — мгновенное значение тока на $i$-м измерении, $\Delta t_i$ — интервал времени между измерениями.

### Энергия и mWh

Энергия $E$ измеряется в джоулях (Дж). Мощность $P$ — это скорость потребления энергии:

$$
P = U \times I
$$

Потреблённая энергия — это интеграл мощности по времени:

$$
E = \int_{t_0}^{t_1} P(t) \, dt = \int_{t_0}^{t_1} U(t) \cdot I(t) \, dt
$$

В дискретном виде:

$$
E \approx \sum_{i=1}^{n} U_i \cdot I_i \cdot \Delta t_i
$$

На практике используется производная единица **ватт-час (Wh)** и её дольная единица **милливатт-час (mWh)**:

$$
1 \text{ Wh} = 3600 \text{ Дж}
$$

$$
1 \text{ mWh} = 3.6 \text{ Дж}
$$

### Связь mAh и mWh

Эти две величины связаны через напряжение:

$$
E = Q \times U
$$

или в производных единицах:

$$
\text{mWh} = \text{mAh} \times U
$$

где $U$ — среднее напряжение за время измерения.

Таким образом:
- **mAh** характеризует количество перенесённого заряда (сколько электронов перенесено).
- **mWh** характеризует количество потреблённой энергии (с учётом напряжения).

Для батарей с разным номинальным напряжением (например, 3.7 В Li-ion vs 1.2 В NiMH) сравнение по mAh некорректно — необходимо сравнивать по mWh.

### Аналогия с бытовым электросчётчиком

Принцип работы данного трекера полностью аналогичен бытовому электросчётчику, который установлен в квартире или доме. Такой счётчик измеряет потреблённую электроэнергию в *киловатт-часах (кВт·ч)*:

$$
1 \text{ кВт}\cdot\text{ч} = 1000 \text{ Вт}\cdot\text{ч} = 3\,600\,000 \text{ Дж}
$$

Электросчётчик работает по тому же принципу интеграла мощности по времени: он непрерывно измеряет мгновенное напряжение $U(t)$ и ток $I(t)$ в цепи, перемножает их (получая мгновенную мощность $P(t) = U(t) \cdot I(t)$) и суммирует эти значения за каждый период времени. По сути, счётчик выполняет ту же дискретную аппроксимацию интеграла:

$$
E \approx \sum_{i=1}^{n} U_i \cdot I_i \cdot \Delta t_i
$$

Разница лишь в масштабе:
- **Бытовой счётчик** оперирует киловаттами и киловатт-часами, измеряя энергопотребление за дни и месяцы.
- **Данный трекер** оперирует милливаттами и милливатт-часами, измеряя энергопотребление одного устройства (телефона, планшета) за время тестового прогона (минуты-часы).

Таким образом, проект представляет собой **прецизионный миниатюрный электросчётчик** для измерений энергопотребления мобильных и не только устройств.

### Напряжение источника питания

Номинальное напряжение Li-ion аккумулятора составляет 3,6–3,7 В. Это среднее значение напряжения на интервале от 4,2 В (полностью заряжен) до 3,0 В (полностью разряжен). Полная зарядка аккумулятора (100 %) означает, что напряжение на его клеммах достигло 4,2 В. При разряженном аккумуляторе (0 %) считается, что напряжение на батарее упало до 3,0 В (возможны значения до 2,5 В в зависимости от технологии производства, поколения и т.д.). Существуют высоковольтные Li-ion аккумуляторы с напряжением до **4,45 В**. Кривая разряда (зависимость напряжения от глубины разряда) нелинейна и может выглядеть следующим образом:

<img src="docs/0/li-ion.jpg" width="500">

Не существует единой стандартизированной методики оценки процента потреблённого заряда батареи, поскольку под нагрузкой происходит просадка напряжения, а при меняющейся нагрузке напряжение будет просаживаться и восстанавливаться; положение напряжения в интервале от 4,2 до 3,0 В не даёт точной оценки. На практике между литиевой батареей и прибором устанавливается плата BMS (battery management system), которая выполняет следующие функции:
- **Защита от перезаряда** — отключение зарядного источника при достижении верхнего порогового напряжения (предотвращение пожара и взрыва).
- **Защита от переразряда** — при выходе за пределы нижнего порогового напряжения в литиевых батареях начинаются необратимые процессы, приводящие к потере ёмкости и росту внутреннего сопротивления, что снижает способность отдавать большие токи и повышает скорость саморазряда.
- **Защита от короткого замыкания** — отключение нагрузки при резком падении напряжения.

Функции подсчёта заряда (Coulomb counting), учёта циклов и определения степени износа (State of Health) реализуются отдельными микросхемами **fuel gauge** (например, BQ40Z50, MAX17048) или продвинутыми BMS с интегрированным fuel gauge. В простых платах защиты (DW01 + FS8205) эти возможности отсутствуют.

Существует два основных подхода к оценке процента заряда (State of Charge, SoC):
1. **По напряжению (voltage-based)** — простой метод, но неточный при переменной нагрузке из-за просадок и восстановлений напряжения.
2. **Кулонометрический (Coulomb counting)** — интегрирование тока по времени. В этом случае в fuel gauge хранится эталонная ёмкость полностью заряженной батареи в mAh/Ah и mWh/Wh, а текущий % заряда определяется как отношение оставшейся ёмкости к эталонной.

Для качественной оценки энергопотребления различных сценариев необходимо измерять ёмкость в mAh/Ah и mWh/Wh, которые отдал источник питания во время прогона. Желательно обеспечить одинаковые начальные условия теста, такие как температура измеряемого прибора (телефоны/планшеты при нагреве могут снижать частоты CPU и GPU для уменьшения тепловыделения). Также при разряде аккумулятора могут включаться дополнительные ограничения на максимальные частоты CPU и GPU или режим энергосбережения. Из-за нелинейности кривой разряда для качественной оценки энергопотребления используется подсчёт потреблённого тока и мощности в mAh/Ah и mWh/Wh.

### mAh/Ah

Ток разряда (А) × Время разряда (ч) до конечного напряжения на интервале значений. Измеряется множество мгновенных значений тока, помноженных на дельту между двумя соседними измерениями. Результат — итоговая сумма. Например, 1000 mAh означает, что источник питания способен отдавать ток 1 А в течение 1 часа или ток 0,5 А в течение 2 часов и т.д.
Промежуточные «дельты» (мгновенные значения) не используются, так как в процессе измерения меняется нагрузка на источник питания и важна суммарная, итоговая потребленная энергия от источника питания за весь период прогона сценария.

### mWh/Wh

Среднее напряжение (В) × Суммарный заряд (Ач) — сумма мгновенных значений (Напряжение × Ток) за всё время разряда. Включает в себя напряжение, даёт прямое сравнение энергии между батареями с разными диапазонами напряжений разряда.

### Измерение тока и напряжения на датчиках серии INAxxx

На каждом датчике имеется 3 выходных провода: *земля* (подключается к '-' источника питания), **IN+** (подключается к '+' источника питания), **IN-** (подключается к '+' прибора). Токовый шунт (резистор), на котором происходит измерение напряжения, вставляется в разрыв цепи питания. Сопротивление шунта постоянно, и для его изготовления используется сплав, обеспечивающий стабильное значение сопротивления даже при изменении температуры шунта. По закону Ома ($I = U/R$) вычисляется ток цепи.
Между землей и **IN-** измеряется напряжение на приборе/источнике питания.

### Сравнение датчиков INA

| Характеристика | INA219 | INA226 | INA228 | INA231 |
| :--- | :--- | :--- | :--- | :--- |
| **Основное назначение** | Монитор тока/мощности | Монитор тока/мощности | Монитор тока/мощности/энергии | Монитор тока/мощности |
| **Разрешение АЦП** | 12-бит | 16-бит | **20-бит** | 16-бит |
| **Диапазон напряжений (Common Mode)** | 0 … +26 В | 0 … +36 В | **-0.3 … +85 В** | 0 … +28 В |
| **Диапазон измерения шунта (±Vsense)** | ±40 / ±80 / ±160 / ±320 мВ (программно) | ±81.92 мВ | ±163.84 мВ / ±40.96 мВ | ±81.92 мВ |
| **Интерфейс связи** | I2C / SMBus | I2C / SMBus | **I2C / SMBus (High Speed)** | I2C / SMBus (1.8V совместимый) |
| **Макс. ток потребления** | 1 мА | 330 мкА | 640 мкА | 330 мкА (тип.) |
| **Ток в режиме сна** | 100 мкА | 2.5 мкА | 5 мкА | 100 мкА |
| **Входной ток смещения** | 10 мкА | 10 мкА | **2.5 нА (макс.)** | 10 мкА |
| **Напряжение питания (Vdd)** | 3.0 … 5.5 В | 2.7 … 5.5 В | 2.7 … 5.5 В | 2.7 … 5.5 В |
| **Встроенный датчик температуры** | Нет | Нет | **Да (±1°C точность)** | Нет |
| **Аккумуляция энергии/заряда** | Нет | Нет | **Да** | Нет |
| **Компенсация температуры шунта** | Нет | Нет | **Да** | Нет |
| **Кол-во адресов (I2C)** | 16 | 16 | 16 | 16 |
| **Alert (сигнальный пин)** | Нет | Да | **Да (быстрый отклик 75 мкс)** | Да |
| **Основные ошибки (макс)** | ±0.5% (по мощности) | ±0.5% (по мощности) | ±0.5% (по мощности), **±0.05%** (по усилению) | Gain: 0.5%, Offset: 50 мкВ |
| **Ошибка смещения (макс.)** | — | — | **±1 мкВ** | ±50 мкВ |
| **Температурный дрейф нуля** | 0.1 мкВ/°C | 0.02 мкВ/°C | **0.01 мкВ/°C** | 0.1 мкВ/°C |
| **Диапазон рабочих температур** | -40 … +125 °C | -40 … +125 °C | -40 … +125 °C | -40 … +125 °C |

## Быстрый старт

1. Подключить датчики INA и дисплей SSD1306 к I2C (SDA→D2, SCL→D1 для ESP8266)
2. Подать питание 4.2–7 В (USB или V-IN)
3. МК создаст точку доступа `PT_XXXXXX`, пароль `power_tracker`
4. Подключиться, открыть `192.168.4.1`, ввести SSID/пароль WiFi
5. В настройках указать тип INA, сопротивление шунта, макс. ток для каждого датчика
6. На главной нажать **Track** — измерение запущено

## API

| Метод | URL | Описание |
|-------|-----|----------|
| start | `/start?serial=...` | Запуск измерения |
| stop | `/stop?serial=...` | Остановка, результат в ответе (без сохранения в файл) |
| status | `/status?serial=...` | Текущие значения измеряемых величин |
| all | `/all` | Статусы всех датчиков |

Пример:
```bash
curl "192.168.1.100/start?serial=SW0123456"
sleep 3600
curl "192.168.1.100/stop?serial=SW0123456" > result.json
```

## Настройки (веб-интерфейс)

### Wi-Fi settings
Смена SSID/пароля. После применения — перезагрузка.

### Board config
Частота шины I2C: 100 или 400 кГц.

### Updates
- URL к `ota.json`, интервал проверки, автообновление
- Если есть активное измерение — OTA не запустится

Формат `ota.json`:
```json
{
  "version": "1.0.7",
  "buildNumber": 10,
  "esp8266": "https://example.com/firmware.bin"
}
```

### Device info (на каждый датчик)
Название, ОС, серийный номер, полная ёмкость батареи.

### Circuit params (на каждый датчик)
- Тип INA: INA219/INA226/INA231
- Сопротивление шунта (Ом), макс. ток (А)
- Коэффициенты коррекции тока и напряжения
- Интервал опроса (по умолчанию 500 мс)
- Power Strategy: Battery / PowerSource (заглушка)

## LED-индикация

| Режим | Значение |
|-------|----------|
| Постоянно горит | Настройка, режим AP |
| Медленное мигание (500/1500 мс) | Idle, нет активных измерений |
| Быстрое мигание (300/300 мс) | Idle, идёт измерение |
| Выключен | OTA |

## Архитектура

Проект построен на конечных автоматах:
- [`McStateMachine`](src/states/mc/McStateMachine.cpp) — 6 состояний МК (ConnectingWifi, Idle, SetupWifi, Settings, Warning, OtaUpdating)
- [`InaStateMachine`](src/states/ina/InaStateMachine.cpp) — 2 состояния на каждый датчик (Idle, Tracking)

Подробнее: [docs/5/5.md](docs/5/5.md)

### Структура проекта

```
src/
├── api/          # Интерфейсы (Board, WifiAdapter, PowerMonitor, ...)
├── db/           # GyverDB, ключи конфигурации
├── devices/      # Аппаратные реализации
│   └── circuit/  # INA219, INA226, INA231, детектор
├── hack/         # WebUi (Gyver Settings)
├── net/          # HTTP, WiFi, обработка команд
├── ota/          # OTA (проверка, загрузка, установка)
├── res/          # Строковые ресурсы
├── states/
│   ├── mc/       # Состояния МК
│   └── ina/      # Состояния датчиков
└── util/         # BuildInfo, Prefs, утилиты
```

## Файловая система (LittleFS)

| Файл | Назначение |
|------|-----------|
| `/data.db` | Настройки (GyverDB) |

## Сборка

```bash
# VS Code + PlatformIO
pio run -e nodemcu    # ESP8266
pio run -e esp32_env   # ESP32 (закомментирован в platformio.ini)
```

Скрипты:
- [`generate_buildinfo.py`](scripts/generate_buildinfo.py) — генерация BuildInfo.h/.cpp
- [`release.py`](scripts/release.py) — сборка + GitHub Release

## Известные ограничения

- WarningState — заглушка (не реализовано)
- INA231 может определяться как INA226 (нужно указать тип вручную)
- PowerSource — заглушка (пустые методы)
- Несколько клиентов веб-интерфейса — диалог подтверждения может уйти не тому клиенту

## Лицензия

[MIT](LICENSE)

</details>

<details>
<summary><b>&#x1F1EC;&#x1F1E7; English</b></summary>

<br>

# Power Tracker — power consumption tracker for INA219/INA226/INA231 and ESP8266/ESP32

## Features
- Connect up to 16 sensors: INA219, INA226, INA231 via I2C (addresses 0x40–0x4F)
- Measure current (mA), voltage (V), power (mW); accumulate mAh and mWh
- Parallel independent measurements on multiple sensors
- OLED display SSD1306 128×64
- Web interface (setup, measurement control, file manager)
- API: start/stop/status/all
- OTA (Over-the-Air) updates
- Current and voltage calibration against reference instruments

## Physics of Measurements

### Ohm's Law

All current and voltage measurements are based on Ohm's law:

$$
I = \frac{U}{R}
$$

where:
- $I$ — current (A),
- $U$ — voltage (V),
- $R$ — resistance (Ω).

In the INA family of sensors, current is measured indirectly: the voltage drop $U_{shunt}$ is measured across a current shunt of known resistance $R_{shunt}$, and the current is calculated using Ohm's law:

$$
I = \frac{U_{shunt}}{R_{shunt}}
$$

### Mathematical Model of the Measurement Circuit

Equivalent circuit diagram for connecting an INA sensor to measure the power consumption of a load:

```
    ┌──────────────────────────────────────────┐
    │              Power Source                 │
    │          (battery / PSU)                  │
    └────────────────────┬─────────────────────┘
                         │
                         │ I (circuit current)
                         │
                    ┌────┴────┐
                    │ R_shunt │  ← current-sense shunt (0.005 Ω)
                    │  (Ush)  │    voltage drop measured across it
                    └────┬────┘
                         │
                         │
                    ┌────┴────┐
                    │  IN+   │
                    │ ────── │  ← INA sensor (measures Ushunt and Uload)
                    │  IN-   │
                    └────┬────┘
                         │
                         │
                    ┌────┴────┐
                    │ R_load  │  ← load (phone, tablet, ...)
                    │ (Uload) │
                    └────┬────┘
                         │
                         │
                    ┌────┴────┐
                    │   GND   │  ← common ground
                    └─────────┘
```

**The mathematical model** is described by the following relations:

1. **Circuit current** — the same throughout the entire series circuit (shunt + load):

   $$
   I = \frac{U_{shunt}}{R_{shunt}} = \frac{U_{load}}{R_{load}}
   $$

2. **Source voltage** equals the sum of voltage drops across the shunt and the load:

   $$
   U_{src} = U_{shunt} + U_{load}
   $$

   Since $R_{shunt} \ll R_{load}$ (the shunt has a resistance of units to tens of mΩ, while the load has units to hundreds of Ω), the voltage drop across the shunt can be neglected: $U_{shunt} \approx 0$, therefore $U_{src} \approx U_{load}$. The INA sensor measures exactly this voltage $U_{load}$ between the GND and IN- pins.

3. **Instantaneous power** consumed by the load:

   $$
   P = U_{load} \times I
   $$

4. **Consumed charge** (integral of current over time):

   $$
   Q = \int_{t_0}^{t_1} I(t) \, dt \quad \text{[C]}
   $$

   $$
   Q_{mAh} = \frac{1000}{3600} \int_{t_0}^{t_1} I(t) \, dt \quad \text{[mAh]}
   $$

5. **Consumed energy** (integral of power over time):

   $$
   E = \int_{t_0}^{t_1} U_{load}(t) \cdot I(t) \, dt \quad \text{[J]}
   $$

   $$
   E_{mWh} = \frac{1000}{3600} \int_{t_0}^{t_1} U_{load}(t) \cdot I(t) \, dt \quad \text{[mWh]}
   $$

These are the integral quantities accumulated by the microcontroller in Tracking mode, polling the INA sensor at a given interval $\Delta t$ and performing discrete summation.

### Electric Charge and mAh

Electric charge $Q$ is measured in coulombs (C). One coulomb is the charge transported by a current of 1 A in 1 s:

$$
Q = I \times t
$$

where $t$ is the time in seconds.

In practice, for measuring battery capacity, the derived unit **ampere-hour (Ah)** and its submultiple **milliampere-hour (mAh)** are more convenient:

$$
1 \text{ Ah} = 3600 \text{ C}
$$

$$
1 \text{ mAh} = 3.6 \text{ C}
$$

Since the current in the circuit is not constant and varies over time, the total consumed charge is computed as the **integral of current over time**:

$$
Q = \int_{t_0}^{t_1} I(t) \, dt
$$

In discrete form (for a microcontroller with a fixed sampling interval $\Delta t$):

$$
Q \approx \sum_{i=1}^{n} I_i \cdot \Delta t_i
$$

where $I_i$ is the instantaneous current at the $i$-th measurement, and $\Delta t_i$ is the time interval between measurements.

### Energy and mWh

Energy $E$ is measured in joules (J). Power $P$ is the rate of energy consumption:

$$
P = U \times I
$$

Consumed energy is the integral of power over time:

$$
E = \int_{t_0}^{t_1} P(t) \, dt = \int_{t_0}^{t_1} U(t) \cdot I(t) \, dt
$$

In discrete form:

$$
E \approx \sum_{i=1}^{n} U_i \cdot I_i \cdot \Delta t_i
$$

In practice, the derived unit **watt-hour (Wh)** and its submultiple **milliwatt-hour (mWh)** are used:

$$
1 \text{ Wh} = 3600 \text{ J}
$$

$$
1 \text{ mWh} = 3.6 \text{ J}
$$

### Relationship between mAh and mWh

These two quantities are related through voltage:

$$
E = Q \times U
$$

or in derived units:

$$
\text{mWh} = \text{mAh} \times U
$$

where $U$ is the average voltage over the measurement period.

Thus:
- **mAh** characterizes the amount of charge transferred (how many electrons were moved).
- **mWh** characterizes the amount of energy consumed (accounting for voltage).

For batteries with different nominal voltages (e.g., 3.7 V Li-ion vs 1.2 V NiMH), comparison by mAh is incorrect — you must compare by mWh.

### Analogy with a Household Electricity Meter

The operating principle of this tracker is fully analogous to a household electricity meter installed in an apartment or house. Such a meter measures consumed electrical energy in *kilowatt-hours (kWh)*:

$$
1 \text{ kWh} = 1000 \text{ Wh} = 3\,600\,000 \text{ J}
$$

The electricity meter works on the same principle of the power integral over time: it continuously measures the instantaneous voltage $U(t)$ and current $I(t)$ in the circuit, multiplies them (obtaining instantaneous power $P(t) = U(t) \cdot I(t)$), and sums these values over each time period. In essence, the meter performs the same discrete approximation of the integral:

$$
E \approx \sum_{i=1}^{n} U_i \cdot I_i \cdot \Delta t_i
$$

The only difference is the scale:
- **A household meter** operates in kilowatts and kilowatt-hours, measuring energy consumption over days and months.
- **This tracker** operates in milliwatts and milliwatt-hours, measuring the energy consumption of a single device (phone, tablet) during a test run (minutes to hours).

Thus, the project represents a **precision miniature electricity meter** for measuring the energy consumption of mobile and other devices.

### Power Source Voltage

The nominal voltage of a Li-ion battery is 3.6–3.7 V. This is the average voltage over the range from 4.2 V (fully charged) to 3.0 V (fully discharged). A fully charged battery (100%) means the voltage at its terminals has reached 4.2 V. A discharged battery (0%) is considered to have dropped to 3.0 V (values as low as 2.5 V are possible depending on manufacturing technology, generation, etc.). There are high-voltage Li-ion batteries with voltages up to **4.45 V**. The discharge curve (voltage vs. depth of discharge) is non-linear and may look like this:

<img src="docs/0/li-ion.jpg" width="500">

There is no single standardized method for estimating the percentage of consumed battery charge, because under load the voltage sags, and under varying load the voltage will sag and recover; the voltage position in the 4.2 to 3.0 V range does not provide an accurate estimate. In practice, a BMS (battery management system) board is placed between the lithium battery and the device, performing the following functions:
- **Overcharge protection** — disconnects the charging source when the upper threshold voltage is reached (prevents fire and explosion).
- **Over-discharge protection** — exceeding the lower voltage threshold in lithium batteries initiates irreversible processes leading to capacity loss and increased internal resistance, which reduces the ability to deliver high currents and increases self-discharge rate.
- **Short-circuit protection** — disconnects the load upon a sharp voltage drop.

Charge counting (Coulomb counting), cycle tracking, and State of Health (SoH) determination are implemented by dedicated **fuel gauge** ICs (e.g., BQ40Z50, MAX17048) or advanced BMS with an integrated fuel gauge. Simple protection boards (DW01 + FS8205) lack these capabilities.

There are two main approaches to State of Charge (SoC) estimation:
1. **Voltage-based** — a simple method, but inaccurate under variable load due to voltage sags and recovery.
2. **Coulomb counting** — integrating current over time. In this case, the fuel gauge stores the reference capacity of a fully charged battery in mAh/Ah and mWh/Wh, and the current % charge is determined as the ratio of remaining capacity to the reference capacity.

For a quality assessment of energy consumption across different scenarios, it is necessary to measure the capacity in mAh/Ah and mWh/Wh delivered by the power source during the run. It is advisable to ensure identical initial test conditions, such as the temperature of the device under test (phones/tablets may reduce CPU and GPU frequencies under heating to decrease heat dissipation). Additionally, as the battery discharges, additional limits on maximum CPU and GPU frequencies or power-saving modes may be activated. Due to the non-linearity of the discharge curve, counting consumed current and power in mAh/Ah and mWh/Wh is used for a quality assessment of energy consumption.

### mAh/Ah

Discharge current (A) × Discharge time (h) down to the cutoff voltage over the value interval. Numerous instantaneous current values are measured, multiplied by the delta between two adjacent measurements. The result is the total sum. For example, 1000 mAh means the power source can deliver 1 A for 1 hour, or 0.5 A for 2 hours, etc.
Intermediate "deltas" (instantaneous values) are not used since the load on the power source changes during measurement, and the total, final consumed energy from the power source over the entire scenario run is what matters.

### mWh/Wh

Average voltage (V) × Total charge (Ah) — the sum of instantaneous values (Voltage × Current) over the entire discharge period. Includes voltage, providing a direct energy comparison between batteries with different discharge voltage ranges.

### Current and Voltage Measurement on INAxxx Series Sensors

Each sensor has 3 output wires: *ground* (connected to '-' of the power source), **IN+** (connected to '+' of the power source), **IN-** (connected to '+' of the device). The current shunt (resistor), across which the voltage is measured, is inserted in series with the power circuit. The shunt resistance is constant, and an alloy is used in its manufacture to ensure a stable resistance value even with temperature changes of the shunt. Using Ohm's law ($I = U/R$), the circuit current is calculated.
The voltage across the device/power source is measured between ground and **IN-**.

### INA Sensor Comparison

| Characteristic | INA219 | INA226 | INA228 | INA231 |
| :--- | :--- | :--- | :--- | :--- |
| **Primary purpose** | Current/power monitor | Current/power monitor | Current/power/energy monitor | Current/power monitor |
| **ADC resolution** | 12-bit | 16-bit | **20-bit** | 16-bit |
| **Voltage range (Common Mode)** | 0 … +26 V | 0 … +36 V | **-0.3 … +85 V** | 0 … +28 V |
| **Shunt measurement range (±Vsense)** | ±40 / ±80 / ±160 / ±320 mV (programmable) | ±81.92 mV | ±163.84 mV / ±40.96 mV | ±81.92 mV |
| **Communication interface** | I2C / SMBus | I2C / SMBus | **I2C / SMBus (High Speed)** | I2C / SMBus (1.8V compatible) |
| **Max. current consumption** | 1 mA | 330 µA | 640 µA | 330 µA (typ.) |
| **Sleep mode current** | 100 µA | 2.5 µA | 5 µA | 100 µA |
| **Input bias current** | 10 µA | 10 µA | **2.5 nA (max.)** | 10 µA |
| **Supply voltage (Vdd)** | 3.0 … 5.5 V | 2.7 … 5.5 V | 2.7 … 5.5 V | 2.7 … 5.5 V |
| **Built-in temperature sensor** | No | No | **Yes (±1°C accuracy)** | No |
| **Energy/charge accumulation** | No | No | **Yes** | No |
| **Shunt temperature compensation** | No | No | **Yes** | No |
| **Number of addresses (I2C)** | 16 | 16 | 16 | 16 |
| **Alert (signal pin)** | No | Yes | **Yes (75 µs fast response)** | Yes |
| **Main errors (max)** | ±0.5% (power) | ±0.5% (power) | ±0.5% (power), **±0.05%** (gain) | Gain: 0.5%, Offset: 50 µV |
| **Offset error (max.)** | — | — | **±1 µV** | ±50 µV |
| **Zero-drift vs temperature** | 0.1 µV/°C | 0.02 µV/°C | **0.01 µV/°C** | 0.1 µV/°C |
| **Operating temperature range** | -40 … +125 °C | -40 … +125 °C | -40 … +125 °C | -40 … +125 °C |

## Quick Start

1. Connect INA sensors and SSD1306 display to I2C (SDA→D2, SCL→D1 for ESP8266)
2. Apply 4.2–7 V power (USB or V-IN)
3. The MCU will create an access point `PT_XXXXXX`, password `power_tracker`
4. Connect, open `192.168.4.1`, enter WiFi SSID/password
5. In settings, specify INA type, shunt resistance, max current for each sensor
6. On the main page, click **Track** — measurement starts

## API

| Method | URL | Description |
|-------|-----|----------|
| start | `/start?serial=...` | Start measurement |
| stop | `/stop?serial=...` | Stop, result in response (not saved to file) |
| status | `/status?serial=...` | Current measured values |
| all | `/all` | Status of all sensors |

Example:
```bash
curl "192.168.1.100/start?serial=SW0123456"
sleep 3600
curl "192.168.1.100/stop?serial=SW0123456" > result.json
```

## Settings (Web Interface)

### Wi-Fi settings
Change SSID/password. Reboots after applying.

### Board config
I2C bus frequency: 100 or 400 kHz.

### Updates
- URL to `ota.json`, check interval, auto-update
- OTA will not start if any measurement is active

`ota.json` format:
```json
{
  "version": "1.0.7",
  "buildNumber": 10,
  "esp8266": "https://example.com/firmware.bin"
}
```

### Device info (per sensor)
Device name, OS, serial number, full battery capacity.

### Circuit params (per sensor)
- INA type: INA219/INA226/INA231
- Shunt resistance (Ω), max current (A)
- Current and voltage correction coefficients
- Polling interval (default 500 ms)
- Power Strategy: Battery / PowerSource (stub)

## LED Indication

| Mode | Meaning |
|-------|----------|
| Constantly on | Setup, AP mode |
| Slow blink (500/1500 ms) | Idle, no active measurements |
| Fast blink (300/300 ms) | Idle, measurement in progress |
| Off | OTA |

## Architecture

The project is built on finite state machines:
- [`McStateMachine`](src/states/mc/McStateMachine.cpp) — 6 MCU states (ConnectingWifi, Idle, SetupWifi, Settings, Warning, OtaUpdating)
- [`InaStateMachine`](src/states/ina/InaStateMachine.cpp) — 2 states per sensor (Idle, Tracking)

More details: [docs/5/5.md](docs/5/5.md)

### Project Structure

```
src/
├── api/          # Interfaces (Board, WifiAdapter, PowerMonitor, ...)
├── db/           # GyverDB, configuration keys
├── devices/      # Hardware implementations
│   └── circuit/  # INA219, INA226, INA231, detector
├── hack/         # WebUi (Gyver Settings)
├── net/          # HTTP, WiFi, command handling
├── ota/          # OTA (check, download, install)
├── res/          # String resources
├── states/
│   ├── mc/       # MCU states
│   └── ina/      # Sensor states
└── util/         # BuildInfo, Prefs, utilities
```

## File System (LittleFS)

| File | Purpose |
|------|-----------|
| `/data.db` | Settings (GyverDB) |

## Build

```bash
# VS Code + PlatformIO
pio run -e nodemcu    # ESP8266
pio run -e esp32_env   # ESP32 (commented out in platformio.ini)
```

Scripts:
- [`generate_buildinfo.py`](scripts/generate_buildinfo.py) — generates BuildInfo.h/.cpp
- [`release.py`](scripts/release.py) — build + GitHub Release

## Known Limitations

- WarningState — stub (not implemented)
- INA231 may be detected as INA226 (type must be specified manually)
- PowerSource — stub (empty methods)
- Multiple web interface clients — confirmation dialog may go to the wrong client

## License

[MIT](LICENSE)

</details>
