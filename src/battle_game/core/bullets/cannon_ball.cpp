#include "battle_game/core/bullets/cannon_ball.h"

#include "battle_game/core/game_core.h"
#include "battle_game/core/particles/particles.h"

namespace battle_game::bullet {
CannonBall::CannonBall(GameCore *core,
                       uint32_t id,
                       uint32_t unit_id,
                       uint32_t player_id,
                       glm::vec2 position,
                       float rotation,
                       float damage_scale,
                       glm::vec2 velocity)
    : Bullet(core, id, unit_id, player_id, position, rotation, damage_scale),
      velocity_(velocity) {
}

void CannonBall::Render() {
  SetTransformation(position_, rotation_, glm::vec2{0.1f});
  SetColor(game_core_->GetPlayerColor(player_id_));
  SetTexture(BATTLE_GAME_ASSETS_DIR "textures/particle3.png");
  DrawModel(0);
}

void CannonBall::Update() {
  bool should_die = false;
  for (int updaten = 0; updaten < 10; updaten++) {
    if (game_core_->IsBlockedByObstacles(position_ +
                                         velocity_ * (kSecondPerTick / 10))) {
      float x1 = 0.0;
      glm::vec2 vposition = position_ + Rotate({0.0f, 2.05f * kSecondPerTick},
                                               glm::radians(0.0f));
      bool vdie = game_core_->IsBlockedByObstacles(vposition);
      for (float x = 1.0; x < 360.0; x += 1.0) {
        glm::vec2 vpositionx =
            position_ + Rotate({0.0f, 2.05f * kSecondPerTick}, glm::radians(x));
        if (game_core_->IsBlockedByObstacles(vpositionx) != vdie) {
          x1 = x;
          vdie = game_core_->IsBlockedByObstacles(vpositionx);
          break;
        }
      }
      for (float x = x1; x < 360.0 + x1; x += 1.0) {
        glm::vec2 vpositionx =
            position_ + Rotate({0.0f, 2.05f * kSecondPerTick}, glm::radians(x));
        if (game_core_->IsBlockedByObstacles(vpositionx) != vdie) {
          glm::vec2 vtangent =
              Rotate({0.0f, 2.05f * kSecondPerTick}, glm::radians(x1)) -
              Rotate({0.0f, 2.05f * kSecondPerTick}, glm::radians(x - 1.0f));
          glm::vec2 tangent = {
              vtangent.x /
                  std::sqrt(vtangent.x * vtangent.x + vtangent.y * vtangent.y),
              vtangent.y /
                  std::sqrt(vtangent.x * vtangent.x +
                            vtangent.y *
                                vtangent.y)};  // Find the normalized tangent
                                               // of the obstacle
          velocity_ = {
              -velocity_.x +
                  2 * (velocity_.x * tangent.x + velocity_.y * tangent.y) *
                      tangent.x,
              -velocity_.y +
                  2 * (velocity_.x * tangent.x + velocity_.y * tangent.y) *
                      tangent.y,
          };  // Rebound
          reboundn--;
          if (!reboundn) {
            should_die = true;
          }
          break;
        }
        if (should_die) {
          break;
        }
      }
    }
    position_ += velocity_ * (kSecondPerTick / 10);
  }
  auto &units = game_core_->GetUnits();
  for (auto &unit : units) {
    if (unit.first == unit_id_) {
      continue;
    }
    if (unit.second->IsHit(position_)) {
      game_core_->PushEventDealDamage(unit.first, id_, damage_scale_ * 10.0f);
      should_die = true;
    }
  }

  if (should_die) {
    game_core_->PushEventRemoveBullet(id_);
  }
}

CannonBall::~CannonBall() {
  for (int i = 0; i < 5; i++) {
    game_core_->PushEventGenerateParticle<particle::Smoke>(
        position_, rotation_, game_core_->RandomInCircle() * 2.0f, 0.2f,
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, 3.0f);
  }
}
}  // namespace battle_game::bullet
