#include "get_user_by_id_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <schemas/user.hpp>

namespace handlers {

    GetUserByIdHandler::GetUserByIdHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context),
        usecase_(context.FindComponent<usecase::UserUseCase>())
    {

    }

    userver::formats::json::Value GetUserByIdHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value&,
        userver::server::request::RequestContext&
    ) const
    {
        const auto& id_str = request.GetPathArg("id");

        int64_t id = 0;
        try
        {
            id = std::stoll(id_str);
        }
        catch (const std::exception&)
        {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"Invalid user id"}
            );
        }

        auto user = usecase_.GetUserById(id);
        if (!user)
        {
            throw userver::server::handlers::ResourceNotFound(
                userver::server::handlers::ExternalBody{"User not found"}
            );
        }

        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ValueBuilder{*user}.ExtractValue();
    }

}
