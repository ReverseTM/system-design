#pragma once

#include <userver/components/component_fwd.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <usecase/user_usecase.hpp>

namespace handlers {

    class GetUserByIdHandler final : public userver::server::handlers::HttpHandlerJsonBase
    {
    public:
        static constexpr std::string_view kName = "handler-get-user-by-id";

        GetUserByIdHandler(
            const userver::components::ComponentConfig&,
            const userver::components::ComponentContext&
        );

        userver::formats::json::Value HandleRequestJsonThrow(
            const userver::server::http::HttpRequest&,
            const userver::formats::json::Value&,
            userver::server::request::RequestContext&
        ) const override;

    private:
        usecase::UserUseCase& usecase_;
    };

}
