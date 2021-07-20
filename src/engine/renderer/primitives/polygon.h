/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "engine/renderer/mesh.h"

namespace conquerspace {
namespace primitive {
void CreateFilledCircle(conquerspace::engine::Mesh& renderable, int segments = 64);
void CreateFilledTriangle(conquerspace::engine::Mesh& renderable);
void CreateFilledSquare(conquerspace::engine::Mesh& renderable);
}  // namespace primitive
}  // namespace conquerspace
