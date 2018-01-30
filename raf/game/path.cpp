#include "path.hpp"
#include <iostream>

namespace raf {
namespace game {

std::ostream& operator<<(std::ostream&os, const Path& path) {
  os << "Path: id=" << path.ship_id << ", start=" << path.start_pos << ", end=" << path.end_pos;
  return os;
}

}
}
