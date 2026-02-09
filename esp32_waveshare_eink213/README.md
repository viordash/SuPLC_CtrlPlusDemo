# ESP32 Self-sufficient PLC (SuPLC)

Управляющий модуль программируемого логического контроллера (ПЛК) на базе ESP32 с e-ink дисплеем Waveshare 2.13".

## Описание проекта

Это автономный ПЛК, программируемый непосредственно с помощью кнопок, подключённых к микроконтроллеру ESP32. Для программирования используется язык **Ladder Diagram (Ladder Logic)** — графический язык программирования ПЛК, представляющий логику в виде электрических цепей.

### Основные возможности

- **Программирование без ПК** — создание и редактирование программы с помощью физических кнопок
- **Визуализация программы** — отображение Ladder-диаграммы на e-ink дисплее и веб-странице
- **WiFi-подключение** — работа в режиме станции (STA) или точки доступа (AP)
- **Веб-интерфейс** — просмотр и управление программой через HTTP API
- **OTA-обновления** — обновление прошивки по воздуху
- **Hot-reload** — горячая перезагрузка с сохранением состояния

### Ресурсы ввода-вывода

| Тип | Количество | Обозначение |
|-----|------------|-------------|
| Дискретные входы | 4 | D0–D3 |
| Дискретные выходы | 4 | O0–O3 |
| Аналоговые входы | 3 | A0–A2 |
| Переменные | 4 | V1–V4 |

## Сборка

### Требования

- ESP-IDF 5.4.1+
- GNU Make
- Toolchain для ESP32

### Сборка через командную строку

```bash
cd esp32_waveshare_eink213
make -j $(nproc) all
```

### Прошивка устройства

```bash
make -j $(nproc) flash
```

### Сборка в VSCode

Откройте папку `esp32_waveshare_eink213` в VSCode и используйте задачи из `.vscode/tasks.json`:

| Задача | Описание |
|--------|----------|
| `build` | Параллельная сборка проекта |
| `build & install` | Сборка и прошивка через USB (921600 бод) |
| `build & ota` | Сборка и OTA-обновление |
| `rebuild` | Полная пересборка (clean + build) |

Запуск задач: `Ctrl+Shift+B` или `Terminal → Run Build Task...`

### Конфигурация

```bash
make menuconfig
```

## Архитектура BSP (Board Support Package)

Папка `main/BSP` содержит платформо-зависимый код, обеспечивающий абстракцию аппаратного обеспечения. Проект поддерживает несколько платформ:

- **ESP32** (основная платформа) — FreeRTOS
- **ESP8266** — FreeRTOS
- **Linux/POSIX** — pthread

### Структура BSP

```
BSP/
├── display/EPD_2in13_V4/    # Драйвер e-ink дисплея Waveshare 2.13"
├── gpio/esp32_waveshare_eink213/  # Конфигурация GPIO
├── wifi/esp32/              # WiFi (STA, AP, сканирование)
├── network/esp32/           # HTTP-сервер, SmartConfig, SNTP
├── storage/esp32/           # Flash, NVS, RAM
├── system/esp32/            # Атрибуты устройства, OTA
└── os/esp32/                # Обёртки над FreeRTOS
```

### Обёртки над FreeRTOS (os/esp32)

Папка `main/BSP/os/esp32` содержит унифицированный API для работы с операционной системой:

#### Примитивы синхронизации (semphr.h)

```cpp
// Создание и удаление мьютекса
mutex_handle_t create_mutex();
void delete_mutex(mutex_handle_t mutex);

// Захват и освобождение мьютекса
void lock_mutex(mutex_handle_t mutex);
void unlock_mutex(mutex_handle_t mutex);

// ISR-безопасные варианты для обработчиков прерываний
void lock_mutex_from_ISR(mutex_handle_t mutex, int32_t *pxHigherPriorityTaskWoken);
void unlock_mutex_from_ISR(mutex_handle_t mutex, int32_t *pxHigherPriorityTaskWoken);
```

#### Управление задачами (tasks.h)

```cpp
// Приоритеты задач
enum Priority { Idle = 0, Low = 5, Middle = 10, High = 15, Critical = 20 };

// Создание и удаление задач
task_handle_t create_task(const char *name, void (*task_func)(void *),
                          void *params, uint32_t stack_size, Priority priority);
void delete_current_task();
void sleep_ms(uint32_t ms);
```

#### Система событий (events.h)

```cpp
// Управление событиями
event_t create_event();
void delete_event(event_t event);

// Подписка на события
void subscribe_to_event(event_t event, task_handle_t task);
void unsubscribe_from_event(event_t event, task_handle_t task);

// Генерация событий
void raise_event(event_t event);
void raise_event_from_ISR(event_t event);  // ISR-безопасный вариант

// Ожидание событий
bool wait_event(event_t event);
bool wait_event_timed(event_t event, uint32_t timeout_ms);
```

#### Прочие компоненты

| Файл | Назначение |
|------|------------|
| `errors.h` | Определения кодов ошибок |
| `logging.h` | Макросы логирования (ESP_LOG*) |
| `system.h` | Включения FreeRTOS |
| `timer.h` | Работа с таймерами |

### Портирование на другие платформы

Для добавления поддержки новой платформы необходимо реализовать аналогичные интерфейсы в соответствующей папке `os/<platform>/`. Существующие реализации для Linux/POSIX и ESP8266 могут служить примером.
