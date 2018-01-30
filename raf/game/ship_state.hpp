#ifndef RAF_SHIP_STATE_H_
#define RAF_SHIP_STATE_H_

#include "ship.hpp"

namespace raf {
namespace game {

enum ShipState {
  Spawned,
  TravellingToPlanet,
  UnderAttack,
  DefendingPlanet,
  AttackingSolo,
  AttackingWithSquad,
};


template<typename T, typename impl>
class State {
public:
  void run(const T& obj) { impl::run(obj); }
};

class AttackState : public State<Ship, AttackState> {
public:
  void run(const Ship& obj) {
  }
};

class DockedState : public State<Ship, DockedState> {
public:
  void run(const Ship& obj) {

    // Look for enemies
    // Sound alarm for help if near by

  }
};

}
}
#endif // !RAF_SHIP_STATE_H_
