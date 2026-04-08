#include "get_user_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <schemas/user.hpp>
#include <usecase/user_usecase.hpp>

namespace handlers {

    GetUserHandler::GetUserHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context),
        usecase_(context.FindComponent<usecase::UserUseCase>())
    {

	}

    userver::formats::json::Value GetUserHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value&,
        userver::server::request::RequestContext&
    ) const
    {
        if (request.HasArg("login"))
        {
            auto body = usecase_.GetUserByLogin(request.GetArg("login"));
            if (!body)
            {
                throw userver::server::handlers::ResourceNotFound(
                    userver::server::handlers::ExternalBody{"User not found"}
                );
            }

			request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
			return userver::formats::json::ValueBuilder{*body}.ExtractValue();
        }
        else if (request.HasArg("mask"))
        {
            auto body = usecase_.GetUsersByMask(request.GetArg("mask"));
			request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
			return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }

        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{
                "Either 'login' or 'mask' query parameter is required"
            }
        );
    }
}
