#include <spdlog/sinks/stdout_color_sinks.h>

#include "logging.h"

namespace pivot {

void configureLogger() {
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
  spdlog::set_level(spdlog::level::info);
  spdlog::stdout_color_mt("pivot");
}

} // namespace pivot
