#include <userver/clients/dns/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/postgres/component.hpp>
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
            .Append<userver::components::Postgres>("db-postgres")
            .Append<storage::PackageStorage>()
            .Append<usecase::PackageUseCase>()
            .Append<handlers::CreatePackageHandler>()
            .Append<handlers::GetPackagesHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
