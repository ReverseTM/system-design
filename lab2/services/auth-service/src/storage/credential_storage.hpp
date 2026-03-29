#pragma once

#include <optional>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/storages/sqlite/client.hpp>

namespace storage {
    class CredentialStorage final : public userver::components::ComponentBase
    {
        public:
            static constexpr std::string_view kName = "storage-credential";

            CredentialStorage(
                const userver::components::ComponentConfig&,
                const userver::components::ComponentContext&
            );

            int64_t SaveCredentials(const std::string&, const std::string&);
            void DeleteCredentials(const std::string&);
            std::optional<std::string> GetPasswordHash(const std::string&) const;

        private:
            userver::storages::sqlite::ClientPtr client_;
    };
}
