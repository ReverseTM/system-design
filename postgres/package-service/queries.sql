-- Создание посылки

INSERT INTO packages (user_id, weight, length, width, height, description)
VALUES ($1, $2, $3, $4, $5, $6)
RETURNING id;

-- Получение посылок по id пользователя, отсортированных по дате создания

SELECT id, user_id, weight, length, width, height, description, created_at
FROM packages
WHERE user_id = $1
ORDER BY created_at DESC, id DESC;
