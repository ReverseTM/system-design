CREATE TABLE IF NOT EXISTS users (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    login VARCHAR(32) NOT NULL,
    email VARCHAR(64) NOT NULL,
    first_name VARCHAR(64) NOT NULL,
    last_name VARCHAR(64) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT chk_users_login_length CHECK (LENGTH(TRIM(login)) >= 3),
    CONSTRAINT chk_users_email_format CHECK (email ~* '^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$'),
    CONSTRAINT chk_users_first_name_length CHECK (LENGTH(TRIM(first_name)) > 0),
    CONSTRAINT chk_users_last_name_length CHECK (LENGTH(TRIM(last_name)) > 0)
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_users_user_id ON users(user_id);

CREATE UNIQUE INDEX IF NOT EXISTS idx_users_login ON users(login);

CREATE UNIQUE INDEX IF NOT EXISTS idx_users_email ON users(email);

CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX IF NOT EXISTS idx_users_fullname_trgm ON users USING GIN ((first_name || ' ' || last_name) gin_trgm_ops);
