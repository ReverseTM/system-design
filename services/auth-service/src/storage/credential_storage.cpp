#include "credential_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/query.hpp>

namespace storage {
    namespace {
        const userver::storages::postgres::Query kInsertCredentials
        {
            R"(
                INSERT INTO credentials (login, password_hash)
                VALUES ($1, $2)
                ON CONFLICT (login) DO NOTHING
                RETURNING id;
            )"
        };

        const userver::storages::postgres::Query kSelectPasswordHash
        {
            "SELECT password_hash FROM credentials WHERE login = $1;"
        };

        const userver::storages::postgres::Query kDeleteCredentials
        {
            "DELETE FROM credentials WHERE login = $1;"
        };
    }

    CredentialStorage::CredentialStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        cluster_(context.FindComponent<userver::components::Postgres>("db-postgres").GetCluster())
    {

    }

    int64_t CredentialStorage::SaveCredentials(const std::string& login, const std::string& password_hash)
    {
        auto result = cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, kInsertCredentials, login, password_hash);
        if (result.IsEmpty())
        {
            return 0;
        }

        return result.AsSingleRow<int64_t>();
    }

    void CredentialStorage::DeleteCredentials(const std::string& login)
    {
        cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, kDeleteCredentials, login);
    }

    std::optional<std::string> CredentialStorage::GetPasswordHash(const std::string& login) const
    {
        auto result = cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, kSelectPasswordHash, login);
        if (result.IsEmpty())
        {
            return std::nullopt;
        }

        return result.AsSingleRow<std::string>();
    }

}
