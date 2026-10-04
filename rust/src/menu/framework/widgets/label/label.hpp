#pragma once

class CLabel : public CObject
{
public:
    CLabel(std::string name, bool useCustomColor = false, Color customColor = Color::White())
    {
        SetName(name);
        SetUseCustomColor(useCustomColor);
        SetCustomColor(customColor);
    }

    void Render() override;

    bool GetUseCustomColor() const
    {
        return useCustomColor;
    }

    void SetUseCustomColor(bool useCustomColor)
    {
        this->useCustomColor = useCustomColor;
    }

    Color GetCustomColor() const
    {
        return customColor;
    }

    void SetCustomColor(Color customColor)
    {
        this->customColor = customColor;
    }

private:
    bool useCustomColor;
    Color customColor;
};
