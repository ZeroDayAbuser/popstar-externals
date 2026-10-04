#include "slider.hpp"
// Slider<T> is fully header-defined because it's templated.
// Explicit instantiations to satisfy the vcxproj ClCompile entry.
namespace gui {
template class Slider<int>;
template class Slider<float>;
}
