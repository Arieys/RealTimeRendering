#include "spdlogMgr.h"

std::shared_ptr<spdlog::logger> spdlogManagement::file_handle = nullptr;
std::shared_ptr<spdlog::logger> spdlogManagement::console_handle = nullptr;