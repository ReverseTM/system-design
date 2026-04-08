-- Создание пользователя

INSERT INTO users (id, login, email, first_name, last_name)
VALUES ($1, $2, $3, $4, $5)
RETURNING id;

-- Получение пользователя по лоигну

SELECT id, login, email, first_name, last_name, created_at
FROM users
WHERE login = $1;

-- Получение пользователя по id

SELECT id, login, email, first_name, last_name, created_at
FROM users
WHERE id = $1;

-- Получение пользователей по маске (имя и фамилия)

SELECT id, login, email, first_name, last_name, created_at
FROM users
WHERE first_name || ' ' || last_name LIKE $1;
