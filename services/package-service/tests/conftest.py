import os
import pathlib
import yaml
import pytest

from testsuite.databases.pgsql import discover

pytest_plugins = [
    'pytest_userver.plugins.core',
    'testsuite.databases.pgsql.pytest_plugin',
]

_SCHEMA_FILE = pathlib.Path(__file__).parent / 'sql' / 'schema.sql'


@pytest.fixture(scope='session')
def pgsql_local(pgsql_local_create):
    shard = discover.PgShard(
        shard_id=0,
        pretty_name='package_service',
        dbname='package_service',
        files=[_SCHEMA_FILE],
        migrations=[],
    )
    db = discover.PgShardedDatabase(
        service_name=None,
        dbname='package_service',
        shards=[shard],
    )
    return pgsql_local_create([db])


@pytest.fixture(scope='session')
def service_config_vars(pgsql_local):
    config_path = os.path.join(
        os.path.dirname(__file__), '..', 'configs', 'config_vars.testing.yaml'
    )
    with open(config_path) as f:
        vars = yaml.safe_load(f)
    vars['dbconnection'] = pgsql_local['package_service'].get_uri()
    return vars


@pytest.fixture(scope='session')
def service_env():
    return {'LSAN_OPTIONS': 'detect_leaks=0'}