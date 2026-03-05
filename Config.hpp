#pragma once

// Window
constexpr unsigned int WINDOW_W = 1280;
constexpr unsigned int WINDOW_H = 720;
constexpr unsigned int MIN_WINDOW_W = 800;
constexpr unsigned int MIN_WINDOW_H = 600;

// Tiles
constexpr float TILE_SIZE = 16.f;
constexpr float TILE_SCALE = 3.0f;
constexpr float TILE_DRAW_SIZE = TILE_SIZE * TILE_SCALE; // 48px

// Entities (player, enemies — sprite 32x32)
constexpr float ENTITY_SCALE = 3.0f;
constexpr float ENTITY_FRAME_SIZE = 32.f;
constexpr float ENTITY_DRAW_SIZE = ENTITY_FRAME_SIZE * ENTITY_SCALE; // 96px

// Animation
constexpr float ANIM_WALK_FRAME_TIME = 0.12f;
constexpr float ANIM_IDLE_FRAME_TIME = 0.18f;

// Player
constexpr float PLAYER_SPEED = 140.f;

// Framerate
constexpr unsigned int FRAMERATE_LIMIT = 60;