#include "create_delivery_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <schemas/delivery.hpp>
#include <usecase/delivery_usecase.hpp>

namespace handlers {
    CreateDeliveryHandler::CreateDeliveryHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::DeliveryUseCase>())
    {

    }

    userver::formats::json::Value CreateDeliveryHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& json,
        userver::server::request::RequestContext&
    ) const
    {
        try
        {
            auto body = usecase_.CreateDelivery(json.As<delivery::dto::CreateDeliveryRequestBody>());
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        catch (const usecase::UserNotFoundError& e)
        {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{e.what()}
            );
        }
        catch (const userver::formats::json::Exception& e)
        {
            throw userver::server::handlers::RequestParseError(
                userver::server::handlers::ExternalBody{e.what()}
            );
        }
    }
}
