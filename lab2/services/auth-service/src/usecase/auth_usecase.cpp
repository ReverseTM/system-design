#include "auth_usecase.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../utils/jwt_utils.hpp"

namespace usecase {
    userver::yaml_config::Schema AuthUseCase::GetStaticConfigSchema()
    {
        return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
            R"(
                type: object
                description: Auth use case component
                additionalProperties: false
                properties:
                    jwt-secret:
                        type: string
                        description: Secret key for signing JWT tokens
                    user-service-url:
                        type: string
                        description: URL of the user-service POST /users endpoint
            )"
        );
    }

    AuthUseCase::AuthUseCase(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        storage_(context.FindComponent<storage::CredentialStorage>()),
        http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
        jwt_secret_(config["jwt-secret"].As<std::string>()),
        user_service_url_(config["user-service-url"].As<std::string>())
    {

    }

    auth::dto::RegisterResponseBody AuthUseCase::Register(auth::dto::RegisterRequestBody&& request)
    {
        const int64_t id = storage_.SaveCredentials(request.login, auth::HashPassword(request.password));
        if (id == 0)
        {
            throw UserExistsError{"User already exists"};
        }

        userver::formats::json::ValueBuilder builder;
        builder["id"]         = id;
        builder["login"]      = request.login;
        builder["email"]      = request.email;
        builder["first_name"] = request.first_name;
        builder["last_name"]  = request.last_name;
        const std::string body = userver::formats::json::ToString(builder.ExtractValue());

        try
        {
            const auto response = http_client_.CreateRequest()
                .post(user_service_url_, body)
                .timeout(std::chrono::seconds(5))
                .perform();

            if (response->status_code() != 201)
            {
                storage_.DeleteCredentials(request.login);
                throw std::runtime_error{
                    "user-service returned unexpected status: "
                    + std::to_string(response->status_code())
                };
            }
        }
        catch (const userver::clients::http::HttpClientException& e)
        {
            storage_.DeleteCredentials(request.login);
            throw std::runtime_error{
                std::string{"Failed to reach user-service: "} + e.what()
            };
        }

        return auth::dto::RegisterResponseBody{.id = id, .message = "User registered successfully"};
    }

    auth::dto::LoginResponseBody AuthUseCase::Login(auth::dto::LoginRequestBody&& request) const
    {
        const auto stored_hash = storage_.GetPasswordHash(request.login);
        if (!stored_hash || *stored_hash != auth::HashPassword(request.password))
        {
            throw InvalidCredsError{"Invalid credentials"};
        }

        return auth::dto::LoginResponseBody{.token = auth::CreateJwt(request.login, jwt_secret_)};
    }

    auth::dto::ValidateResponseBody AuthUseCase::Validate(const std::string& token) const
    {
        const auto claims = auth::ValidateJwt(token, jwt_secret_);
        if (!claims)
        {
            throw InvalidTokenError{"Invalid or expired token"};
        }

        return auth::dto::ValidateResponseBody{
            .login   = claims->login,
            .issued_at  = static_cast<int>(claims->issued_at),
            .expires_at = static_cast<int>(claims->expires_at),
        };
    }
}