#pragma once

#include <string>

#include <userver/clients/http/client.hpp>
#include <userver/components/component_base.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include <producer/delivery_producer.hpp>
#include <storage/delivery_storage.hpp>
#include <schemas/delivery.hpp>

namespace usecase {

    struct UserNotFoundError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class DeliveryUseCase final : public userver::components::ComponentBase
    {
        public:
            static constexpr std::string_view kName = "usecase-delivery";

            static userver::yaml_config::Schema GetStaticConfigSchema();

            DeliveryUseCase(
                const userver::components::ComponentConfig&,
                const userver::components::ComponentContext&
            );

            delivery::dto::CreateDeliveryResponseBody CreateDelivery(delivery::dto::CreateDeliveryRequestBody&&) const;
            delivery::dto::GetDeliveriesResponseBody GetDeliveriesBySenderId(int64_t sender_id) const;
            delivery::dto::GetDeliveriesResponseBody GetDeliveriesByRecipientId(int64_t recipient_id) const;

        private:
            void CheckUserExists(int64_t user_id) const;

            storage::DeliveryStorage& storage_;
            producer::DeliveryProducer& producer_;
            userver::clients::http::Client& http_client_;
            std::string user_service_url_;
    };
}
