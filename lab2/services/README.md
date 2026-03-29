# Delivery Platform

Микросервисная система управления доставками. Включает 4 сервиса, nginx-шлюз с JWT-аутентификацией и Swagger UI.

## Архитектура

```
Client → nginx:8080 → auth-service
                    → user-service
                    → package-service
                    → delivery-service
```

Все запросы проходят через nginx. Защищённые эндпоинты требуют заголовок `Authorization: Bearer <token>` — nginx проверяет токен через `auth-service` перед проксированием.

## Запуск

```bash
docker compose up -d
```

Swagger UI доступен по адресу: `http://localhost:8080/docs/`

## API

### Auth Service — `/v1/auth/...` (без JWT)

| Метод | Путь | Описание |
|-------|------|----------|
| POST | `/v1/auth/register` | Регистрация нового пользователя |
| POST | `/v1/auth/login` | Вход, возвращает JWT-токен |
| GET | `/v1/auth/validate` | Проверка токена (используется nginx внутренне) |

### User Service — `/v1/users/...` (🔒 JWT)

| Метод | Путь | Описание |
|-------|------|----------|
| POST | `/v1/users` | Создание пользователя |
| GET | `/v1/users?login=...` или `/v1/users?mask=...` | Поиск пользователей по логину или маске имени |
| GET | `/v1/users/{id}` | Получение пользователя по ID |

### Package Service — `/v1/packages/...` (🔒 JWT)

| Метод | Путь | Описание |
|-------|------|----------|
| POST | `/v1/packages` | Создание посылки |
| GET | `/v1/packages?user_id=...` | Получение посылок пользователя |

### Delivery Service — `/v1/deliveries/...` (🔒 JWT)

| Метод | Путь | Описание |
|-------|------|----------|
| POST | `/v1/deliveries` | Создание доставки (проверяет существование отправителя и получателя) |

## Примеры использования

### Аутентификация

**Регистрация:**
```bash
curl -X POST http://localhost:8080/v1/auth/register \
  -H 'Content-Type: application/json' \
  -d '{"login":"alice","password":"secret","email":"alice@example.com","first_name":"Alice","last_name":"Smith"}'
# → {"id": 1, "message": "User registered successfully"}
```

**Вход и сохранение токена:**
```bash
TOKEN=$(curl -s -X POST http://localhost:8080/v1/auth/login \
  -H 'Content-Type: application/json' \
  -d '{"login":"alice","password":"secret"}' | jq -r '.token')
```

### Пользователи

**Поиск по логину:**
```bash
curl "http://localhost:8080/v1/users?login=alice"
```

**Поиск по маске имени:**
```bash
curl "http://localhost:8080/v1/users?mask=Ali"
```

**Получение пользователя по ID:**
```bash
curl http://localhost:8080/v1/users/1
# → {"id": 1, "login": "alice", "email": "alice@example.com", ...}
```

### Посылки

**Создание посылки:**
```bash
curl -X POST http://localhost:8080/v1/packages \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"user_id":1,"weight":2.5,"description":"Книги"}'
# → {"id": 10, "message": "Package created successfully"}
```

**Получение посылок пользователя:**
```bash
curl "http://localhost:8080/v1/packages?user_id=1" \
  -H "Authorization: Bearer $TOKEN"
```

### Доставки

**Создание доставки:**
```bash
curl -X POST http://localhost:8080/v1/deliveries \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"sender_id":1,"recipient_id":2,"package_id":10,"address":"Moscow, Lenina 1"}'
# → {"id": 5, "message": "Delivery created successfully"}
```