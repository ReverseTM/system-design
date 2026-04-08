#include "validate_handler.hpp"

#include <userver/components/component_context.hpp>

#include <userver/formats/json/value_builder.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <schemas/auth.hpp>

namespace handlers {
    namespace {
        constexpr std::string_view kBearer = "Bearer ";
    }

    ValidateHandler::ValidateHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::AuthUseCase>())
    {

    }

    userver::formats::json::Value ValidateHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value&,
        userver::server::request::RequestContext&
    ) const
    {
        const auto& auth_header = request.GetHeader(userver::http::headers::kAuthorization);

        if (auth_header.empty())
        {
            throw userver::server::handlers::Unauthorized(
                userver::server::handlers::ExternalBody{"Missing 'Authorization' header"}
            );
        }

        if (!auth_header.starts_with(kBearer))
        {
            throw userver::server::handlers::Unauthorized(
                userver::server::handlers::ExternalBody{"Invalid authorization type, expected 'Bearer'"}
            );
        }

        const std::string token{auth_header.substr(kBearer.length())};

        try
        {
            auto body = usecase_.Validate(token);
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        catch (const usecase::InvalidTokenError& e)
        {
            throw userver::server::handlers::Unauthorized(
                userver::server::handlers::ExternalBody{e.what()}
            );
        }
    }
}
