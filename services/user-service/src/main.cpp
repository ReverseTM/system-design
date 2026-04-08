#include <userver/clients/dns/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include <handlers/create_user_handler.hpp>
#include <handlers/get_user_by_id_handler.hpp>
#include <handlers/get_user_handler.hpp>
#include <storage/user_storage.hpp>
#include <usecase/user_usecase.hpp>

int main(int argc, char* argv[])
{
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .Append<userver::components::TestsuiteSupport>()
            .Append<userver::clients::dns::Component>()
            .Append<userver::components::Postgres>("db-postgres")
            .Append<storage::UserStorage>()
            .Append<usecase::UserUseCase>()
            .Append<handlers::CreateUserHandler>()
            .Append<handlers::GetUserByIdHandler>()
            .Append<handlers::GetUserHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
