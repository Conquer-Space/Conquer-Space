/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#pragma once

#include <string>

#include "RmlUi/Core/ElementDocument.h"
#include "engine/application.h"

namespace cqsp::client {
class CreditsWindow : public Rml::EventListener {
 public:
    explicit CreditsWindow(cqsp::engine::Application& app);
    ~CreditsWindow();
    void OpenDocument();
    void Show();
    void Update(double delta_time);
    void Hide();
    std::string GetDocumentName();
    void ProcessEvent(Rml::Event& event);

 private:
    void LoadCreditsText();
    void SetCreditsScroll();

    Rml::ElementDocument* document;
    Rml::Element* credits_text_element;
    cqsp::engine::Application& m_app;

    std::string credits_text;
    float scroll_percentage = 0.0f;

    const float scroll_speed = 50;
    bool fast_scroll = false;
};
}  // namespace cqsp::client
