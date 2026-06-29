#include <string>
#include <cstdio>
#include <vector>
namespace exd::app::services {
struct Notification { std::string title, message; int duration_ms = 3000; };
static std::vector<Notification> queue;
void notify(const std::string& title, const std::string& msg, int duration) {
    std::fprintf(stdout, "[NOTIFY] %s: %s\n", title.c_str(), msg.c_str());
    queue.push_back({title, msg, duration});
}
}
