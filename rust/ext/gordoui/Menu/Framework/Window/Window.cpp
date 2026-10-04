#include <Cheat.hpp>

void CWindow::Render()
{
    Style::Update();

    this->SetWindowAlpha(
        std::clamp(this->GetWindowAlpha() + (15.0f * ImGui::GetIO().DeltaTime * (this->GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));

    //if (Input::IsKeyPressed(GetKey()))
    //    this->SetOpened(!this->GetOpened());

    Render::alpha = this->GetWindowAlpha();
    if (this->GetWindowAlpha() <= 0.0f)
    {
        Render::alpha = 1.0f;
        return;
    }

    Render::AddShadowRect(GetPosition(), GetSize(), Style::shadow, 25.0f, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::background, Style::rounding);

    // header
    Render::AddDropshadow(GetPosition() + Vector2{0.0f, Style::headerHeight}, GetSize().x, Style::shadow, 15.0f);
    Render::AddRectFilled(GetPosition(), Vector2{GetSize().x, Style::headerHeight}, Style::header, Style::rounding, ImDrawFlags_RoundCornersTop);
    Render::AddRectFilled(GetPosition() + Vector2{0.0f, Style::headerHeight - 1.0f}, Vector2{GetSize().x, 1.0f}, Style::outline);

    Render::AddText(GetName(), GetPosition() + Vector2{Style::padding, Style::headerHeight / 2.0f}, Style::headerText, TEXT_FLAG_NONE,
                     {0.0f, 0.5f}, Render::Fonts::montserratBold16px);

    if (!GetTld().empty())
    {
        Render::AddText(GetTld(),
                         GetPosition() + Vector2{Style::padding + Render::CalcTextSize(GetName(), Render::Fonts::montserratBold16px).x,
                                                  Style::headerHeight / 2.0f},
                         Style::accentColor, TEXT_FLAG_NONE, {0.0f, 0.5f}, Render::Fonts::montserratBold16px);
    }

    // render tabs if they exist
    // calculate the base tab width as we go left to right aligned right :^)
    float tab_offset = Style::padding / 2.0f;
    for (auto c : GetChildren())
    {
        CTab *t = dynamic_cast<CTab *>(c);
        if (!t)
            continue;

        // init the initial tab hash
        if (this->GetTabHash() == 0)
            this->SetTabHash(std::hash<std::string>()(t->GetName()));

        // give the tabs the working area
        t->SetSize({GetSize().x - (Style::padding * 2.0f), GetSize().y - Style::headerHeight + (Style::padding * 2.0f)});
        t->SetPosition(GetPosition());
        t->Render();

        tab_offset += Render::CalcTextSize(t->GetName()).x + Style::padding;
    }

    float copied_offset = tab_offset;

    // actually render the tabs
    for (auto c : GetChildren())
    {
        CTab *t = dynamic_cast<CTab *>(c);
        if (!t)
            continue;

        Vector2 base_position = GetPosition() + Vector2{GetSize().x - tab_offset, 0.0f};
        Vector2 size = Vector2{Render::CalcTextSize(t->GetName()).x + Style::padding, Style::headerHeight};

        bool hovered = Input::IsMouseOverRect(base_position, size) && !this->HasBlockingObject();
        if (hovered && Input::IsMouseClicked())
        {
            this->SetTabHash(t->GetTabHash());
        }

        t->SetTabAlpha(std::clamp(t->GetTabAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (t->ShouldShow() ? 1.0f : -1.0f)), 0.0f, 1.0f));

        Render::AddText(t->GetName(), base_position + size / 2.0f, Style::dimmedText.Lerp(Style::headerText, t->GetTabAlpha()), TEXT_FLAG_NONE,
                         {0.5f, 0.5f});
        tab_offset -= size.x;
    }

    // explicitly render base objects
    for (auto c : GetChildren())
    {
        CTab *t = dynamic_cast<CTab *>(c);
        CContainer *cc = dynamic_cast<CContainer *>(c);

        if (t || cc)
            continue;

        c->SetPosition(GetPosition() + Vector2{Style::padding, Style::headerHeight + Style::padding});
        c->SetSize(GetSize());

        c->Render();

        Vector2 s = c->GetSize();
        if (s.x > GetSize().x)
            SetSize(Vector2{s.x + (Style::padding * 2.0f), GetSize().y});

        if (s.y > GetSize().y)
            SetSize(Vector2{GetSize().x, s.y + Style::headerHeight + (Style::padding * 2.0f)});
    }

    Render::AddRect(GetPosition(), GetSize(), Style::outline, Style::rounding);

    if (Input::IsMouseOverRect(GetPosition(), Vector2{GetSize().x - copied_offset, Style::headerHeight}) && Input::IsMouseClicked() &&
        !this->HasBlockingObject())
    {
        SetDragging(true);
        SetDragOffset(ImGui::GetIO().MousePos - GetPosition());
    }

    if (GetDragging())
    {
        if (!Input::IsKeyDown(VK_LBUTTON))
            SetDragging(false);

        Vector2 desired = ImGui::GetIO().MousePos - GetDragOffset();
        const float max_x = std::max(0.0f, ImGui::GetIO().DisplaySize.x - GetSize().x - Style::padding);
        const float max_y = std::max(0.0f, ImGui::GetIO().DisplaySize.y - GetSize().y - Style::padding);
        const float cx = std::clamp(desired.x, Style::padding, max_x);
        const float cy = std::clamp(desired.y, Style::padding, max_y);

        SetPosition({std::roundf(cx), std::roundf(cy)});
    }

    Render::alpha = 1.0f;
}
