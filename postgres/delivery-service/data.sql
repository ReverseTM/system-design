TRUNCATE TABLE deliveries RESTART IDENTITY CASCADE;

DO $$
DECLARE
statuses  TEXT[] := ARRAY['created', 'in_progress', 'delivered', 'cancelled'];
addresses TEXT[] := ARRAY['ul. Lenina 1', 'pr. Mira 42', 'ul. Gagarina 7', 'ul. Pushkina 13', 'bulvar Cvetnoy 5'];

BEGIN
INSERT INTO deliveries (sender_id, recipient_id, package_id, address, status)
SELECT
    g.id,
    (g.id % 999999) + 1,
    g.id,
    addresses[(g.id % array_length(addresses, 1)) + 1],
    statuses[(g.id % array_length(statuses, 1)) + 1]
FROM generate_series(1, 1000000) AS g(id);

RAISE NOTICE 'Generated 1.000.000 deliveries.';
END $$;

ANALYZE deliveries;