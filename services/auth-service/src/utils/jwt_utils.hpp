#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace auth {
    struct JwtClaims {
        std::string login;
        int64_t issued_at{};
        int64_t expires_at{};
    };

    std::string CreateJwt(
        const std::string&,
        const std::string&,
        int64_t = 3600
    );

    std::optional<JwtClaims> ValidateJwt(
        const std::string&,
        const std::string&
    );

    std::string HashPassword(const std::string&);
}
