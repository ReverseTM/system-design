#pragma once

#include <optional>
#include <string>
#include <vector>

#include <userver/components/component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include <schemas/user.hpp>

namespace storage {

    struct UserAlreadyExistsError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class UserStorage final : public userver::components::ComponentBase
    {
    public:
        static constexpr std::string_view kName = "storage-user";

        UserStorage(
            const userver::components::ComponentConfig&,
            const userver::components::ComponentContext&
        );

        int64_t CreateUser(const user::dto::CreateUserRequestBody& request) const;
        std::optional<user::dto::User> GetUserById(int64_t id) const;
        std::optional<user::dto::User> GetUserByLogin(const std::string& login) const;
        std::vector<user::dto::User> GetUsersByMask(const std::string& mask) const;

    private:
        userver::storages::postgres::ClusterPtr cluster_;
    };

}
