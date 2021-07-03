/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "engine/renderer/renderable.h"
#include "engine/renderer/mesh.h"
namespace conquerspace {
namespace primitive {
void MakeTexturedPaneMesh(conquerspace::engine::Renderable&);
// Normal pane
void MakeTexturedPaneMesh(conquerspace::engine::Mesh&);
// In case you need a mirrored pane, like for framebuffers
void MakeTexturedPaneMesh(conquerspace::engine::Mesh&, bool);
}
}
