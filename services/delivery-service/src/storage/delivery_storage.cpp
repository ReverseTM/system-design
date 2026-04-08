#include "delivery_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/query.hpp>

namespace storage {
    namespace {
        const userver::storages::postgres::Query kInsertDelivery
        {
            R"(
                INSERT INTO deliveries (sender_id, recipient_id, package_id, address)
                VALUES ($1, $2, $3, $4)
                RETURNING id;
            )"
        };

        const userver::storages::postgres::Query kSelectDeliveriesBySenderId
        {
            R"(
                SELECT id, sender_id, recipient_id, package_id, address, status, created_at::text
                FROM deliveries
                WHERE sender_id = $1;
            )"
        };

        const userver::storages::postgres::Query kSelectDeliveriesByRecipientId
        {
            R"(
                SELECT id, sender_id, recipient_id, package_id, address, status, created_at::text
                FROM deliveries
                WHERE recipient_id = $1;
            )"
        };
    }

    DeliveryStorage::DeliveryStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        cluster_(context.FindComponent<userver::components::Postgres>("db-postgres").GetCluster())
    {

    }

    int64_t DeliveryStorage::CreateDelivery(const delivery::dto::CreateDeliveryRequestBody& request) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster, kInsertDelivery,
            request.sender_id, request.recipient_id, request.package_id, request.address
        );

        return result.AsSingleRow<int64_t>();
    }

    std::vector<delivery::dto::Delivery> DeliveryStorage::GetDeliveriesBySenderId(int64_t sender_id) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave, kSelectDeliveriesBySenderId,
            sender_id
        );

        return result.AsContainer<std::vector<delivery::dto::Delivery>>(userver::storages::postgres::kRowTag);
    }

    std::vector<delivery::dto::Delivery> DeliveryStorage::GetDeliveriesByRecipientId(int64_t recipient_id) const
    {
        auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave, kSelectDeliveriesByRecipientId,
            recipient_id
        );

        return result.AsContainer<std::vector<delivery::dto::Delivery>>(userver::storages::postgres::kRowTag);
    }
}
