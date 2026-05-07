#include "package_usecase.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/redis/component.hpp>

namespace usecase {

    namespace {
        constexpr auto kPackagesByUserTtl = std::chrono::seconds{120};
    }

    PackageUseCase::PackageUseCase(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        storage_(context.FindComponent<storage::PackageStorage>()),
        redis_client_(
            context.FindComponent<userver::components::Redis>("redis-cache")
                   .GetClient("cache"))
    {

    }

    package::dto::CreatePackageResponseBody PackageUseCase::CreatePackage(
        package::dto::CreatePackageRequestBody&& request
    ) const
    {
        const auto id = storage_.CreatePackage(request);

        try
        {
            redis_client_->Del(
                {"packages:user:" + std::to_string(request.user_id)}, {}
            ).Get();
        }
        catch (const std::exception&) {}

        return package::dto::CreatePackageResponseBody{
            .id      = id,
            .message = "Package created successfully",
        };
    }

    package::dto::GetPackagesResponseBody PackageUseCase::GetPackages(int64_t user_id) const
    {
        const auto cache_key = "packages:user:" + std::to_string(user_id);

        try
        {
            auto cached = redis_client_->Get(cache_key, {}).Get();
            if (cached)
            {
                auto json = userver::formats::json::FromString(*cached);
                return json.As<package::dto::GetPackagesResponseBody>();
            }
        }
        catch (const std::exception&)
        {
            // Redis недоступен — идём в MongoDB
        }

        auto result = package::dto::GetPackagesResponseBody{
            .packages = storage_.GetPackagesByUser(user_id),
        };

        try
        {
            auto json_str = userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{result}.ExtractValue()
            );
            redis_client_->Set(cache_key, json_str, kPackagesByUserTtl, {}).Get();
        }
        catch (const std::exception&) {}

        return result;
    }
}
