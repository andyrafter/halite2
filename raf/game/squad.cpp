#include "squad.hpp"
#include "../math/math.hpp"

struct Message {
  enum class MessageId {
    AttackEnemy,
    JoinSquad,
  };

  int msg_target_entity_id;
  int msg_src_entity_id;

  struct {
    union {
      struct {
        int attack_id;
      } attack;
      struct {
        int squad_id;
      } squad;
    } x;
  } data;
};

double raf::game::UnitGroup::radius()
{
  math::Vec2d min(math::Vec2d::Max());
  math::Vec2d max(math::Vec2d::Min());

  double min_x = std::numeric_limits<double>::max();
  double min_y = std::numeric_limits<double>::max();
  double max_x = std::numeric_limits<double>::lowest();
  double max_y = std::numeric_limits<double>::lowest();
  for (const auto &e : ent_to_slot_) {

    // Game interface lookup ent pos
    // e.first
    double x = 0;
    double y = 0;

    min_x = std::min(min_x, x);
    min_y = std::min(min_y, x);
    max_x = std::max(max_x, x);
    max_y = std::max(max_y, x);
  }

  math::Vec2d min_bounds(min_x, min_y);
  math::Vec2d max_bounds(max_x, max_y);
  return 0.0;
}
