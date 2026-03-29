#include "create_package_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/server/handlers/exceptions.hpp>

#include <usecase/package_usecase.hpp>
#include <schemas/package.hpp>

namespace handlers {
    CreatePackageHandler::CreatePackageHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context), usecase_(context.FindComponent<usecase::PackageUseCase>())
    {
        
    }

    userver::formats::json::Value CreatePackageHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& json,
        userver::server::request::RequestContext&
    ) const
    {
        try
        {
            auto body = usecase_.CreatePackage(json.As<package::dto::CreatePackageRequestBody>());
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ValueBuilder{body}.ExtractValue();
        }
        catch (const userver::formats::json::Exception& e)
        {
            throw userver::server::handlers::RequestParseError(
                userver::server::handlers::ExternalBody{e.what()}
            );
        }
    }
}
