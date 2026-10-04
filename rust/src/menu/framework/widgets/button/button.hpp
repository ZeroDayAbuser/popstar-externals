#pragma once

class CButton : public CObject
{
public:
    CButton(std::string name, std::function<void()> callback, std::string tooltip = "")
    {
        SetName(name);
        SetCallback(callback);
        SetTooltip(tooltip);

        SetHoverAlpha(0.0f);
        SetClickedAlpha(0.0f);
    }

    void Render() override;

    void SetCallback(const std::function<void()> &callback)
    {
        this->callback = callback;
    }

    void TriggerCallback()
    {
        if (callback)
        {
            callback();
        }
    }

    float GetHoverAlpha() const
    {
        return hoverAlpha;
    }

    void SetHoverAlpha(float hoverAlpha)
    {
        this->hoverAlpha = hoverAlpha;
    }

    float GetClickedAlpha() const
    {
        return clickedAlpha;
    }

    void SetClickedAlpha(float clickedAlpha)
    {
        this->clickedAlpha = clickedAlpha;
    }

private:
    std::function<void()> callback;
    float hoverAlpha;
    float clickedAlpha;
};
