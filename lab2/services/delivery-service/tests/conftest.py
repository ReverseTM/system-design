import os
import yaml
import pytest


pytest_plugins = [
    'pytest_userver.plugins.core',
    'testsuite.mockserver',
]


@pytest.fixture(scope='session')
def service_config_vars(mockserver_info):
    config_path = os.path.join(
        os.path.dirname(__file__), '..', 'configs', 'config_vars.testing.yaml'
    )
    with open(config_path) as f:
        vars = yaml.safe_load(f)
    vars['user-service-url'] = mockserver_info.base_url.rstrip('/')
    return vars


@pytest.fixture(scope='session', autouse=True)
def cleanup_test_db():
    db_path = '/tmp/delivery-service-test.db'
    if os.path.exists(db_path):
        os.remove(db_path)


@pytest.fixture(scope='session')
def service_env():
    return {'LSAN_OPTIONS': 'detect_leaks=0'}
