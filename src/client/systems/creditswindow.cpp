/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "client/systems/creditswindow.h"

#include <fstream>
#include <sstream>
#include <string>

#include "common/util/paths.h"

namespace {
std::string strip(const std::string& inpt) {
    if (inpt.empty()) {
        return "";
    }
    auto start_it = inpt.begin();
    auto end_it = inpt.rbegin();
    while (std::isspace(*start_it)) ++start_it;
    while (std::isspace(*end_it)) ++end_it;
    return std::string(start_it, end_it.base());
}
}  // namespace

cqsp::client::CreditsWindow::CreditsWindow(cqsp::engine::Application& app) : m_app(app) {}

cqsp::client::CreditsWindow::~CreditsWindow() {
    document->RemoveEventListener(Rml::EventId::Click, this);
    document->RemoveEventListener(Rml::EventId::Keydown, this);
    document->RemoveEventListener(Rml::EventId::Keyup, this);
}

void cqsp::client::CreditsWindow::OpenDocument() {
    document = m_app.LoadDocument(GetDocumentName());
    // Read credits text file and then parse
    credits_text_element = document->GetElementById("credits_text");
    document->AddEventListener(Rml::EventId::Click, this);
    document->AddEventListener(Rml::EventId::Keydown, this);
    document->AddEventListener(Rml::EventId::Keyup, this);

    LoadCreditsText();
}

void cqsp::client::CreditsWindow::Show() {
    scroll_percentage = m_app.GetWindowHeight();
    SetCreditsScroll();
    document->Show();
    document->SetClass("visible", true);
}

void cqsp::client::CreditsWindow::Update(double delta_time) {
    if (!document->IsVisible()) {
        return;
    }
    scroll_percentage -= (scroll_speed * delta_time);
    if (fast_scroll) {
        scroll_percentage -= (scroll_speed * delta_time * 5);
    }
    // Transform looks better for animations compared to using top.
    SetCreditsScroll();

    if (credits_text_element->GetBox().GetSize().y < (-scroll_percentage - scroll_speed * 2)) {
        Hide();
    }
    if (document->GetProperty("opacity")->Get<float>() <= 0 && !document->IsClassSet("visible")) {
        document->Hide();
    }
}

void cqsp::client::CreditsWindow::Hide() { document->SetClass("visible", false); }

std::string cqsp::client::CreditsWindow::GetDocumentName() {
    return cqsp::common::util::GetCqspDataPath() + "/core/gui/credits.rml";
}

void cqsp::client::CreditsWindow::ProcessEvent(Rml::Event& event) {
    if (event.GetId() == Rml::EventId::Keydown) {
        Rml::Input::KeyIdentifier key_identifier =
            (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);
        if (key_identifier == Rml::Input::KI_DOWN) {
            fast_scroll = true;
            return;
        }
    }
    if (event.GetId() == Rml::EventId::Keyup) {
        Rml::Input::KeyIdentifier key_identifier =
            (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);
        if (key_identifier == Rml::Input::KI_DOWN) {
            fast_scroll = false;
            return;
        }
    }
    document->Hide();
    document->SetClass("visible", false);
}

void cqsp::client::CreditsWindow::LoadCreditsText() {
    std::string credits_file = cqsp::common::util::GetCqspDataPath() + "/core/credits.md";

    std::ifstream credits_stream(credits_file);
    std::string line;
    std::stringstream buffer;
    // A really bad markdown to html/rml parser
    bool previous_return = false;
    while (std::getline(credits_stream, line)) {
        if (line.empty()) {
            if (previous_return) {
                // Then do a new line
                buffer << "<br />";
            }
            previous_return = true;
            continue;
        }
        previous_return = false;
        std::string tag = "p";
        if (line.rfind("###", 0) == 0) {
            tag = "h3";
            // remove prefix
            line = line.substr(3);
        } else if (line.rfind("##", 0) == 0) {
            tag = "h2";
            line = line.substr(2);
        } else if (line.rfind("#", 0) == 0) {
            tag = "h1";
            line = line.substr(1);
        }
        line = strip(line);

        buffer << fmt::format("<{0}>{1}</{0}>", tag, line);
        buffer << "<br />";
    }
    credits_text = buffer.str();
    // Parse the text, and then set the things
    // Go through line by line
    // Set document text
    document->GetElementById("credits_text")->SetInnerRML(credits_text);
}

void cqsp::client::CreditsWindow::SetCreditsScroll() {
    credits_text_element->SetProperty("transform", fmt::format("translateY({}px)", scroll_percentage));
}
