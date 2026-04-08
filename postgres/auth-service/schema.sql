CREATE TABLE IF NOT EXISTS credentials (
    id BIGSERIAL PRIMARY KEY,
    login VARCHAR(64) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,

    CONSTRAINT chk_credentials_login_length CHECK (LENGTH(TRIM(login)) >= 3)
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_credentials_login ON credentials(login);
