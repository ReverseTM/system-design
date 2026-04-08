#include "user_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/exceptions.hpp>
#include <userver/storages/postgres/query.hpp>

namespace storage {
    namespace {
        const userver::storages::postgres::Query kInsertUser
        {
            R"(
                INSERT INTO users (user_id, login, email, first_name, last_name)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING id;
            )"
        };

        const userver::storages::postgres::Query kSelectUserByLogin
        {
            R"(
                SELECT id, user_id, login, email, first_name, last_name, created_at::text
                FROM users
                WHERE login = $1;
            )"
        };

        const userver::storages::postgres::Query kSelectUserById
        {
            R"(
                SELECT id, user_id, login, email, first_name, last_name, created_at::text
                FROM users
                WHERE user_id = $1;
            )"
        };

        const userver::storages::postgres::Query kSelectUsersByMask
        {
            R"(
                SELECT id, user_id, login, email, first_name, last_name, created_at::text
                FROM users
                WHERE first_name || ' ' || last_name LIKE $1;
            )"
        };
    }

    UserStorage::UserStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        cluster_(context.FindComponent<userver::components::Postgres>("db-postgres").GetCluster())
    {

    }

    int64_t UserStorage::CreateUser(const user::dto::CreateUserRequestBody& request) const
    {
        try
        {
            auto result = cluster_->Execute(
                userver::storages::postgres::ClusterHostType::kMaster, kInsertUser,
                request.user_id, request.login, request.email, request.first_name, request.last_name
            );

            return result.AsSingleRow<int64_t>();
        }
        catch (const userver::storages::postgres::UniqueViolation& e)
        {
            throw UserAlreadyExistsError{e.what()};
        }
    }

    std::optional<user::dto::User> UserStorage::GetUserById(int64_t userId) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave, kSelectUserById,
            userId
        );
        if (result.IsEmpty())
        {
            return std::nullopt;
        }

        return result.AsSingleRow<user::dto::User>(userver::storages::postgres::kRowTag);
    }

    std::optional<user::dto::User> UserStorage::GetUserByLogin(const std::string& login) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave, kSelectUserByLogin,
            login
        );
        if (result.IsEmpty())
        {
            return std::nullopt;
        }

        return result.AsSingleRow<user::dto::User>(userver::storages::postgres::kRowTag);
    }

    std::vector<user::dto::User> UserStorage::GetUsersByMask(const std::string& mask) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave, kSelectUsersByMask,
            mask + "%"
        );

        return result.AsContainer<std::vector<user::dto::User>>(userver::storages::postgres::kRowTag);
    }

}
