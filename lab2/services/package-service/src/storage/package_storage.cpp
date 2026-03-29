#include "package_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/sqlite/component.hpp>
#include <userver/storages/sqlite/operation_types.hpp>
#include <userver/storages/sqlite/query.hpp>

namespace storage {
    namespace {
        const userver::storages::sqlite::Query kCreateTable
        {
            R"(
                CREATE TABLE IF NOT EXISTS packages (
                    id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                    user_id INTEGER NOT NULL,
                    weight REAL NOT NULL,
                    length REAL NOT NULL,
                    width REAL NOT NULL,
                    height REAL NOT NULL,
                    description TEXT NOT NULL DEFAULT '',
                    created_at TIMESTAMP NOT NULL DEFAULT (datetime('now'))
                );
            )"
        };

        const userver::storages::sqlite::Query kInsertPackage
        {
            R"(
                INSERT INTO packages (user_id, weight, length, width, height, description)
                VALUES (?, ?, ?, ?, ?, ?)
                RETURNING id;
            )"
        };

        const userver::storages::sqlite::Query kSelectByUser
        {
            R"(
                SELECT id, user_id, weight, length, width, height, description, created_at
                FROM packages WHERE user_id = ? ORDER BY created_at DESC, id DESC;
            )"
        };
    }

    PackageStorage::PackageStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        client_(context.FindComponent<userver::components::SQLite>("sqlite-db").GetClient())
    {
        client_->Execute(userver::storages::sqlite::OperationType::kReadWrite, kCreateTable);
    }

    int64_t PackageStorage::CreatePackage(const package::dto::CreatePackageRequestBody& request) const
    {
        struct Row { int64_t id; };

        auto result = client_->Execute(
            userver::storages::sqlite::OperationType::kReadWrite, kInsertPackage,
            request.user_id, request.weight, request.length, request.width, request.height, request.description
        );

        return std::move(result).AsVector<Row>().front().id;
    }

    std::vector<package::dto::Package> PackageStorage::GetPackagesByUser(int64_t user_id) const
    {
        struct Row
        {
            int64_t id;
            int64_t user_id;
            double  weight;
            double  length;
            double  width;
            double  height;
            std::string description;
            std::string created_at;
        };

        auto result = client_->Execute(userver::storages::sqlite::OperationType::kReadOnly, kSelectByUser, user_id);
        auto rows = std::move(result).AsVector<Row>();

        std::vector<package::dto::Package> packages;
        packages.reserve(rows.size());
        for (auto& row : rows)
        {
            packages.push_back(package::dto::Package{
                .id          = row.id,
                .user_id     = row.user_id,
                .weight      = row.weight,
                .length      = row.length,
                .width       = row.width,
                .height      = row.height,
                .description = row.description,
                .created_at  = row.created_at,
            });
        }

        return packages;
    }
}
