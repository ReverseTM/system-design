# Event-Driven архитектура: Сервис доставки

## Анализ событий

Система работает с тремя основными сущностями: пользователь, посылка, доставка. Из всех операций реально порождают события только операции с доставкой — создание и изменение статуса. Именно они интересны другим частям системы, потому что пользователь ожидает уведомления о том, что с его посылкой что-то происходит.

Команды и их события:

| Команда | Кто выполняет | Событие |
|---|---|---|
| CreateDelivery | delivery-service | `delivery.created` |
| UpdateDeliveryStatus | delivery-service | `delivery.status_changed` |

Оба события потребляет `notification-service` — он отвечает за уведомление пользователей.

---

## Проектирование

### Почему RabbitMQ, а не Kafka

Kafka мощнее, но сложнее: нужен KRaft или ZooKeeper, тяжелее в настройке, pull-модель требует больше кода на стороне consumer. Для задачи «отправить уведомление при создании доставки» это избыточно.

RabbitMQ проще, userver его поддерживает через компонент `userver::rabbitmq`, push-модель — consumer просто переопределяет метод `Process()`. Этого достаточно.

### Топология

Один exchange типа `topic`, одна очередь:

```
delivery-service ──► [Exchange: delivery-events, topic]
                              │
                    ┌─────────┴──────────┐
             delivery.created    delivery.status_changed
                              │
                    [Queue: notification-queue]
                              │
                    notification-service
```

Exchange объявлен как `durable`, очередь тоже — чтобы сообщения пережили перезапуск брокера.

### Поток события от создания доставки

```
POST /v1/deliveries
        │
        ▼
CreateDeliveryHandler
        │
        ▼
DeliveryUseCase::CreateDelivery()
        │
        ├──► DeliveryStorage::CreateDelivery() ──► PostgreSQL
        │
        └──► DeliveryProducer::PublishDeliveryCreated()
                    │
                    ▼
              RabbitMQ: delivery-events
              routing_key: delivery.created
                    │
                    ▼
              notification-queue
                    │
                    ▼
              NotificationConsumer::Process()
```

---

## Формат сообщений и гарантии

Все события — JSON. Базовая структура:

```json
{
  "event_type": "delivery.created",
  "event_id": "uuid-v4",
  "payload": { ... }
}
```

Гарантии — `at-least-once`: producer использует Publisher Confirms, очередь durable. Consumer может получить одно сообщение дважды (например, при падении до ack) — при необходимости дедуплицируем по `event_id`.

---

## CQRS

В текущей системе разделение уже есть де-факто: write-операции идут через usecase и меняют состояние в БД, read-операции просто читают. События встраиваются в write-путь.

**Write (команды):**
- `CreateDelivery` → запись в PostgreSQL → публикация `delivery.created`
- `UpdateDeliveryStatus` → обновление в PostgreSQL → публикация `delivery.status_changed`

**Read (запросы):**
- `GetDeliveriesBySenderId` — читает из PostgreSQL
- `GetDeliveriesByRecipientId` — читает из PostgreSQL
- `GetPackagesByUserId` — читает из MongoDB

В будущем события можно использовать для поддержания отдельной read-модели (например, денормализованная таблица или Elasticsearch), чтобы не нагружать write-БД запросами.

---

## Итоговая архитектура

```
Client ──► nginx ──► user-service    (Postgres)
                 ├──► auth-service    (Postgres)
                 ├──► package-service (MongoDB)
                 └──► delivery-service (Postgres)
                            │
                            │ delivery.created
                            │ delivery.status_changed
                            ▼
                         RabbitMQ
                    (delivery-events, topic)
                            │
                            ▼
                   notification-service
                   (notification-queue)
```
