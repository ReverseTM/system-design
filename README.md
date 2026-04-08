# Сервис доставки

## Оглавление

- [Архитектура](#архитектура)
- [Схема базы данных](#схема-базы-данных)
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

Микросервисная система управления доставками. Включает 4 сервиса, nginx-шлюз с JWT-аутентификацией и Swagger UI.

```
Client -> nginx:8080 -> auth-service -> user-service -> package-service -> delivery-service
```

Все запросы проходят через nginx. Защищённые эндпоинты требуют заголовок `Authorization: Bearer <token>` — nginx проверяет токен через `auth-service` перед проксированием.

## Схема базы данных

### auth-service

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

**`packages`** — посылки

| Колонка       | Тип              | Описание                             |
|---------------|------------------|--------------------------------------|
| `id`          | BIGSERIAL PK     | Идентификатор посылки                |
| `user_id`     | BIGINT           | ID владельца посылки                 |
| `weight`      | DOUBLE PRECISION | Вес (> 0)                            |
| `length`      | DOUBLE PRECISION | Длина (> 0)                          |
| `width`       | DOUBLE PRECISION | Ширина (> 0)                         |
| `height`      | DOUBLE PRECISION | Высота (> 0)                         |
| `description` | VARCHAR(255)     | Описание (по умолчанию пустая строка)|
| `created_at`  | TIMESTAMPTZ      | Время создания                       |

Индексы: составной по `(user_id, created_at DESC, id DESC)`.

Запросы к этой таблице [запросы](./postgres/package-service/queries.sql)

---

### delivery-service

**`deliveries`** — доставки

| Колонка        | Тип          | Описание                                                           |
|----------------|--------------|--------------------------------------------------------------------|
| `id`           | BIGSERIAL PK | Идентификатор доставки                                             |
| `sender_id`    | BIGINT       | ID отправителя                                                     |
| `recipient_id` | BIGINT       | ID получателя (≠ sender_id)                                        |
| `package_id`   | BIGINT       | Уникальный ID посылки                                              |
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
```

Запустить все сервисы и базы данных

```bash
make services/start
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
make db/fill/package-service
make db/fill/delivery-service
```

Данные берутся из файлов `postgres/<service>/data.sql`.

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
  -d '{"sender_id":1,"recipient_id":2,"package_id":10,"address":"Moscow, Lenina 1"}'
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
