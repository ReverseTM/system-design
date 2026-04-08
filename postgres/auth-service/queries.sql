-- Создание реквизитов пользователя для входа

INSERT INTO credentials (login, password_hash)
VALUES ($1, $2)
ON CONFLICT (login) DO NOTHING
RETURNING id;

-- Получение хэш пароля пользователя по логину

SELECT password_hash FROM credentials WHERE login = $1;

-- Удаление реквизитов пользователя для входа по логину

DELETE FROM credentials WHERE login = $1;
