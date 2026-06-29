#include <string>
namespace exd::app {
enum class BuildProfile { Debug, Release };
static BuildProfile current = BuildProfile::Debug;
BuildProfile active_profile() { return current; }
}
