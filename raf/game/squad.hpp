#ifndef RAF_SQUAD_H_
#define RAF_SQUAD_H_

#include "entity.hpp"

#include <array>
#include <map>
#include <vector>

namespace raf {
namespace game {

template<int SIZE>
//constexpr in c++14
const static std::array<int, SIZE> static_table_generator() {
  static std::array<int, SIZE> arr;
  for (int i = 0; i < SIZE; i++) {
    arr[i] = i;
  }
  return arr;
}


// NameS? Squads?, Groups?
class UnitSquad;
class UnitGroup {
public:
  // FIXME: Dynamically generate free slot for any range up to N.
  // Currently hardcoded for MAX = 3.
  UnitGroup() : free_slots_(std::begin(static_table_generator<NUM_ATTACHMENT_POINTS>()), std::end(static_table_generator<NUM_ATTACHMENT_POINTS>()))
  {
  }

  bool add_entity(const Entity *ent) {
    // Ent already exists
    if (is_member(ent)) {
      return false;
    }

    // There are no free slots.
    if (free_slots_.empty()) {
      return false;
    }

    auto slot = free_slots_.back();
    free_slots_.pop_back();
  }

  bool remove_entity(const Entity *ent) {
    // Entity is not a member of this grou[
    if (!is_member(ent)) {
      return false;
    }
    auto slot = ent_to_slot_[ent->id()];
    free_slots_.push_back(slot);

    ent_to_slot_.erase(
      ent_to_slot_.find(ent->id())
    );
  }

  math::Vec2d attachment_offset(const Entity *ent) const {
    auto slot_it = ent_to_slot_.find(ent->id());
    if (slot_it == std::end(ent_to_slot_)) {
      return math::Vec2d::Zero();
    }
    return attachment_offsets_[slot_it->second];
  }

  bool is_member(const Entity *ent) const {
    return lookup_entity(ent);
  }

  void update_origin(math::Vec2d origin) {}

  UnitGroup merge(const UnitGroup& other) {
    return UnitGroup{};
  }

  math::Vec2d origin() const;
  double radius();

private:
  // See if an entity exists in our map
  // returns true if it does
  // returns false if it does not
  bool lookup_entity(const Entity *ent) const {
    return ent_to_slot_.find(ent->id()) != std::end(ent_to_slot_);
  }

  using SlotId = int;
  constexpr static SlotId NUM_ATTACHMENT_POINTS = 3;
  constexpr static SlotId SLOT_BASE = 0;
  // Attachment offers. Offsets from center point where units can be attached.
  static const math::Vec2d attachment_offsets_[NUM_ATTACHMENT_POINTS];

  // Center origin of the group. This is the point that is moved towards other points.
  // This is used along with attachment offset to calculate the position of each entity.
  math::Vec2d center_origin_;

  std::map<EntityId, SlotId> ent_to_slot_;
  std::vector<SlotId> free_slots_;


};

//const math::Vec2d UnitGroup::attachment_offsets_[NUM_ATTACHMENT_POINTS] = {
//  { 0, 1 },
//  { -1, -1 },
//  { 1, -1 },
//};

class AttackGroup {};
class DefenceGroup {};

}
}


#endif // !RAF_SQUAD_H_
