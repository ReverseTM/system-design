#include "user_usecase.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/redis/component.hpp>

namespace usecase {

    namespace {
        constexpr auto kUserByLoginTtl = std::chrono::seconds{300};
    }

    UserUseCase::UserUseCase(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        storage_(context.FindComponent<storage::UserStorage>()),
        redis_client_(
            context.FindComponent<userver::components::Redis>("redis-cache")
                   .GetClient("cache"))
    {
	}

    user::dto::CreateUserResponseBody UserUseCase::CreateUser(
        user::dto::CreateUserRequestBody&& request) const
    {
        try
        {
            auto id = storage_.CreateUser(request);

            redis_client_->Del({"user:login:" + request.login}, {}).Get();

            return user::dto::CreateUserResponseBody{
                .id = id,
                .message = "User created successfully"
            };
        }
        catch (const storage::UserAlreadyExistsError& e)
        {
            throw UserAlreadyExistsError(e.what());
        }
    }

    std::optional<user::dto::User> UserUseCase::GetUserById(int64_t id) const
    {
        return storage_.GetUserById(id);
    }

    std::optional<user::dto::GetUsersResponseBody> UserUseCase::GetUserByLogin(const std::string& login) const
    {
        const auto cache_key = "user:login:" + login;

        try
        {
            auto cached = redis_client_->Get(cache_key, {}).Get();
            if (cached)
            {
                auto json = userver::formats::json::FromString(*cached);
                return json.As<user::dto::GetUsersResponseBody>();
            }
        }
        catch (const std::exception&)
        {
            // Redis недоступен — идём в БД
        }

        auto user = storage_.GetUserByLogin(login);
        if (!user)
        {
            return std::nullopt;
        }

        auto body = user::dto::GetUsersResponseBody{.users = {*user}};

        try
        {
            auto json_str = userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{body}.ExtractValue()
            );
            redis_client_->Set(cache_key, json_str, kUserByLoginTtl, {}).Get();
        }
        catch (const std::exception&) {}

        return body;
    }

    user::dto::GetUsersResponseBody UserUseCase::GetUsersByMask(const std::string& mask) const
    {
        return user::dto::GetUsersResponseBody{.users = storage_.GetUsersByMask(mask)};
    }
}
