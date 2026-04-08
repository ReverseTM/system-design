import pytest


def make_user(suffix: str) -> dict:
    return {
        'login': f'user_{suffix}',
        'password': 'secret123',
        'email': f'user_{suffix}@example.com',
        'first_name': 'Test',
        'last_name': f'User_{suffix}',
    }


@pytest.fixture
def user(request):
    suffix = request.node.name.replace('[', '_').replace(']', '')
    return make_user(suffix)


@pytest.fixture
def mock_user_service_created(mockserver):
    """Mock user-service to accept POST /users and return 201."""
    @mockserver.handler('/users')
    def handle(request):
        return mockserver.make_response(
            status=201,
            json={'id': 1, 'message': 'User created successfully'},
        )
    return handle


@pytest.fixture
async def registered_user(service_client, mock_user_service_created, user):
    """Register a user and return its credentials."""
    resp = await service_client.post('/v1/auth/register', json=user)
    assert resp.status == 201
    return user


@pytest.fixture
async def auth_token(service_client, registered_user):
    """Log in as registered_user and return the JWT token."""
    resp = await service_client.post('/v1/auth/login', json={
        'login': registered_user['login'],
        'password': registered_user['password'],
    })
    assert resp.status == 200
    return resp.json()['token']


# ── REGISTER ───────────────────────────────────────────────────────────────────

async def test_register_success(service_client, mock_user_service_created, user):
    response = await service_client.post('/v1/auth/register', json=user)
    assert response.status == 201
    body = response.json()
    assert body['message'] == 'User registered successfully'
    assert isinstance(body['id'], int)
    assert body['id'] > 0


async def test_register_duplicate_login(service_client, mock_user_service_created, user):
    await service_client.post('/v1/auth/register', json=user)
    # Second attempt with same login — storage rejects before calling user-service
    response = await service_client.post('/v1/auth/register', json=user)
    assert response.status == 409


async def test_register_missing_password(service_client, user):
    data = {k: v for k, v in user.items() if k != 'password'}
    response = await service_client.post('/v1/auth/register', json=data)
    assert response.status == 400


async def test_register_missing_email(service_client, user):
    data = {k: v for k, v in user.items() if k != 'email'}
    response = await service_client.post('/v1/auth/register', json=data)
    assert response.status == 400


async def test_register_missing_first_name(service_client, user):
    data = {k: v for k, v in user.items() if k != 'first_name'}
    response = await service_client.post('/v1/auth/register', json=data)
    assert response.status == 400


async def test_register_user_service_error(service_client, mockserver, user):
    @mockserver.handler('/users')
    def handle(request):
        return mockserver.make_response(status=500)

    response = await service_client.post('/v1/auth/register', json=user)
    assert response.status == 500


# ── LOGIN ──────────────────────────────────────────────────────────────────────

async def test_login_success(service_client, registered_user):
    response = await service_client.post('/v1/auth/login', json={
        'login': registered_user['login'],
        'password': registered_user['password'],
    })
    assert response.status == 200
    body = response.json()
    assert 'token' in body
    assert len(body['token']) > 10


async def test_login_wrong_password(service_client, registered_user):
    response = await service_client.post('/v1/auth/login', json={
        'login': registered_user['login'],
        'password': 'wrong_password',
    })
    assert response.status == 401


async def test_login_unknown_user(service_client):
    response = await service_client.post('/v1/auth/login', json={
        'login': 'nobody_xyz_123',
        'password': 'irrelevant',
    })
    assert response.status == 401


async def test_login_missing_password(service_client, registered_user):
    response = await service_client.post('/v1/auth/login', json={
        'login': registered_user['login'],
    })
    assert response.status == 400


async def test_login_missing_login(service_client):
    response = await service_client.post('/v1/auth/login', json={
        'password': 'secret',
    })
    assert response.status == 400


# ── VALIDATE ───────────────────────────────────────────────────────────────────

async def test_validate_success(service_client, auth_token, registered_user):
    response = await service_client.get(
        '/v1/auth/validate',
        headers={'Authorization': f'Bearer {auth_token}'},
    )
    assert response.status == 200
    body = response.json()
    assert body['login'] == registered_user['login']
    assert 'issued_at' in body
    assert 'expires_at' in body
    assert body['expires_at'] > body['issued_at']


async def test_validate_missing_header(service_client):
    response = await service_client.get('/v1/auth/validate')
    assert response.status == 401


async def test_validate_invalid_token(service_client):
    response = await service_client.get(
        '/v1/auth/validate',
        headers={'Authorization': 'Bearer this.is.not.a.valid.jwt'},
    )
    assert response.status == 401


async def test_validate_wrong_scheme(service_client):
    response = await service_client.get(
        '/v1/auth/validate',
        headers={'Authorization': 'Basic dXNlcjpwYXNz'},
    )
    assert response.status == 401
