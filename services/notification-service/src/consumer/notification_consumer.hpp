#pragma once

#include <userver/components/component_base.hpp>
#include <userver/urabbitmq/consumer_component_base.hpp>

namespace consumer {

class NotificationConsumer final : public userver::urabbitmq::ConsumerComponentBase
{
public:
    static constexpr std::string_view kName = "consumer-notification";

    NotificationConsumer(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

protected:
    void Process(std::string message) override;
};

}
