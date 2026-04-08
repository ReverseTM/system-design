CREATE TABLE IF NOT EXISTS deliveries (
    id BIGSERIAL PRIMARY KEY,
    sender_id BIGINT NOT NULL,
    recipient_id BIGINT NOT NULL,
    package_id BIGINT UNIQUE NOT NULL,
    address VARCHAR(128) NOT NULL,
    status VARCHAR(32) NOT NULL DEFAULT 'created',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT chk_deliveries_sender_recipient CHECK (sender_id != recipient_id),
    CONSTRAINT chk_deliveries_status CHECK (status IN ('created', 'in_progress', 'delivered', 'cancelled')),
    CONSTRAINT chk_deliveries_address CHECK (LENGTH(TRIM(address)) > 0)
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_deliveries_package_id ON deliveries(package_id);

CREATE INDEX IF NOT EXISTS idx_deliveries_sender_id ON deliveries(sender_id);

CREATE INDEX IF NOT EXISTS idx_deliveries_recipient_id ON deliveries(recipient_id);
