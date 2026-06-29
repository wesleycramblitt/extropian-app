// CurveEditor component — C++ stub.
// In the future: interactive plot for editing BC profiles over time.

#include <string>
#include <vector>

namespace exd::app::rmlui {

struct CurvePoint {
    double x = 0, y = 0;
};

inline void set_curve_points(const std::string& canvas_id,
                              const std::vector<CurvePoint>& points) {
    // TODO: Render curve points to the curve-editor canvas element.
    (void)canvas_id;
    (void)points;
}

} // namespace exd::app::rmlui
