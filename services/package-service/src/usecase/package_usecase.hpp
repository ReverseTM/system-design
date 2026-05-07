#pragma once

#include <string>

#include <userver/components/component_base.hpp>
#include <userver/storages/redis/client.hpp>

#include <storage/package_storage.hpp>
#include <schemas/package.hpp>

namespace usecase {
    class PackageUseCase final : public userver::components::ComponentBase
    {
        public:
            static constexpr std::string_view kName = "usecase-package";

            PackageUseCase(
                const userver::components::ComponentConfig&,
                const userver::components::ComponentContext&
            );

            package::dto::CreatePackageResponseBody CreatePackage(package::dto::CreatePackageRequestBody&&) const;
            package::dto::GetPackagesResponseBody GetPackages(int64_t user_id) const;

        private:
            storage::PackageStorage& storage_;
            userver::storages::redis::ClientPtr redis_client_;
    };
}
