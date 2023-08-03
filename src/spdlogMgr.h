#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>

class spdlogManagement
{
public:
	static const auto getFileLogHandle()
	{
		if (file_handle == nullptr) {
			file_handle = spdlog::stdout_color_mt("file_log");
		}
		return file_handle;
	}
	static const auto getConsoleLogHandle()
	{
		if (console_handle == nullptr) {
			console_handle = spdlog::stdout_color_mt("console_log");
		}
		return console_handle;
	}
private:
	static std::shared_ptr<spdlog::logger> file_handle;
	static std::shared_ptr<spdlog::logger> console_handle;
};

