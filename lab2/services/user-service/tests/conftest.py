import os
import pytest


pytest_plugins = [
    'pytest_userver.plugins.core',
]


@pytest.fixture(scope='session', autouse=True)
def cleanup_test_db():
    db_path = '/home/user/user_service/build-debug/users_test.db'
    if os.path.exists(db_path):
        os.remove(db_path)


@pytest.fixture(scope='session')
def service_env():
    return {'LSAN_OPTIONS': 'detect_leaks=0'}





