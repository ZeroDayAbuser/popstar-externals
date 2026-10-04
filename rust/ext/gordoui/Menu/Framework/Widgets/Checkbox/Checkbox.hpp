#pragma once

class CCheckbox : public CObject
{
public:
    CCheckbox(std::string name, bool *value, std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetTooltip(tooltip);
    }

    void Render() override;

    bool *GetValue() const
    {
        return value;
    }

    void SetValue(bool *value)
    {
        this->value = value;
    }

    float GetHoverAlpha() const
    {
        return hoverAlpha;
    }

    void SetHoverAlpha(float hoverAlpha)
    {
        this->hoverAlpha = hoverAlpha;
    }

    float GetActiveAlpha() const
    {
        return activeAlpha;
    }

    void SetActiveAlpha(float activeAlpha)
    {
        this->activeAlpha = activeAlpha;
    }
private:
    bool *value = nullptr;
    float hoverAlpha = 0.0f;
    float activeAlpha = 0.0f;
};
