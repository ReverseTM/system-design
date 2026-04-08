TRUNCATE TABLE credentials RESTART IDENTITY CASCADE;

DO $$
BEGIN
INSERT INTO credentials (login, password_hash)
SELECT
    'user_' || g.id,
    '$2b$12$' || substr(md5('secret_' || g.id::text), 1, 50)
FROM generate_series(1, 1000000) AS g(id);

RAISE NOTICE 'Generated 1.000.000 credentials.';
END $$;

ANALYZE credentials;