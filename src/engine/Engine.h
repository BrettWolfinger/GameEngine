/// @file Engine.h
/// @brief Public entry point for the Engine API.
///
/// Game code should include this header (or a per-subsystem facade header)
/// instead of reaching into engine internals directly.
///
/// @code
/// #include <engine/Engine.h>
///
/// Engine::Audio::playTone(440.f, 0.1f);
/// Engine::Particles::emit(params);
/// Engine::Collision::add(desc, callback);
/// @endcode
#pragma once
#include <engine/facade/Audio.h>
#include <engine/facade/Collision.h>
#include <engine/facade/Config.h>
#include <engine/facade/Events.h>
#include <engine/facade/Particles.h>
#include <engine/renderer/UVRect.h>
#include <engine/tilemap/Tilemap.h>
#include <engine/tilemap/TilemapRenderer.h>
