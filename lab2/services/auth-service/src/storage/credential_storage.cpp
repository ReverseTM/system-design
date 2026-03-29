#include "credential_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/sqlite/component.hpp>
#include <userver/storages/sqlite/operation_types.hpp>
#include <userver/storages/sqlite/query.hpp>

namespace storage {
    namespace {
        const userver::storages::sqlite::Query kCreateTable
        {
            R"(
                CREATE TABLE IF NOT EXISTS credentials (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    login TEXT UNIQUE NOT NULL,
                    password_hash TEXT NOT NULL
                );
            )"
        };

        const userver::storages::sqlite::Query kInsertUser
        {
            "INSERT OR IGNORE INTO credentials (login, password_hash) VALUES (?, ?) RETURNING id;"
        };

        const userver::storages::sqlite::Query kSelectPasswordHash
        {
            "SELECT password_hash FROM credentials WHERE login = ?;"
        };

        const userver::storages::sqlite::Query kDeleteUser
        {
            "DELETE FROM credentials WHERE login = ?;"
        };
    }

    CredentialStorage::CredentialStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context), client_(context.FindComponent<userver::components::SQLite>("sqlite-db").GetClient())
    {
        client_->Execute(userver::storages::sqlite::OperationType::kReadWrite, kCreateTable);
    }

    int64_t CredentialStorage::SaveCredentials(const std::string& login, const std::string& password_hash)
    {
        struct Row { int64_t id; };

        auto result = client_->Execute(userver::storages::sqlite::OperationType::kReadWrite, kInsertUser, login, password_hash);
        auto rows = std::move(result).AsVector<Row>();
        if (rows.empty())
        {
            return 0;
        }

        return rows.front().id;
    }

    void CredentialStorage::DeleteCredentials(const std::string& login)
    {
        client_->Execute(userver::storages::sqlite::OperationType::kReadWrite, kDeleteUser, login);
    }

    std::optional<std::string> CredentialStorage::GetPasswordHash(const std::string& login) const
    {
        struct Row 
        {
            std::string password_hash;
        };

        auto result = client_->Execute(userver::storages::sqlite::OperationType::kReadOnly, kSelectPasswordHash, login);
        auto rows = std::move(result).AsVector<Row>();
        if (rows.empty())
        {
            return std::nullopt;
        }

        return rows.front().password_hash;
    }

}
