#include "delivery_usecase.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>

namespace usecase {

    userver::yaml_config::Schema DeliveryUseCase::GetStaticConfigSchema()
    {
        return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
            R"(
                type: object
                description: Delivery use case component
                additionalProperties: false
                properties:
                    user-service-url:
                        type: string
                        description: Base URL of user-service (e.g. http://user-service:8080)
            )"
        );
    }

    DeliveryUseCase::DeliveryUseCase(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        storage_(context.FindComponent<storage::DeliveryStorage>()),
        http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
        user_service_url_(config["user-service-url"].As<std::string>())
    {

    }

    void DeliveryUseCase::CheckUserExists(int64_t user_id) const
    {
        const auto url = user_service_url_ + "/users/" + std::to_string(user_id);
        try
        {
            const auto response = http_client_.CreateRequest()
                .get(url)
                .timeout(std::chrono::seconds(5))
                .perform();

            if (response->status_code() == 404)
            {
                throw UserNotFoundError{"User with id " + std::to_string(user_id) + " not found"};
            }
            if (response->status_code() != 200)
            {
                throw std::runtime_error{
                    "user-service returned unexpected status: "
                    + std::to_string(response->status_code())
                };
            }
        }
        catch (const userver::clients::http::HttpClientException& e)
        {
            throw std::runtime_error{
                std::string{"Failed to reach user-service: "} + e.what()
            };
        }
    }

    delivery::dto::CreateDeliveryResponseBody DeliveryUseCase::CreateDelivery(
        delivery::dto::CreateDeliveryRequestBody&& request
    ) const
    {
        CheckUserExists(request.sender_id);
        CheckUserExists(request.recipient_id);

        const auto id = storage_.CreateDelivery(request);

        return delivery::dto::CreateDeliveryResponseBody{
            .id      = id,
            .message = "Delivery created successfully",
        };
    }

    delivery::dto::GetDeliveriesResponseBody DeliveryUseCase::GetDeliveriesBySenderId(int64_t sender_id) const
    {
        return delivery::dto::GetDeliveriesResponseBody{.deliveries = storage_.GetDeliveriesBySenderId(sender_id)};
    }

    delivery::dto::GetDeliveriesResponseBody DeliveryUseCase::GetDeliveriesByRecipientId(int64_t recipient_id) const
    {
        return delivery::dto::GetDeliveriesResponseBody{.deliveries = storage_.GetDeliveriesByRecipientId(recipient_id)};
    }
}
