/*
 * Copyright 2021 Conquer Space
 */

#include <iostream>
#include <memory>
#include <fstream>

#include "client/scenes/loadingscene.h"
#include "engine/application.h"

int main(int argc, char* argv[]) {
    conquerspace::engine::Application application;

    // Set initial scene
    application.SetScene<conquerspace::scene::LoadingScene>();
    application.run();
}
