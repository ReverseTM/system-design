#include "package_storage.hpp"

#include <chrono>

#include <userver/components/component_context.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/options.hpp>
#include <userver/utils/datetime.hpp>

namespace storage {
    PackageStorage::PackageStorage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    ) : ComponentBase(config, context),
        pool_(context.FindComponent<userver::components::Mongo>("mongo-package").GetPool())
    {

    }

    std::string PackageStorage::CreatePackage(const package::dto::CreatePackageRequestBody& request) const
    {
        auto collection = pool_->GetCollection("packages");

        userver::formats::bson::Oid oid;

        userver::formats::bson::ValueBuilder dim(userver::formats::bson::ValueBuilder::Type::kObject);
        dim["length"] = request.length;
        dim["width"]  = request.width;
        dim["height"] = request.height;

        userver::formats::bson::ValueBuilder doc(userver::formats::bson::ValueBuilder::Type::kObject);
        doc["_id"]         = oid;
        doc["user_id"]     = request.user_id;
        doc["weight"]      = request.weight;
        doc["dimensions"]  = dim.ExtractValue();
        doc["description"] = request.description;
        doc["created_at"]  = std::chrono::system_clock::now();

        collection.InsertOne(doc.ExtractValue());

        return oid.ToString();
    }

    std::vector<package::dto::Package> PackageStorage::GetPackagesByUser(int64_t user_id) const
    {
        auto collection = pool_->GetCollection("packages");

        auto cursor = collection.Find(
            userver::formats::bson::MakeDoc("user_id", user_id),
            userver::storages::mongo::options::Sort{
                {"created_at", userver::storages::mongo::options::Sort::kDescending}
            }
        );

        std::vector<package::dto::Package> packages;
        for (auto doc : cursor) {
            package::dto::Package pkg;
            pkg.id          = doc["_id"].As<userver::formats::bson::Oid>().ToString();
            pkg.user_id     = doc["user_id"].As<int64_t>();
            pkg.weight      = doc["weight"].As<double>();
            pkg.length      = doc["dimensions"]["length"].As<double>();
            pkg.width       = doc["dimensions"]["width"].As<double>();
            pkg.height      = doc["dimensions"]["height"].As<double>();
            pkg.description = doc["description"].As<std::string>();
            auto tp         = doc["created_at"].As<std::chrono::system_clock::time_point>();
            pkg.created_at  = userver::utils::datetime::Timestring(tp);
            packages.push_back(std::move(pkg));
        }

        return packages;
    }
}
