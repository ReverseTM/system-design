#include "package_usecase.hpp"

#include <userver/components/component_context.hpp>

namespace usecase {
    PackageUseCase::PackageUseCase(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context), storage_(context.FindComponent<storage::PackageStorage>())
    {

    }

    package::dto::CreatePackageResponseBody PackageUseCase::CreatePackage(
        package::dto::CreatePackageRequestBody&& request
    ) const
    {
        const auto id = storage_.CreatePackage(request);

        return package::dto::CreatePackageResponseBody{
            .id      = id,
            .message = "Package created successfully",
        };
    }

    package::dto::GetPackagesResponseBody PackageUseCase::GetPackages(int64_t user_id) const
    {
        return package::dto::GetPackagesResponseBody{
            .packages = storage_.GetPackagesByUser(user_id),
        };
    }
}
