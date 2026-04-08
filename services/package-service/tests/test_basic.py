import pytest


def make_package(user_id: int) -> dict:
    """Generate a package payload for the given user."""
    return {
        'user_id': user_id,
        'weight': 1.5,
        'length': 10.0,
        'width': 5.0,
        'height': 3.0,
        'description': 'Test package',
    }


def unique_user_id(request) -> int:
    """Derive a stable unique user_id from the test name."""
    return abs(hash(request.node.name)) % 10**8


@pytest.fixture
def user_id(request) -> int:
    return unique_user_id(request)


@pytest.fixture
def package(user_id) -> dict:
    return make_package(user_id)


# ── CREATE PACKAGE ─────────────────────────────────────────────────────────────

async def test_create_package_success(service_client, package):
    response = await service_client.post('/v1/packages', json=package)
    assert response.status == 201
    body = response.json()
    assert 'id' in body
    assert body['message'] == 'Package created successfully'


async def test_create_package_returns_id(service_client, package):
    response = await service_client.post('/v1/packages', json=package)
    assert response.status == 201
    assert isinstance(response.json()['id'], int)


async def test_create_multiple_packages_for_same_user(service_client, user_id):
    pkg = make_package(user_id)
    r1 = await service_client.post('/v1/packages', json=pkg)
    r2 = await service_client.post('/v1/packages', json=pkg)
    assert r1.status == 201
    assert r2.status == 201
    assert r1.json()['id'] != r2.json()['id']


async def test_create_package_missing_field(service_client, user_id):
    pkg = make_package(user_id)
    del pkg['weight']
    response = await service_client.post('/v1/packages', json=pkg)
    assert response.status == 400


# ── GET PACKAGES ───────────────────────────────────────────────────────────────

async def test_get_packages_success(service_client, package, user_id):
    await service_client.post('/v1/packages', json=package)

    response = await service_client.get('/v1/packages', params={'user_id': user_id})
    assert response.status == 200
    body = response.json()
    assert 'packages' in body
    assert len(body['packages']) >= 1


async def test_get_packages_returns_correct_fields(service_client, package, user_id):
    await service_client.post('/v1/packages', json=package)

    response = await service_client.get('/v1/packages', params={'user_id': user_id})
    assert response.status == 200
    pkg = response.json()['packages'][0]
    assert pkg['user_id'] == user_id
    assert pkg['weight'] == package['weight']
    assert pkg['length'] == package['length']
    assert pkg['width'] == package['width']
    assert pkg['height'] == package['height']
    assert pkg['description'] == package['description']
    assert 'id' in pkg
    assert 'created_at' in pkg


async def test_get_packages_empty_for_unknown_user(service_client):
    response = await service_client.get('/v1/packages', params={'user_id': 999999999})
    assert response.status == 200
    assert response.json()['packages'] == []


# ── MISSING / INVALID PARAMS ───────────────────────────────────────────────────

async def test_get_packages_no_user_id(service_client):
    response = await service_client.get('/v1/packages')
    assert response.status == 400


async def test_get_packages_invalid_user_id(service_client):
    response = await service_client.get('/v1/packages', params={'user_id': 'not_a_number'})
    assert response.status == 400