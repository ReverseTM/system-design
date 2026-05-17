# Event Catalog

Exchange: `delivery-events` (topic, durable)  
Queue: `notification-queue` (durable, binding: `delivery.#`)

---

## delivery.created

Публикуется сразу после того, как доставка успешно сохранена в БД.

**Producer:** delivery-service  
**Consumer:** notification-service  
**Гарантии:** at-least-once

```json
{
  "event_type": "delivery.created",
  "event_id": "3fa85f64-5717-4562-b3fc-2c963f66afa6",
  "payload": {
    "delivery_id": 42,
    "sender_id": 1,
    "recipient_id": 2,
    "status": "created"
  }
}
```

Триггер: `POST /v1/deliveries`

---

## delivery.status_changed

Публикуется при смене статуса доставки. Notification-service использует это, чтобы уведомить получателя, когда посылка вышла на доставку или доставлена.

**Producer:** delivery-service  
**Consumer:** notification-service  
**Гарантии:** at-least-once

```json
{
  "event_type": "delivery.status_changed",
  "event_id": "7fa85f64-5717-4562-b3fc-2c963f66bcd9",
  "payload": {
    "delivery_id": 42,
    "sender_id": 1,
    "recipient_id": 2,
    "old_status": "created",
    "new_status": "in_transit"
  }
}
```

Возможные значения `new_status`: `in_transit`, `delivered`, `cancelled`

Триггер: `PATCH /v1/deliveries/{id}/status`

---

## Статусы доставки

```
created → in_transit → delivered
    └──────────────→ cancelled
```

---

## Топология

```
delivery-service -> delivery-events (topic) -> notification-queue -> notification-service
```

| Queue              | Exchange        | Binding    |
|--------------------|-----------------|------------|
| notification-queue | delivery-events | delivery.# |
