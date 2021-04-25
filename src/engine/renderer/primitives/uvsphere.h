/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "engine/renderer/renderable.h"
#include "engine/renderer/mesh.h"

namespace conquerspace {
namespace primitive {
void ConstructSphereMesh(int x_segments, int y_segments, conquerspace::engine::Mesh& mesh);
}  // namespace primitive
}  // namespace conquerspace
