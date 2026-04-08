#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include <handlers/create_delivery_handler.hpp>
#include <handlers/get_deliveries_handler.hpp>
#include <storage/delivery_storage.hpp>
#include <usecase/delivery_usecase.hpp>

int main(int argc, char* argv[]) {
    const auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .Append<userver::components::TestsuiteSupport>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::components::Postgres>("db-postgres")
            .Append<storage::DeliveryStorage>()
            .Append<usecase::DeliveryUseCase>()
            .Append<handlers::CreateDeliveryHandler>()
            .Append<handlers::GetDeliveriesByUserHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
