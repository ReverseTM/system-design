SERVICES := auth-service user-service package-service delivery-service

docker/build: $(addprefix docker/build/, $(SERVICES))

docker/build/%:
	docker build -t $*:latest ./services/$* --no-cache

services/start:
	docker compose up -d

services/stop:
	docker compose down

db/fill: $(addprefix db/fill/, $(SERVICES))

db/fill/%:
	docker exec -i $*-db psql -U postgres -d $* < ./postgres/$*/data.sql
