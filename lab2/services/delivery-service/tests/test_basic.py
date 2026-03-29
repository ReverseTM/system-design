import pytest


SENDER_ID = 1
RECIPIENT_ID = 2
PACKAGE_ID = 10


def make_delivery(**kwargs) -> dict:
    base = {
        'sender_id': SENDER_ID,
        'recipient_id': RECIPIENT_ID,
        'package_id': PACKAGE_ID,
        'address': 'Moscow, Lenina 1',
    }
    base.update(kwargs)
    return base


def _user_response(user_id: int) -> dict:
    return {
        'id': user_id,
        'login': f'user{user_id}',
        'email': f'user{user_id}@example.com',
        'first_name': 'Test',
        'last_name': 'User',
        'created_at': '2026-01-01T00:00:00Z',
    }


@pytest.fixture
def mock_users_exist(mockserver):
    """Mock user-service: both sender and recipient exist."""
    @mockserver.handler(f'/users/{SENDER_ID}')
    def handle_sender(request):
        return mockserver.make_response(status=200, json=_user_response(SENDER_ID))

    @mockserver.handler(f'/users/{RECIPIENT_ID}')
    def handle_recipient(request):
        return mockserver.make_response(status=200, json=_user_response(RECIPIENT_ID))


# ── CREATE DELIVERY ────────────────────────────────────────────────────────────

async def test_create_delivery_success(service_client, mock_users_exist):
    response = await service_client.post('/v1/deliveries', json=make_delivery())
    assert response.status == 201
    body = response.json()
    assert isinstance(body['id'], int)
    assert body['id'] > 0
    assert body['message'] == 'Delivery created successfully'


async def test_create_delivery_missing_address(service_client, mock_users_exist):
    response = await service_client.post(
        '/v1/deliveries',
        json=make_delivery(address=None),
    )
    assert response.status == 400


async def test_create_delivery_missing_sender_id(service_client):
    data = make_delivery()
    del data['sender_id']
    response = await service_client.post('/v1/deliveries', json=data)
    assert response.status == 400


async def test_create_delivery_missing_recipient_id(service_client):
    data = make_delivery()
    del data['recipient_id']
    response = await service_client.post('/v1/deliveries', json=data)
    assert response.status == 400


async def test_create_delivery_missing_package_id(service_client):
    data = make_delivery()
    del data['package_id']
    response = await service_client.post('/v1/deliveries', json=data)
    assert response.status == 400


# ── USER VALIDATION ────────────────────────────────────────────────────────────

async def test_create_delivery_sender_not_found(service_client, mockserver):
    @mockserver.handler(f'/users/{SENDER_ID}')
    def handle_sender(request):
        return mockserver.make_response(status=404)

    response = await service_client.post('/v1/deliveries', json=make_delivery())
    assert response.status == 400


async def test_create_delivery_recipient_not_found(service_client, mockserver):
    @mockserver.handler(f'/users/{SENDER_ID}')
    def handle_sender(request):
        return mockserver.make_response(status=200, json=_user_response(SENDER_ID))

    @mockserver.handler(f'/users/{RECIPIENT_ID}')
    def handle_recipient(request):
        return mockserver.make_response(status=404)

    response = await service_client.post('/v1/deliveries', json=make_delivery())
    assert response.status == 400


async def test_create_delivery_user_service_unavailable(service_client, mockserver):
    @mockserver.handler(f'/users/{SENDER_ID}')
    def handle_sender(request):
        return mockserver.make_response(status=500)

    response = await service_client.post('/v1/deliveries', json=make_delivery())
    assert response.status == 500
