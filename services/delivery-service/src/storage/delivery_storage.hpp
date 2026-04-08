#pragma once

#include <string>
#include <vector>

#include <userver/components/component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include <schemas/delivery.hpp>

namespace storage {
    class DeliveryStorage final : public userver::components::ComponentBase
    {
        public:
            static constexpr std::string_view kName = "storage-delivery";

            DeliveryStorage(
                const userver::components::ComponentConfig&,
                const userver::components::ComponentContext&
            );

            int64_t CreateDelivery(const delivery::dto::CreateDeliveryRequestBody& request) const;
            std::vector<delivery::dto::Delivery> GetDeliveriesBySenderId(int64_t sender_id) const;
            std::vector<delivery::dto::Delivery> GetDeliveriesByRecipientId(int64_t recipient_id) const;

        private:
            userver::storages::postgres::ClusterPtr cluster_;
    };
}