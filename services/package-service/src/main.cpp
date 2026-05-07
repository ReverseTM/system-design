#include <userver/clients/dns/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/redis/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include <handlers/create_package_handler.hpp>
#include <handlers/get_packages_handler.hpp>
#include <storage/package_storage.hpp>
#include <usecase/package_usecase.hpp>

int main(int argc, char* argv[]) {
    const auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .Append<userver::components::TestsuiteSupport>()
            .Append<userver::clients::dns::Component>()
            .Append<userver::components::Secdist>()
            .Append<userver::components::DefaultSecdistProvider>()
            .Append<userver::components::Redis>("redis-cache")
            .Append<userver::components::Mongo>("mongo-package")
            .Append<storage::PackageStorage>()
            .Append<usecase::PackageUseCase>()
            .Append<handlers::CreatePackageHandler>()
            .Append<handlers::GetPackagesHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}