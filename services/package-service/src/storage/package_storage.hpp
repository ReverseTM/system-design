#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <userver/components/component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include <schemas/package.hpp>

namespace storage {
    class PackageStorage final : public userver::components::ComponentBase
    {
        public:
            static constexpr std::string_view kName = "storage-package";

            PackageStorage(
                const userver::components::ComponentConfig&,
                const userver::components::ComponentContext&
            );

            int64_t CreatePackage(const package::dto::CreatePackageRequestBody& request) const;
            std::vector<package::dto::Package> GetPackagesByUser(int64_t user_id) const;

        private:
            userver::storages::postgres::ClusterPtr cluster_;
    };
}
