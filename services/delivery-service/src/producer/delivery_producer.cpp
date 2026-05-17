#include "delivery_producer.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/urabbitmq/admin_channel.hpp>
#include <userver/urabbitmq/client.hpp>
#include <userver/urabbitmq/component.hpp>
#include <userver/utils/uuid4.hpp>

namespace producer {

namespace {

constexpr std::string_view kExchangeName = "delivery-events";

std::string BuildDeliveryCreatedPayload(
    int64_t delivery_id,
    int64_t sender_id,
    int64_t recipient_id
)
{
    userver::formats::json::ValueBuilder builder;
    builder["event_type"] = "delivery.created";
    builder["event_id"] = userver::utils::generators::GenerateUuid();
    builder["payload"]["delivery_id"] = delivery_id;
    builder["payload"]["sender_id"] = sender_id;
    builder["payload"]["recipient_id"] = recipient_id;
    builder["payload"]["status"] = "created";
    return userver::formats::json::ToString(builder.ExtractValue());
}

}

DeliveryProducer::DeliveryProducer(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
) : ComponentBase(config, context),
    client_(context.FindComponent<userver::components::RabbitMQ>("my-rabbit").GetClient())
{
    const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{5});

    client_->DeclareExchange(
        userver::urabbitmq::Exchange{std::string{kExchangeName}},
        userver::urabbitmq::Exchange::Type::kTopic,
        userver::urabbitmq::Exchange::Flags::kDurable,
        deadline
    );

    LOG_INFO() << "DeliveryProducer: exchange '" << kExchangeName << "' declared";
}

void DeliveryProducer::PublishDeliveryCreated(
    int64_t delivery_id,
    int64_t sender_id,
    int64_t recipient_id
) const
{
    const auto message = BuildDeliveryCreatedPayload(delivery_id, sender_id, recipient_id);
    const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{5});

    client_->PublishReliable(
        userver::urabbitmq::Exchange{std::string{kExchangeName}},
        "delivery.created",
        message,
        deadline
    );

    LOG_INFO() << "DeliveryProducer: published delivery.created for delivery_id=" << delivery_id;
}

}
