#pragma once

#include <memory>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/urabbitmq/client.hpp>
#include <userver/urabbitmq/component.hpp>

namespace producer {

class DeliveryProducer final : public userver::components::ComponentBase
{
public:
    static constexpr std::string_view kName = "producer-delivery";

    DeliveryProducer(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    void PublishDeliveryCreated(int64_t delivery_id, int64_t sender_id, int64_t recipient_id) const;

private:
    std::shared_ptr<userver::urabbitmq::Client> client_;
};

}
