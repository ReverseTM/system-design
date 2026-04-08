#pragma once

#include <string>

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component_base.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include <storage/credential_storage.hpp>
#include <schemas/auth.hpp>

namespace usecase {
    struct ValidationError   : std::runtime_error { using std::runtime_error::runtime_error; };
    struct UserExistsError   : std::runtime_error { using std::runtime_error::runtime_error; };
    struct InvalidCredsError : std::runtime_error { using std::runtime_error::runtime_error; };
    struct InvalidTokenError : std::runtime_error { using std::runtime_error::runtime_error; };

    class AuthUseCase final : public userver::components::ComponentBase
    {
        public:
            static constexpr std::string_view kName = "usecase-auth";

            AuthUseCase(
                const userver::components::ComponentConfig&,
                const userver::components::ComponentContext&
            );

            static userver::yaml_config::Schema GetStaticConfigSchema();

            auth::dto::RegisterResponseBody Register(auth::dto::RegisterRequestBody&&);
            auth::dto::LoginResponseBody    Login(auth::dto::LoginRequestBody&&) const;
            auth::dto::ValidateResponseBody Validate(const std::string&) const;

        private:
            storage::CredentialStorage& storage_;
            userver::clients::http::Client& http_client_;
            std::string jwt_secret_;
            std::string user_service_url_;
    };
}
