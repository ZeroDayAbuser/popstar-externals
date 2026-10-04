#pragma once

struct Keybind
{
    int key = 0;
    int mode = 0;

    bool GetState(std::function<void()> stateChangeCallback = nullptr);

    bool GetConstState(std::function<void()> stateChangeCallback = nullptr);

    bool Enabled() const
    {
        if (mode == 0)
            return false;
        
        if (mode == 3)
            return true;

        if (key > 0)
            return true;

        return false;
    }

    bool toggleState = false;
    bool constCb = false;
};

class CKeybind : public CObject
{
public:
    CKeybind(std::string name, Keybind *value, std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetTooltip(tooltip);
    }

    void Render() override;

    Keybind *GetValue() const
    {
        return value;
    }

    void SetValue(Keybind *value)
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

    float GetFieldActiveAlpha() const
    {
        return fieldActiveAlpha;
    }

    void SetFieldActiveAlpha(float alpha)
    {
        this->fieldActiveAlpha = alpha;
    }

    float GetFieldHoverAlpha() const
    {
        return fieldHoverAlpha;
    }

    void SetFieldHoverAlpha(float alpha)
    {
        this->fieldHoverAlpha = alpha;
    }

    float GetEnabledAlpha() const
    {
        return enabledAlpha;
    }

    void SetEnabledAlpha(float enabledAlpha)
    {
        this->enabledAlpha = enabledAlpha;
    }

    float GetAlwaysAlpha() const
    {
        return alwaysAlpha;
    }

    void SetAlwaysAlpha(float alwaysAlpha)
    {
        this->alwaysAlpha = alwaysAlpha;
    }

private:
    Keybind *value;
    float hoverAlpha = 0.0f;
    float fieldActiveAlpha = 0.0f;
    float fieldHoverAlpha = 0.0f;

    float enabledAlpha = 0.0f;
    float alwaysAlpha = 0.0f;

    std::unique_ptr<CPopup> statePopup = nullptr;

    bool listening = false;
    bool awaitRelease = false;
};
