#include "get_deliveries_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <schemas/delivery.hpp>
#include <usecase/delivery_usecase.hpp>

namespace handlers
{
    GetDeliveriesByUserHandler::GetDeliveriesByUserHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::DeliveryUseCase>())
    {

    }

    userver::formats::json::Value GetDeliveriesByUserHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value&,
        userver::server::request::RequestContext&
    ) const
    {
        auto parse_id = [](const std::string& s) -> int64_t {
            try
            {
                return std::stoll(s);
            }
            catch (const std::exception&) {
                throw userver::server::handlers::ClientError(
                    userver::server::handlers::ExternalBody{"Invalid id: not a number"}
                );
            }
        };

        if (request.HasArg("sender_id"))
        {
            auto body = usecase_.GetDeliveriesBySenderId(parse_id(request.GetArg("sender_id")));
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        else if (request.HasArg("recipient_id"))
        {
            auto body = usecase_.GetDeliveriesByRecipientId(parse_id(request.GetArg("recipient_id")));
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }

        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{
                "Either 'sender_id' or 'recipient_id' query parameter is required"
            }
        );
    }
}