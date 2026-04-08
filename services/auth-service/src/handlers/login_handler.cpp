#include "login_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>

#include <schemas/auth.hpp>

namespace handlers {
    LoginHandler::LoginHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::AuthUseCase>())
    {

    }

    userver::formats::json::Value LoginHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& json,
        userver::server::request::RequestContext&
    ) const
    {
        try 
        {
            auto body = usecase_.Login(json.As<auth::dto::LoginRequestBody>());
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        catch (const usecase::InvalidCredsError& e)
        {
            throw userver::server::handlers::Unauthorized(
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
