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
#include "client/scenes/rmlscene.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Debugger.h>

namespace cqsp::client::scene {
void RmlClientScene::CheckUiReload() {
    if (!GetApp().ButtonIsReleased(engine::KeyInput::KEY_F5)) {
        return;
    }
    Rml::Factory::ClearStyleSheetCache();
    Rml::Factory::ClearTemplateCache();
    auto context = Rml::GetContext(0);
    for (int i = 0; i < context->GetNumDocuments(); i++) {
        Rml::ElementDocument* document = context->GetDocument(i);
        const Rml::String& src = document->GetSourceURL();
        if (src.empty()) {
            continue;
        }
        if (GetApp().DocumentIsLoaded(src)) {
            continue;
        }
        document->Close();
    }
    for (auto& ui : documents) {
        ui->ReloadWindow();
    }
}
}  // namespace cqsp::client::scene
