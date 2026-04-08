TRUNCATE TABLE packages RESTART IDENTITY CASCADE;

DO $$
BEGIN
INSERT INTO packages (user_id, weight, length, width, height, description)
SELECT
    (g.id % 1000000) + 1,
    round((random() * 49.9 + 0.1)::numeric, 2),
    round((random() * 199 + 1)::numeric, 2),
    round((random() * 199 + 1)::numeric, 2),
    round((random() * 199 + 1)::numeric, 2),
    'Package description #' || g.id
FROM generate_series(1, 1000000) AS g(id);

RAISE NOTICE 'Generated 1.000.000 packages.';
END $$;

ANALYZE packages;