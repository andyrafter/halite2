#ifndef RAF_GAME_DECISION_H_
#define RAF_GAME_DECISION_H_

#include "entity.hpp"
#include "../math/math.hpp"

namespace raf {
namespace game {

struct Decision {
  enum class Action {
    Attack,
    Defend,
    Dock,
  };

  Action action;
  // C++14: Use default initialisers for these fields.
  // http://en.cppreference.com/w/cpp/language/aggregate_initialization
  EntityId unit_id;
  EntityId target_id;
  EntityId defendee;
  //math::Velocity velocity;
  // Move parameters...

  static Decision attack(EntityId unit, EntityId target) {
    return Decision{ Action::Attack, unit, target, INVALID_ENTITIY_ID };
  }
  static Decision defend(EntityId defender, EntityId defendee, EntityId target) {
    return Decision{ Action::Defend, defender, defendee, target };
  }
  static Decision dock(EntityId unit, EntityId target) {
    return Decision{ Action::Dock, unit, target, INVALID_ENTITIY_ID };
  }
};

}
}

#endif // RAF_GAME_COLLISION_H_
