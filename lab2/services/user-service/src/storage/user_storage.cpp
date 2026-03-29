#include "user_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/sqlite/component.hpp>
#include <userver/storages/sqlite/exceptions.hpp>
#include <userver/storages/sqlite/operation_types.hpp>
#include <userver/storages/sqlite/query.hpp>

namespace storage {

    namespace {

        struct Row
        {
            int64_t id;
            std::string login;
            std::string email;
            std::string first_name;
            std::string last_name;
            std::string created_at;
        };

        const userver::storages::sqlite::Query kCreateTable{
            R"(
                CREATE TABLE IF NOT EXISTS users (
                    id         INTEGER PRIMARY KEY NOT NULL,
                    login      TEXT UNIQUE NOT NULL,
                    email      TEXT UNIQUE NOT NULL,
                    first_name TEXT NOT NULL,
                    last_name  TEXT NOT NULL,
                    created_at TIMESTAMP NOT NULL DEFAULT (datetime('now'))
                );
            )"
        };

        const userver::storages::sqlite::Query kInsertUser{
            R"(
                INSERT INTO users (id, login, email, first_name, last_name)
                VALUES (?, ?, ?, ?, ?)
				RETURNING id;
            )"
        };

        const userver::storages::sqlite::Query kSelectUserByLogin{
            "SELECT id, login, email, first_name, last_name, "
            "strftime('%Y-%m-%dT%H:%M:%SZ', created_at) AS created_at "
            "FROM users WHERE login = ?;"
        };

        const userver::storages::sqlite::Query kSelectUserById{
            "SELECT id, login, email, first_name, last_name, "
            "strftime('%Y-%m-%dT%H:%M:%SZ', created_at) AS created_at "
            "FROM users WHERE id = ?;"
        };

        const userver::storages::sqlite::Query kSelectUsersByMask{
            "SELECT id, login, email, first_name, last_name, "
            "strftime('%Y-%m-%dT%H:%M:%SZ', created_at) AS created_at "
            "FROM users WHERE first_name || ' ' || last_name LIKE ?;"
        };

    }  // namespace

    UserStorage::UserStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        client_(context.FindComponent<userver::components::SQLite>("sqlite-db").GetClient())
    {
        client_->Execute(userver::storages::sqlite::OperationType::kReadWrite, kCreateTable);
    }

    int64_t UserStorage::CreateUser(const user::dto::CreateUserRequestBody& request) const
    {
        static constexpr int kSQLiteConstraintUnique = 2067;
        static constexpr int kSQLiteConstraintPrimaryKey = 1555;

		struct Row { int64_t id; };

        try
        {
            auto result = client_->Execute(
                userver::storages::sqlite::OperationType::kReadWrite, kInsertUser,
                request.id, request.login, request.email, request.first_name, request.last_name
            );

			auto rows = std::move(result).AsVector<Row>();
			return rows.front().id;
        }
        catch (const userver::storages::sqlite::SQLiteException& e)
        {
            const auto code = e.getExtendedErrorCode();
            if (code == kSQLiteConstraintUnique || code == kSQLiteConstraintPrimaryKey)
            {
                throw UserAlreadyExistsError{e.what()};
            }
            throw;
        }
    }

    std::optional<user::dto::User> UserStorage::GetUserById(int64_t id) const
    {
        auto result = client_->Execute(
            userver::storages::sqlite::OperationType::kReadOnly, kSelectUserById, id
        );

        auto rows = std::move(result).AsVector<Row>();
        if (rows.empty())
        {
            return std::nullopt;
        }

        const auto& row = rows.front();
        return user::dto::User{
            .id         = row.id,
            .login      = row.login,
            .email      = row.email,
            .first_name = row.first_name,
            .last_name  = row.last_name,
            .created_at = row.created_at,
        };
    }

    std::optional<user::dto::User> UserStorage::GetUserByLogin(const std::string& login) const
    {
        auto result = client_->Execute(
            userver::storages::sqlite::OperationType::kReadOnly, kSelectUserByLogin, login
        );

        auto rows = std::move(result).AsVector<Row>();
        if (rows.empty())
        {
            return std::nullopt;
        }

        const auto& row = rows.front();
        return user::dto::User{
            .id         = row.id,
            .login      = row.login,
            .email      = row.email,
            .first_name = row.first_name,
            .last_name  = row.last_name,
            .created_at = row.created_at,
        };
    }

    std::vector<user::dto::User> UserStorage::GetUsersByMask(const std::string& mask) const
    {
        auto result = client_->Execute(
            userver::storages::sqlite::OperationType::kReadOnly, kSelectUsersByMask, mask + "%"
        );
        auto rows = std::move(result).AsVector<Row>();

        std::vector<user::dto::User> users;
        users.reserve(rows.size());
        for (const auto& row : rows)
        {
            users.push_back(user::dto::User{
                .id         = row.id,
                .login      = row.login,
                .email      = row.email,
                .first_name = row.first_name,
                .last_name  = row.last_name,
                .created_at = row.created_at,
            });
        }
        return users;
    }

}
