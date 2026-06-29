#include <string>
#include <fstream>
namespace exd::app {
struct Project { std::string name, path; bool dirty = false; };
void project_save(const std::string& path) { std::ofstream f(path); if (f) f << "# project\n"; }
void project_load(const std::string& path) { std::ifstream f(path); }
}
