#include "login_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/redis/component.hpp>

#include <schemas/auth.hpp>

namespace handlers {

    namespace {
        constexpr int64_t kRateLimitMax = 10;
        constexpr int kWindowSeconds = 60;
    }

    LoginHandler::LoginHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : HttpHandlerJsonBase(config, context),
        usecase_(context.FindComponent<usecase::AuthUseCase>()),
        redis_client_(
            context.FindComponent<userver::components::Redis>("redis-cache")
                   .GetClient("cache"))
    {

    }

    userver::formats::json::Value LoginHandler::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& json,
        userver::server::request::RequestContext&
    ) const
    {
        auto client_ip = request.GetHeader("X-Real-IP");
        if (client_ip.empty())
        {
            client_ip = "unknown";
        }

        auto& resp = request.GetHttpResponse();

        try
        {
            const auto rate_key = "rate_limit:login:" + client_ip;
            auto count = redis_client_->Incr(rate_key, {}).Get();
            if (count == 1)
            {
                redis_client_->Expire(
                    rate_key, std::chrono::seconds{kWindowSeconds}, {}
                ).Get();
            }

            const auto remaining = std::max(int64_t{0}, kRateLimitMax - count);
            resp.SetHeader(std::string_view{"X-RateLimit-Limit"}, std::to_string(kRateLimitMax));
            resp.SetHeader(std::string_view{"X-RateLimit-Remaining"}, std::to_string(remaining));
            resp.SetHeader(std::string_view{"X-RateLimit-Reset"}, std::to_string(kWindowSeconds));

            if (count > kRateLimitMax)
            {
                resp.SetStatus(userver::server::http::HttpStatus::kTooManyRequests);
                return userver::formats::json::MakeObject(
                    "code", 429,
                    "message", "Too many requests. Please try again later."
                );
            }
        }
        catch (const std::exception&)
        {
            // Redis недоступен — пропускаем rate limiting
        }

        try
        {
            auto body = usecase_.Login(json.As<auth::dto::LoginRequestBody>());
            resp.SetStatus(userver::server::http::HttpStatus::kOk);
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
