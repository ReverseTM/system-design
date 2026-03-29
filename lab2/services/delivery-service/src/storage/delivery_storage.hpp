#pragma once

#include <string>

#include <userver/components/component_base.hpp>
#include <userver/storages/sqlite/client.hpp>

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
        private:
            userver::storages::sqlite::ClientPtr client_;
    };
}
