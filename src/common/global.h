#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

namespace LanChat {

// Error codes
enum class ErrorCode {
    Success = 0,
    NetworkError = 1001,
    AuthFailed = 1002,
    DatabaseError = 2001,
    InvalidMessage = 3001
};

// Application constants
namespace Constants {
    constexpr int DEFAULT_PORT = 8888;
    constexpr int MAX_MESSAGE_LENGTH = 4096;
    constexpr const char* APP_VERSION = "1.0.0";
    constexpr const char* DB_NAME = "lanchat.db";
}

} // namespace LanChat

#endif // GLOBAL_H
