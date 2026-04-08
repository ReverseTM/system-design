TRUNCATE TABLE users CASCADE;

DO $$
DECLARE
first_names TEXT[] := ARRAY['Ivan', 'Dmitry', 'Alexey', 'Sergey', 'Andrey', 'Anna', 'Maria', 'Elena', 'Olga', 'Natalia'];
last_names  TEXT[] := ARRAY['Ivanov', 'Smirnov', 'Kuznetsov', 'Popov', 'Vasiliev', 'Petrov', 'Sokolov', 'Novikov', 'Fedorov', 'Volkov'];

BEGIN
INSERT INTO users (user_id, login, email, first_name, last_name)
SELECT
    g.id,
    'user_' || g.id,
    'user_' || g.id || '@example.com',
    first_names[(g.id % array_length(first_names, 1)) + 1],
    last_names[(g.id  % array_length(last_names,  1)) + 1]
FROM generate_series(1, 1000000) AS g(id);

RAISE NOTICE 'Generated 1.000.000 users.';
END $$;

ANALYZE users;