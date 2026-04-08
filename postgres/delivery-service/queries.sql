-- Создание доставки

INSERT INTO deliveries (sender_id, recipient_id, package_id, address)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- Получение доставок по отправителю

SELECT id, sender_id, recipient_id, package_id, address, status, created_at
FROM deliveries
WHERE sender_id = $1;

-- Получение доставок по получателю

SELECT id, sender_id, recipient_id, package_id, address, status, created_at
FROM deliveries
WHERE recipient_id = $1;
