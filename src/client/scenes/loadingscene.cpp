/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "client/scenes/loadingscene.h"

#include <RmlUi/Core.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <fstream>
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tracy/Tracy.hpp>
#include <tracy/common/TracySystem.hpp>

#include "client/scenes/mainmenu/mainmenuscene.h"
#include "client/scenes/modelscene.h"
#include "client/scenes/objecteditor/objecteditorscene.h"
#include "client/scenes/texttestscene.h"
#include "client/scenes/universeloadingscene.h"
#include "common/scripting/scripting.h"
#include "common/util/paths.h"
#include "engine/asset/assetmanager.h"
#include "engine/gui.h"

#define LOADING_ID "core/gui/screens/loading_screen.rml"

cqsp::scene::LoadingScene::LoadingScene(cqsp::engine::Application& app) : Scene(app) {
    m_done_loading = false;
    percentage = 0;
}

cqsp::scene::LoadingScene::~LoadingScene() {
    if (thread != nullptr && thread->joinable()) {
        thread->join();
    }
}

void cqsp::scene::LoadingScene::Init() {
    auto loading = [&]() {
        SPDLOG_INFO("Loading resources");
        LoadResources();
        SPDLOG_INFO("Need halt: {}", need_halt);
    };

    thread = std::make_unique<std::thread>(loading);
    thread->detach();

    Rml::DataModelConstructor constructor = GetApp().GetRmlUiContext()->CreateDataModel("loading");
    constructor.Bind("current", &loading_data.current);
    constructor.Bind("max", &loading_data.max);
    model_handle = constructor.GetModelHandle();
    document = GetApp().LoadDocument(LOADING_ID);
    if (document != nullptr) {
        document->Show();
    } else {
        SPDLOG_WARN("Couldn't load document");
    }
}

void cqsp::scene::LoadingScene::Update(float deltaTime) {
    while (assetLoader.QueueHasItems()) {
        assetLoader.BuildNextAsset();
    }
    if (m_done_loading && !assetLoader.QueueHasItems() && !need_halt) {
        // Load font after all the shaders are done
        LoadFont();

        // Load audio
        auto hjson = GetAssetManager().GetAsset<cqsp::asset::HjsonAsset>("core:ui_sounds");
        for (const auto& element : hjson->data) {
            auto audio_asset = GetAssetManager().GetAsset<cqsp::asset::AudioAsset>(element.second.to_string());
            if (audio_asset == nullptr) {
                SPDLOG_WARN("Cannot find audio asset {}", element.second.to_string());
                continue;
            }
            GetApp().GetAudioInterface().AddAudioClip(element.first, audio_asset);
        }

        // Remove data model
        GetApp().GetRmlUiContext()->RemoveDataModel("loading");
        GetApp().CloseDocument(LOADING_ID);

        // Set main menu scene
        if (GetApp().HasCmdLineArgs("-i")) {
            SPDLOG_INFO("Loading universe scene, skipping the main menu scene");
            GetApp().SetScene<cqsp::scene::UniverseLoadingScene>();
        } else if (GetApp().HasCmdLineArgs("-tt")) {
            GetApp().SetScene<cqsp::scene::TextTestScene>();
        } else if (GetApp().HasCmdLineArgs("-ov")) {
            GetApp().SetScene<cqsp::scene::ObjectEditorScene>();
        } else if (GetApp().HasCmdLineArgs("-mv")) {
            GetApp().SetScene<cqsp::scene::ModelScene>();
        } else {
            SPDLOG_INFO("Loading main menu");
            GetApp().SetScene<cqsp::scene::MainMenuScene>();
        }
    }
}

void cqsp::scene::LoadingScene::Ui(float deltaTime) {
    // Load rmlui ui
    if (m_done_loading) {
        return;
    }
    float current = static_cast<float>(assetLoader.getCurrentLoading());
    float max = static_cast<float>(assetLoader.getMaxLoading());
    loading_data.max = static_cast<int>(max);
    loading_data.current = static_cast<int>(current);
    model_handle.DirtyVariable("max");
    model_handle.DirtyVariable("current");
    Rml::ElementProgress* progress = ((Rml::ElementProgress*)document->GetElementById("loading_bar"));
    progress->SetValue(current);
    progress->SetMax(max);
}

void cqsp::scene::LoadingScene::Render(float deltaTime) {}

void cqsp::scene::LoadingScene::LoadResources() {
    ZoneScoped;
    // Loading goes here
    // Read core mod
    assetLoader.manager = &GetAssetManager();
    assetLoader.LoadMods();

    SPDLOG_INFO("Done loading items");
    need_halt = !assetLoader.GetMissingAssets().empty();
    m_done_loading = true;
}

void cqsp::scene::LoadingScene::LoadFont() {
    cqsp::asset::ShaderProgram* fontshader =
        new asset::ShaderProgram(*GetAssetManager().GetAsset<cqsp::asset::Shader>("core:fontvertexshader"),
                                 *GetApp().GetAssetManager().GetAsset<cqsp::asset::Shader>("core:fontfragshader"));

    cqsp::asset::Font* font = GetApp().GetAssetManager().GetAsset<cqsp::asset::Font>("core:defaultfont");

    GetApp().SetFont(font);
    GetApp().SetFontShader(fontshader);
}
