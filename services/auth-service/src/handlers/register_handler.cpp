#include "register_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>

#include <schemas/auth.hpp>

namespace handlers {
    RegisterHandler::RegisterHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::AuthUseCase>())
    {

    }

    userver::formats::json::Value RegisterHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& json,
        userver::server::request::RequestContext&
    ) const
    {
        try 
        {
            auto body = usecase_.Register(json.As<auth::dto::RegisterRequestBody>());
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        catch (const usecase::UserExistsError& e)
        {
            throw userver::server::handlers::ConflictError(
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
