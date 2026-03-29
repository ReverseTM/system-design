#include "delivery_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/sqlite/component.hpp>
#include <userver/storages/sqlite/operation_types.hpp>
#include <userver/storages/sqlite/query.hpp>

namespace storage {
    namespace {
        const userver::storages::sqlite::Query kCreateTable
        {
            R"(
                CREATE TABLE IF NOT EXISTS deliveries (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    sender_id INTEGER NOT NULL,
                    recipient_id INTEGER NOT NULL,
                    package_id INTEGER NOT NULL,
                    address TEXT NOT NULL,
                    status TEXT NOT NULL DEFAULT 'created',
                    created_at TIMESTAMP NOT NULL DEFAULT (datetime('now'))
                );
            )"
        };

        const userver::storages::sqlite::Query kInsertDelivery
        {
            R"(
                INSERT INTO deliveries (sender_id, recipient_id, package_id, address)
                VALUES (?, ?, ?, ?)
                RETURNING id;
            )"
        };
    }

    DeliveryStorage::DeliveryStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        client_(context.FindComponent<userver::components::SQLite>("sqlite-db").GetClient())
    {
        client_->Execute(userver::storages::sqlite::OperationType::kReadWrite, kCreateTable);
    }

    int64_t DeliveryStorage::CreateDelivery(const delivery::dto::CreateDeliveryRequestBody& request) const
    {
        struct Row { int64_t id; };

        auto result = client_->Execute(
            userver::storages::sqlite::OperationType::kReadWrite, kInsertDelivery,
            request.sender_id, request.recipient_id, request.package_id, request.address
        );

        return std::move(result).AsVector<Row>().front().id;
    }
}
