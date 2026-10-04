#pragma once

constexpr std::size_t cp_hs = 1;
constexpr std::size_t cp_hue = 2;
constexpr std::size_t cp_alpha = 3;

class CColorPicker : public CObject
{
public:
    CColorPicker(std::string name, Color *value, bool showAlphaBar = false) {
        SetName(name);
        SetValue(value);
        SetAlphaBar(showAlphaBar);
        SetOpened(false);
        SetOpenAlpha(0.0f);
        savedHSV = value->ToHSV();
    }

    void Render() override;

    void SetValue(Color* value)
    {
        this->value = value;
    }

    Color* GetValue() const
    {
        return value;
    }

    bool GetAlphaBar() const
    {
        return showAlphaBar;
    }

    void SetAlphaBar(bool showAlphaBar)
    {
        this->showAlphaBar = showAlphaBar;
    }

    bool GetOpened() const
    {
        return opened;
    }

    void SetOpened(bool opened)
    {
        this->opened = opened;
    }

    float GetOpenAlpha() const
    {
        return openAlpha;
    }

    void SetOpenAlpha(float alpha)
    {
        this->openAlpha = alpha;
    }

    std::size_t GetCtxHash() const
    {
        return ctxHash;
    }

    void SetCtxHash(std::size_t ctxHash)
    {
        this->ctxHash = ctxHash;
    }

    HSV GetHSV() const
    {
        return savedHSV;
    }

    void SetHSV(HSV newHSV)
    {
        this->savedHSV = newHSV;
    }

private:
    bool opened;
    Color *value;
    float openAlpha;
    HSV savedHSV;
    bool showAlphaBar;
    std::size_t ctxHash;
};
