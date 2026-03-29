#include "create_user_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_status.hpp>

#include <schemas/user.hpp>
#include <usecase/user_usecase.hpp>

namespace handlers {

    CreateUserHandler::CreateUserHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context),
        usecase_(context.FindComponent<usecase::UserUseCase>())
    {

	}

    userver::formats::json::Value CreateUserHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& json,
        userver::server::request::RequestContext&
    ) const
    {
        try
        {
			auto body = usecase_.CreateUser(json.As<user::dto::CreateUserRequestBody>());
			request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        catch (const usecase::UserAlreadyExistsError& e)
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
