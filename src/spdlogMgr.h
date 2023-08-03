#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>

#define _COLOR_RED "\x1b[31m"
#define _COLOR_GREEN "\x1b[32m"
#define _COLOR_YELLOW "\x1b[33m"
#define _COLOR_BLUE "\x1b[34m"
#define _COLOR_MEGENTA "\x1b[35m"
#define _COLOR_CYAN "\x1b[36m"

#define _COLOR_DEFAULT "\x1b[0m"

class spdlogManagement
{
public:
	static const auto getFileLogHandle()
	{
		if (file_handle == nullptr) {
			file_handle = spdlog::stdout_color_mt("FILE");
		}
		return file_handle;
	}
	static const auto getConsoleLogHandle()
	{
		if (console_handle == nullptr) {
			console_handle = spdlog::stdout_color_mt("CONSOLE");
		}
		return console_handle;
	}
private:
	static std::shared_ptr<spdlog::logger> file_handle;
	static std::shared_ptr<spdlog::logger> console_handle;
};

