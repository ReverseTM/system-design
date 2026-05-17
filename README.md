# Сервис доставки

## Оглавление

- [Архитектура](#архитектура)
- [Оптимизация производительности](#оптимизация-производительности)
  - [Кеширование](#кеширование)
  - [Rate Limiting](#rate-limiting)
- [Схема баз данных](#схема-баз-данных)
  - [auth-service](#auth-service)
  - [user-service](#user-service)
  - [package-service](#package-service)
  - [delivery-service](#delivery-service)
  - [Оптимизации запросов](#оптимизации-запросов-к-таблицам)
- [Запуск сервисов](#запуск-сервисов)
- [Заполнение данных в БД](#заполнение-данных-в-бд)
- [API](#api)
  - [Auth Service API](#auth-service--v1auth-без-jwt)
  - [User Service API](#user-service--v1users--jwt)
  - [Package Service API](#package-service--v1packages--jwt)
  - [Delivery Service API](#delivery-service--v1deliveries--jwt)
- [Примеры использования](#примеры-использования)
  - [Аутентификация](#аутентификация)
  - [Пользователи](#пользователи)
  - [Посылки](#посылки)
  - [Доставки](#доставки)

## Архитектура

Микросервисная система управления доставками. Включает 5 сервисов, RabbitMQ, Redis, nginx-шлюз с JWT-аутентификацией и Swagger UI.

```
Client -> nginx:8080 -> auth-service -> user-service -> package-service -> delivery-service
                                                                                  |
                                                                              RabbitMQ
                                                                                  |
                                                                        notification-service
```

Все запросы проходят через nginx. Защищённые эндпоинты требуют заголовок `Authorization: Bearer <token>` — nginx проверяет токен через `auth-service` перед проксированием.

### Event-Driven архитектура

При создании доставки `delivery-service` публикует событие `delivery.created` в RabbitMQ (exchange: `delivery-events`, тип: topic). `notification-service` подписан на очередь `notification-queue` и обрабатывает все события доставки.

Подробнее: [event_driven_design.md](./event_driven_design.md) | [event_catalog.md](./event_catalog.md)

## Оптимизация производительности

Подробное описание стратегий: [performance_design.md](./performance_design.md)

### Кеширование

Используется Redis (Cache-Aside стратегия) для двух наиболее нагруженных `GET`-endpoint'ов:

| Endpoint                       | Сервис          | TTL    | Инвалидация             |
|--------------------------------|-----------------|--------|-------------------------|
| `GET /v1/users?login=...`      | user-service    | 300 с  | При `POST /v1/users`    |
| `GET /v1/packages?user_id=...` | package-service | 120 с  | При `POST /v1/packages` |

Логика: при запросе сначала проверяется Redis. При cache miss — данные берутся из БД и кладутся в Redis с TTL. При создании записи соответствующий ключ явно удаляется.

### Rate Limiting

`POST /v1/auth/login` — Fixed Window Counter через Redis INCR:

- **Лимит**: 10 запросов / 60 секунд на IP (IP берётся из заголовка `X-Real-IP`)
- **При превышении**: `HTTP 429 Too Many Requests`
- **Заголовки ответа**: `X-RateLimit-Limit`, `X-RateLimit-Remaining`, `X-RateLimit-Reset`

```bash
# Пример ответа при превышении лимита
HTTP/1.1 429 Too Many Requests
X-RateLimit-Limit: 10
X-RateLimit-Remaining: 0
X-RateLimit-Reset: 60
{"code": 429, "message": "Too many requests. Please try again later."}
```

---

## Схема баз данных

### auth-service

> Хранилище — **PostgreSQL** (таблица `credentials`).

**`credentials`** — учётные данные пользователей

| Колонка         | Тип          | Описание                          |
|-----------------|--------------|-----------------------------------|
| `id`            | BIGSERIAL PK | Внутренний идентификатор          |
| `login`         | VARCHAR(64)  | Уникальный логин (мин. 3 символа) |
| `password_hash` | VARCHAR(255) | Хеш пароля                        |

Индексы: уникальный по `login`.

Запросы к этой таблице [запросы](./postgres/auth-service/queries.sql)

---

### user-service

> Хранилище — **PostgreSQL** (таблица `users`).

**`users`** — профили пользователей

| Колонка      | Тип           | Описание                                      |
|--------------|---------------|-----------------------------------------------|
| `id`         | BIGSERIAL PK  | Внутренний идентификатор записи               |
| `user_id`    | BIGINT        | Уникальный ID пользователя (из auth-service)  |
| `login`      | VARCHAR(32)   | Уникальный логин (мин. 3 символа)             |
| `email`      | VARCHAR(64)   | Уникальный e-mail (валидный формат)           |
| `first_name` | VARCHAR(64)   | Имя (непустое)                                |
| `last_name`  | VARCHAR(64)   | Фамилия (непустая)                            |
| `created_at` | TIMESTAMPTZ   | Время создания                                |

Индексы: уникальный по `user_id`, `login`, `email`; GIN-индекс по `first_name || ' ' || last_name` (триграммный поиск).

Запросы к этой таблице [запросы](./postgres/user-service/queries.sql)

---

### package-service

> Хранилище — **MongoDB** (коллекция `packages`).

**Структура документа:**

| Поле                | Тип      | Описание                      |
|---------------------|----------|-------------------------------|
| `_id`               | ObjectId | Идентификатор посылки         |
| `user_id`           | Long     | ID владельца посылки          |
| `weight`            | Double   | Вес в кг (> 0)                |
| `dimensions.length` | Double   | Длина в см (> 0)              |
| `dimensions.width`  | Double   | Ширина в см (> 0)             |
| `dimensions.height` | Double   | Высота в см (> 0)             |
| `description`       | String   | Описание (макс. 255 символов) |
| `created_at`        | Date     | Время создания                |

`dimensions` — embedded document (габариты всегда читаются и пишутся вместе).

Индексы: составной по `{ user_id: 1, created_at: -1 }`.

[Документная модель и обоснование](./mongo/package-service/schema_design.md) · [CRUD-запросы](./mongo/package-service/queries.js) · [Валидация схемы](./mongo/package-service/validation.js)

---

### delivery-service

> Хранилище — **PostgreSQL** (таблица `deliveries`).

**`deliveries`** — доставки

| Колонка        | Тип          | Описание                                                           |
|----------------|--------------|--------------------------------------------------------------------|
| `id`           | BIGSERIAL PK | Идентификатор доставки                                             |
| `sender_id`    | BIGINT       | ID отправителя                                                     |
| `recipient_id` | BIGINT       | ID получателя (≠ sender_id)                                        |
| `package_id`   | VARCHAR(24)  | Уникальный ID посылки (MongoDB ObjectId)                           |
| `address`      | VARCHAR(128) | Адрес доставки (непустой)                                          |
| `status`       | VARCHAR(32)  | Статус: `created`, `in_progress`, `delivered`, `cancelled`         |
| `created_at`   | TIMESTAMPTZ  | Время создания                                                     |

Индексы: уникальный по `package_id`; по `sender_id`; по `recipient_id`.

Запросы к этой таблице [запросы](./postgres/delivery-service/queries.sql)

### Оптимизации запросов к таблицам

[Файл с оптимизациями](./postgres/optimization.md)

## Запуск сервисов

Собрать docker образы всех сервисов

```bash
make docker/build
```

Собрать docker образ конкретного сервиса

```bash
make docker/build/auth-service
make docker/build/user-service
make docker/build/package-service
make docker/build/delivery-service
make docker/build/notification-service
```

Запустить все сервисы и базы данных

```bash
make services/start
```

### RabbitMQ

После запуска RabbitMQ Management UI доступен по адресу:

```
http://localhost:15672
login: guest / password: guest
```

Проверить получение событий:

```bash
# Создать доставку (публикует delivery.created в RabbitMQ)
curl -X POST http://localhost:8080/v1/deliveries \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{"sender_id": 1, "recipient_id": 2, "package_id": "<package_id>"}'

# Посмотреть логи notification-service (consumer)
docker logs notification-service -f
```

Остановить все сервисы и базы данных

```bash
make services/stop
```

Swagger UI доступен по адресу: `http://localhost:8080/docs/`

## Заполнение данных в БД

Заполнить все БД тестовыми данными

```bash
make db/fill
```

Заполнить БД конкретного сервиса

```bash
make db/fill/auth-service
make db/fill/user-service
make db/fill/delivery-service
```

Данные берутся из файлов `postgres/<service>/data.sql`.

### package-service (MongoDB)

Тестовые данные для package-service загружаются автоматически при первом запуске контейнера `package-service-mongo` из файла `mongo/package-service/data.js`.

Для ручного запуска:

```bash
docker compose exec package-service-mongo mongosh package-service /docker-entrypoint-initdb.d/02_data.js
```

## API

### Auth Service — `/v1/auth/...` (без JWT)

| Метод | Путь                 | Описание                                       |
|-------|----------------------|------------------------------------------------|
| POST  | `/v1/auth/register`  | Регистрация нового пользователя                |
| POST  | `/v1/auth/login`     | Вход, возвращает JWT-токен                     |
| GET   | `/v1/auth/validate`  | Проверка токена (используется nginx внутренне) |

### User Service — `/v1/users/...` (🔒 JWT)

| Метод | Путь                  | Описание                               |
|-------|-----------------------|----------------------------------------|
| POST  | `/v1/users`           | Создание пользователя                  |
| GET   | `/v1/users?login=...` | Поиск пользователя по логину           |
| GET   | `/v1/users?mask=...`  | Поиск пользователей по маске имени     |
| GET   | `/v1/users/{id}`      | Получение пользователя по ID           |

### Package Service — `/v1/packages/...` (🔒 JWT)

| Метод | Путь                       | Описание                        |
|-------|----------------------------|---------------------------------|
| POST  | `/v1/packages`             | Создание посылки                |
| GET   | `/v1/packages?user_id=...` | Получение посылок пользователя  |

### Delivery Service — `/v1/deliveries/...` (🔒 JWT)

| Метод | Путь                              | Описание                                                             |
|-------|-----------------------------------|----------------------------------------------------------------------|
| POST  | `/v1/deliveries`                  | Создание доставки (проверяет существование отправителя и получателя) |
| GET   | `/v1/deliveries?sender_id=...`    | Получение доставок по отправителю                                    |
| GET   | `/v1/deliveries?recipient_id=...` | Получение доставок по получателю                                     |

## Примеры использования

### Аутентификация

**Регистрация**

```bash
curl -X POST http://localhost:8080/v1/auth/register \
  -H 'Content-Type: application/json' \
  -d '{"login":"alice","password":"secret","email":"alice@example.com","first_name":"Alice","last_name":"Smith"}'
```

**Вход и сохранение токена**

```bash
TOKEN=$(curl -s -X POST http://localhost:8080/v1/auth/login \
  -H 'Content-Type: application/json' \
  -d '{"login":"alice","password":"secret"}' | jq -r '.token')
```

### Пользователи

**Поиск по логину**

```bash
curl "http://localhost:8080/v1/users?login=alice" \
  -H "Authorization: Bearer $TOKEN"
```

**Поиск по маске имени**

```bash
curl "http://localhost:8080/v1/users?mask=Ali" \
  -H "Authorization: Bearer $TOKEN"
```

**Получение пользователя по ID**

```bash
curl http://localhost:8080/v1/users/1 \
  -H "Authorization: Bearer $TOKEN"
```

### Посылки

**Создание посылки**

```bash
curl -X POST http://localhost:8080/v1/packages \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"user_id":1,"weight":2.5,"length":3,"width":5,"height":10,"description":"Книги"}'
```

**Получение посылок пользователя**

```bash
curl "http://localhost:8080/v1/packages?user_id=1" \
  -H "Authorization: Bearer $TOKEN"
```

### Доставки

Создание второго пользователя для успешного создания доставки

```bash
curl -X POST http://localhost:8080/v1/auth/register \
  -H 'Content-Type: application/json' \
  -d '{"login":"bob","password":"secret","email":"bob@example.com","first_name":"Bob","last_name":"Smith"}'
```

**Создание доставки**

```bash
curl -X POST http://localhost:8080/v1/deliveries \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"sender_id":1,"recipient_id":2,"package_id":"6629a1f3e4b0c123456789ab","address":"Moscow, Lenina 1"}'
```

**Получение доставок по отправителю**

```bash
curl "http://localhost:8080/v1/deliveries?sender_id=1" \
  -H "Authorization: Bearer $TOKEN"
```

**Получение доставок по получателю**

```bash
curl "http://localhost:8080/v1/deliveries?recipient_id=2" \
  -H "Authorization: Bearer $TOKEN"
```
