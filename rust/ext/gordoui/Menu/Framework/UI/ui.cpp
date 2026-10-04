#include <Cheat.hpp>

bool UI::InBounds(CObject *parent)
{
    return Input::IsMouseOverRect(parent->GetPosition() + Vector2{0.0f, Style::containerHeaderHeight},
                                     parent->GetSize() - Vector2{0.0f, Style::containerHeaderHeight});
}

Vector2 UI::ClampToScreen(const Vector2 &position, const Vector2 &size)
{
    return ImClamp(position, {Style::padding, Style::padding}, ImGui::GetIO().DisplaySize - Vector2{Style::padding, Style::padding});
}

std::string UI::WrapText(const std::string &text, float max_width)
{
    std::string wrapped_text = "";
    std::stringstream section_ss(text);
    std::string line_segment;

    while (std::getline(section_ss, line_segment, '\n'))
    {
        std::stringstream word_ss(line_segment);
        std::string word;
        std::string current_line = "";

        while (word_ss >> word)
        {
            std::string test_line = current_line.empty() ? word : current_line + " " + word;
            Vector2 text_size = Render::CalcTextSize(test_line);

            if (text_size.x > max_width && !current_line.empty())
            {
                wrapped_text += current_line + "\n";
                current_line = word;
            }
            else
            {
                current_line = test_line;
            }
        }

        wrapped_text += current_line + "\n";
    }

    if (!wrapped_text.empty())
    {
        wrapped_text.pop_back();
    }

    return wrapped_text;
}
