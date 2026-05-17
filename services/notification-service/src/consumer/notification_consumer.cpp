#include "notification_consumer.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/logging/log.hpp>
#include <userver/urabbitmq/admin_channel.hpp>
#include <userver/urabbitmq/client.hpp>
#include <userver/urabbitmq/component.hpp>

namespace consumer {

namespace {

constexpr std::string_view kExchangeName = "delivery-events";
constexpr std::string_view kQueueName = "notification-queue";

}

NotificationConsumer::NotificationConsumer(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
) : ConsumerComponentBase(config, context)
{
    const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{5});
    auto client = context.FindComponent<userver::components::RabbitMQ>("my-rabbit").GetClient();

    const userver::urabbitmq::Exchange exchange{std::string{kExchangeName}};
    const userver::urabbitmq::Queue queue{std::string{kQueueName}};

    client->DeclareExchange(
        exchange,
        userver::urabbitmq::Exchange::Type::kTopic,
        userver::urabbitmq::Exchange::Flags::kDurable,
        deadline
    );

    client->DeclareQueue(
        queue,
        userver::urabbitmq::Queue::Flags::kDurable,
        deadline
    );

    client->BindQueue(exchange, queue, "delivery.#", deadline);

    LOG_INFO() << "NotificationConsumer: queue '" << kQueueName
               << "' declared and bound to exchange '" << kExchangeName << "'";
}

void NotificationConsumer::Process(std::string message)
{
    try
    {
        const auto json = userver::formats::json::FromString(message);
        const auto event_type = json["event_type"].As<std::string>("<unknown>");

        LOG_INFO() << "NotificationConsumer: received event type='" << event_type
                   << "' payload=" << message;

        if (event_type == "delivery.created")
        {
            const auto& payload = json["payload"];
            LOG_INFO() << "NotificationConsumer: delivery created"
                       << " delivery_id=" << payload["delivery_id"].As<int64_t>(0)
                       << " sender_id=" << payload["sender_id"].As<int64_t>(0)
                       << " recipient_id=" << payload["recipient_id"].As<int64_t>(0);
        }
        else if (event_type == "delivery.status_changed")
        {
            const auto& payload = json["payload"];
            LOG_INFO() << "NotificationConsumer: delivery status changed"
                       << " delivery_id=" << payload["delivery_id"].As<int64_t>(0)
                       << " new_status=" << payload["new_status"].As<std::string>("<unknown>");
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING() << "NotificationConsumer: failed to process message: " << e.what()
                      << " raw=" << message;
    }
}

}
