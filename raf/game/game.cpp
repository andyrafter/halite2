#include "game.hpp"
#include <map>
#include <utility>

namespace raf {
namespace game {

std::map<hlt::EntityId, std::map<int, hlt::Location>> location_history;

inline void update_location_history(const hlt::Entity& entity, int round) {
  location_history[entity.entity_id][round] = entity.location;
}

// returns whether found or not.
inline std::pair<bool, hlt::Location> get_location_for_round(const hlt::Entity& entity, int round) {

  auto item = location_history[entity.entity_id].find(round);

  hlt::Location location;
  if (item == std::end(location_history[entity.entity_id])) {
    return { false, location };
  }

  location = item->second;
  return { true, location };
}

inline hlt::Location entity_velocity(const hlt::Entity& entity, int start_round) {
  auto current = get_location_for_round(entity, start_round);
  auto prev = get_location_for_round(entity, start_round);

  if (current.first && prev.first) {
    return { current.second.pos_x - prev.second.pos_x, current.second.pos_y - prev.second.pos_y };
  }

  // Can't tell what the velocity is.
  return { 0, 0 };
}


// Parse the map and derive any useful states
// eg update where ships are docked for example so this can be queried.
// current main() implementation iterates through ships but can send ships
// to locations that are in use my ships with later id numbers.
void parse_map_state() {}

// process all updates
// Parse current delta updates
// Compute new positions
// Recompute all stats about each player
void process_state_updates(int frame_number) {}

// Run all prediction algorithms
void run_prediction() {}

// Generate new moves for this frame.
void generate_moves() {}

} // namespace game
} // namespace raf
