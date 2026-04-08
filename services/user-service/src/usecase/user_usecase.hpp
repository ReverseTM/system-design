#pragma once

#include <optional>
#include <string>

#include <userver/components/component_base.hpp>

#include <schemas/user.hpp>
#include <storage/user_storage.hpp>

namespace usecase {

    struct UserAlreadyExistsError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class UserUseCase final : public userver::components::ComponentBase
    {
    public:
        static constexpr std::string_view kName = "usecase-user";

        UserUseCase(
            const userver::components::ComponentConfig&,
            const userver::components::ComponentContext&
        );

        user::dto::CreateUserResponseBody CreateUser(user::dto::CreateUserRequestBody&&) const;
        std::optional<user::dto::User> GetUserById(int64_t id) const;
        std::optional<user::dto::GetUsersResponseBody> GetUserByLogin(const std::string& login) const;
        user::dto::GetUsersResponseBody GetUsersByMask(const std::string& mask) const;

    private:
        storage::UserStorage& storage_;
    };

}
