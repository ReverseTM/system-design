#include "package_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/query.hpp>

namespace storage {
    namespace {
        const userver::storages::postgres::Query kInsertPackage
        {
            R"(
                INSERT INTO packages (user_id, weight, length, width, height, description)
                VALUES ($1, $2, $3, $4, $5, $6)
                RETURNING id;
            )"
        };

        const userver::storages::postgres::Query kSelectByUser
        {
            R"(
                SELECT id, user_id, weight, length, width, height, description, created_at::text
                FROM packages
                WHERE user_id = $1
                ORDER BY created_at DESC, id DESC;
            )"
        };
    }

    PackageStorage::PackageStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        cluster_(context.FindComponent<userver::components::Postgres>("db-postgres").GetCluster())
    {

    }

    int64_t PackageStorage::CreatePackage(const package::dto::CreatePackageRequestBody& request) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster, kInsertPackage,
            request.user_id, request.weight, request.length, request.width, request.height, request.description
        );

        return result.AsSingleRow<int64_t>();
    }

    std::vector<package::dto::Package> PackageStorage::GetPackagesByUser(int64_t user_id) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave, kSelectByUser,
            user_id
        );

        return result.AsContainer<std::vector<package::dto::Package>>(userver::storages::postgres::kRowTag);
    }
}
