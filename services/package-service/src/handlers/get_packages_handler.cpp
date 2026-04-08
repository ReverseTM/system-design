#include "get_packages_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <schemas/package.hpp>

namespace handlers {
    GetPackagesHandler::GetPackagesHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::PackageUseCase>())
    {

    }

    userver::formats::json::Value GetPackagesHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value&,
        userver::server::request::RequestContext&
    ) const
    {
        if (request.HasArg("user_id"))
        {
            int64_t user_id;
            try {
                user_id = std::stoll(request.GetArg("user_id"));
            } catch (const std::exception&) {
                throw userver::server::handlers::ClientError(
                    userver::server::handlers::ExternalBody{"'user_id' must be a valid integer"}
                );
            }
            auto body = usecase_.GetPackages(user_id);
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }

        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"'user_id' query parameter is required"}
        );
    }
}
