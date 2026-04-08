import uuid
import pytest


def make_user(suffix: str) -> dict:
    """Generate a unique user to avoid conflicts between tests."""
    h = abs(hash(suffix)) % 10**9
    return {
        'user_id': h,
        'login': f'u{h}',
        'email': f'u{h}@example.com',
        'first_name': 'Test',
        'last_name': f'User {suffix[:20]}',
    }


@pytest.fixture
def user(request):
    """Unique user data per test based on test name."""
    suffix = request.node.name.replace('[', '_').replace(']', '')
    return make_user(suffix)


@pytest.fixture
def user2(request):
    suffix = request.node.name.replace('[', '_').replace(']', '') + '_2'
    return make_user(suffix)


# ── CREATE USER ────────────────────────────────────────────────────────────────

async def test_create_user_success(service_client, user):
    response = await service_client.post('/v1/users', json=user)
    assert response.status == 201
    assert response.json()['message'] == 'User created successfully'


async def test_create_user_duplicate_login(service_client, user):
    await service_client.post('/v1/users', json=user)

    # Different user_id and email, same login — conflict on login uniqueness
    duplicate = {**user, 'user_id': user['user_id'] + 1, 'email': 'x' + user['email']}
    response = await service_client.post('/v1/users', json=duplicate)
    assert response.status == 409


async def test_create_user_duplicate_email(service_client, user):
    await service_client.post('/v1/users', json=user)

    # Different user_id and login, same email — conflict on email uniqueness
    duplicate = {**user, 'user_id': user['user_id'] + 1, 'login': 'x' + user['login']}
    response = await service_client.post('/v1/users', json=duplicate)
    assert response.status == 409


async def test_create_user_duplicate_id(service_client, user, user2):
    await service_client.post('/v1/users', json=user)

    response = await service_client.post('/v1/users', json={**user2, 'user_id': user['user_id']})
    assert response.status == 409


# ── GET USER BY LOGIN ──────────────────────────────────────────────────────────

async def test_get_user_by_login_success(service_client, user):
    await service_client.post('/v1/users', json=user)

    response = await service_client.get('/v1/users', params={'login': user['login']})
    assert response.status == 200
    body = response.json()
    assert len(body['users']) == 1
    result = body['users'][0]
    assert result['login'] == user['login']
    assert result['email'] == user['email']
    assert result['first_name'] == user['first_name']
    assert result['last_name'] == user['last_name']
    assert 'created_at' in result


async def test_get_user_by_login_not_found(service_client):
    response = await service_client.get('/v1/users', params={'login': 'definitely_nonexistent_xyz'})
    assert response.status == 404


# ── GET USERS BY MASK ──────────────────────────────────────────────────────────

async def test_get_users_by_mask_success(service_client, user):
    await service_client.post('/v1/users', json=user)

    response = await service_client.get('/v1/users', params={'mask': user['first_name']})
    assert response.status == 200
    logins = [u['login'] for u in response.json()['users']]
    assert user['login'] in logins


async def test_get_users_by_mask_no_results(service_client):
    response = await service_client.get('/v1/users', params={'mask': 'NoSuchNameXYZ99999'})
    assert response.status == 200
    assert response.json()['users'] == []


# ── GET USER BY ID ────────────────────────────────────────────────────────────

async def test_get_user_by_id_success(service_client, user):
    await service_client.post('/v1/users', json=user)

    response = await service_client.get(f'/v1/users/{user["user_id"]}')
    assert response.status == 200
    result = response.json()
    assert result['user_id'] == user['user_id']
    assert result['login'] == user['login']
    assert result['email'] == user['email']
    assert result['first_name'] == user['first_name']
    assert result['last_name'] == user['last_name']
    assert 'created_at' in result


async def test_get_user_by_id_not_found(service_client):
    response = await service_client.get('/v1/users/999999999')
    assert response.status == 404


async def test_get_user_by_id_invalid(service_client):
    response = await service_client.get('/v1/users/not_a_number')
    assert response.status == 400


# ── MISSING PARAMS ─────────────────────────────────────────────────────────────

async def test_get_users_no_params(service_client):
    response = await service_client.get('/v1/users')
    assert response.status == 400
