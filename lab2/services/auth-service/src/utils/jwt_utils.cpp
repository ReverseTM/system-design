#include "jwt_utils.hpp"

#include <jwt-cpp/jwt.h>

#include <userver/crypto/hash.hpp>

namespace auth {
    std::string CreateJwt(
        const std::string& login,
        const std::string& secret,
        int64_t expiry_seconds
    )
    {
        return jwt::create()
            .set_type("JWT")
            .set_payload_claim("sub", jwt::claim(login))
            .set_issued_now()
            .set_expires_in(std::chrono::seconds{expiry_seconds})
            .sign(jwt::algorithm::hs256{secret});
    }

    std::optional<JwtClaims> ValidateJwt(
        const std::string& token,
        const std::string& secret
    )
    {
        try {
            const auto decoded = jwt::decode(token);

            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secret})
                .with_type("JWT")
                .verify(decoded);

            const auto to_unix = [](const auto& tp) -> int64_t
            {
                return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
            };

            JwtClaims claims;
            claims.login = decoded.get_payload_claim("sub").as_string();
            claims.issued_at = to_unix(decoded.get_issued_at());
            claims.expires_at = to_unix(decoded.get_expires_at());
            
            return claims;
        } catch (...) {
            return std::nullopt;
        }
    }

    std::string HashPassword(const std::string& password) 
    {
        return userver::crypto::hash::Sha256(password);
    }
}
