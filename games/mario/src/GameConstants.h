#pragma once
#include <cstdint>

// NES tile size (16×16 source pixels)
static constexpr int TILE  = 16;
// Render scale — NES native is 256×240, scale 3 gives 768×720
static constexpr int SCALE = 3;

// Visible screen in tiles (NES: 16 cols × 15 rows)
static constexpr int SCREEN_COLS = 16;
static constexpr int SCREEN_ROWS = 15;

static constexpr int WIN_W = SCREEN_COLS * TILE * SCALE; // 768
static constexpr int WIN_H = SCREEN_ROWS * TILE * SCALE; // 720

// Mario sprite frame dimensions (native pixels)
static constexpr int MARIO_FRAME_W = 32;
static constexpr int MARIO_FRAME_H = 16;

// Hitbox — inset from the sprite frame to exclude transparent padding
static constexpr int MARIO_HITBOX_OFFSET_X = 10; // transparent px on each horizontal side
static constexpr int MARIO_HITBOX_OFFSET_Y = 0;
static constexpr int MARIO_HITBOX_W = MARIO_FRAME_W - MARIO_HITBOX_OFFSET_X * 2; // 12px
static constexpr int MARIO_HITBOX_H = MARIO_FRAME_H - MARIO_HITBOX_OFFSET_Y * 2;

// Goomba sprite frame dimensions (native pixels, no transparent padding)
static constexpr int GOOMBA_FRAME_W = 16;
static constexpr int GOOMBA_FRAME_H = 16;
static constexpr int GOOMBA_HEAD_H  =  5; // native px of stomp zone at top of sprite

// Collision layer bits
inline constexpr uint32_t kLayerPlayer      = 1 << 0;
inline constexpr uint32_t kLayerPlayerStomp = 1 << 1; // thin sensor at Mario's feet
inline constexpr uint32_t kLayerEnemyBody   = 1 << 2;
inline constexpr uint32_t kLayerEnemyHead   = 1 << 3; // stompable zone at top of enemy
