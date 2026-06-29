#include <string>
#include <fstream>
namespace exd::app {
void save_layout(const std::string& path) { std::ofstream f(path); f << "# layout\n"; }
void load_layout(const std::string& path) { std::ifstream f(path); }
}
