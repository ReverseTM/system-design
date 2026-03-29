# --- Этап 1: Сборка ---
FROM --platform=linux/amd64 ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS builder

WORKDIR /build

COPY CMakeLists.txt ./
COPY cmake/ ./cmake/
COPY schemas/ ./schemas/
COPY src/ ./src/

RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=OFF && \
    make -j$(nproc) auth_service

# --- Этап 2: Рантайм ---
FROM --platform=linux/amd64 ghcr.io/userver-framework/ubuntu-24.04-userver:latest

RUN useradd -m -s /bin/bash appuser

WORKDIR /app

COPY --from=builder /build/build/auth_service ./auth_service

RUN mkdir -p /app/configs /var/lib/auth-service
COPY configs/static_config.yaml ./configs/static_config.yaml
COPY configs/config_vars.yaml   ./configs/config_vars.yaml

RUN chown -R appuser:appuser /app /var/lib/auth-service

USER appuser

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=5 \
    CMD curl -f http://localhost:8080/ping || exit 1

CMD ["./auth_service", "-c", "./configs/static_config.yaml", "--config_vars", "./configs/config_vars.yaml"]