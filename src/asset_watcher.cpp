#include <string>
#include <functional>
#include <unordered_map>
namespace exd::app {
static std::unordered_map<std::string, std::function<void()>> watchers;
void watch_asset(const std::string& path, std::function<void()> cb) { watchers[path] = std::move(cb); }
}
