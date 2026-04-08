#include "user_usecase.hpp"

#include <userver/components/component_context.hpp>

namespace usecase {

    UserUseCase::UserUseCase(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context), storage_(context.FindComponent<storage::UserStorage>())
    {

	}

    user::dto::CreateUserResponseBody UserUseCase::CreateUser(
        user::dto::CreateUserRequestBody&& request) const
    {
        try
        {
            auto id = storage_.CreateUser(request);
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
        auto user = storage_.GetUserByLogin(login);
        if (!user)
        {
            return std::nullopt;
        }

        return user::dto::GetUsersResponseBody{.users = {*user}};
    }

    user::dto::GetUsersResponseBody UserUseCase::GetUsersByMask(const std::string& mask) const
    {
        return user::dto::GetUsersResponseBody{.users = storage_.GetUsersByMask(mask)};
    }
}
