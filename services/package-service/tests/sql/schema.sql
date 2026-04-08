CREATE TABLE IF NOT EXISTS packages (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    weight DOUBLE PRECISION NOT NULL,
    length DOUBLE PRECISION NOT NULL,
    width DOUBLE PRECISION NOT NULL,
    height DOUBLE PRECISION NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT chk_packages_weight CHECK (weight > 0),
    CONSTRAINT chk_packages_length CHECK (length > 0),
    CONSTRAINT chk_packages_width CHECK (width > 0),
    CONSTRAINT chk_packages_height CHECK (height > 0)
);

CREATE INDEX IF NOT EXISTS idx_packages_user_created ON packages(user_id, created_at DESC, id DESC);
