#ifndef RAF_GAME_H_
#define RAF_GAME_H_

#include "entity.hpp"
#include "map_state.hpp"
#include "planet.hpp"
#include "ship.hpp"
#include "squad.hpp"

#include <utility>

namespace raf {
namespace game {

void update_location_history(const hlt::Entity& entity, int round);

// returns whether found or not.
std::pair<bool, hlt::Location> get_location_for_round(const hlt::Entity& entity, int round);

hlt::Location entity_velocity(const hlt::Entity& entity, int start_round);


// Parse the map and derive any useful states
// eg update where ships are docked for example so this can be queried.
// current main() implementation iterates through ships but can send ships
// to locations that are in use my ships with later id numbers.
void parse_map_state();

// process all updates
// Parse current delta updates
// Compute new positions
// Recompute all stats about each player
void process_state_updates(int frame_number);

// Run all prediction algorithms
void run_prediction();

// Generate new moves for this frame.
void generate_moves();

}
}

#endif // !RAF_GAME_H_
