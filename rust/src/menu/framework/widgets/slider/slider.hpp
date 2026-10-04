#pragma once

class CSliderFloat : public CObject
{
public:
    CSliderFloat(std::string name, float *value, float min, float max, std::string format = "{}", std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetMin(min);
        SetMax(max);
        SetFormat(format);
        SetTooltip(tooltip);
        SetActive(false);
        SetHoverAlpha(0.0f);
        SetActiveAlpha(0.0f);
        SetLerpedValue(0.0f);
    }

    void Render() override;

    float* GetValue() const
    {
        return value;
    }

    void SetValue(float* value)
    {
        this->value = value;
    }

    float GetMin() const
    {
        return min;
    }

    void SetMin(float min)
    {
        this->min = min;
    }

    float GetMax() const
    {
        return max;
    }

    void SetMax(float max)
    {
        this->max = max;
    }

    std::string GetFormat() const
    {
        return format;
    }

    void SetFormat(std::string format)
    {
        this->format = format;
    }

    bool GetActive() const
    {
        return active;
    }

    void SetActive(bool active)
    {
        this->active = active;
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

    float GetLerpedValue() const
    {
        return lerpedValue;
    }

    void SetLerpedValue(float lerpedValue)
    {
        this->lerpedValue = lerpedValue;
    }

private:
    float *value;
    float min;
    float max;
    std::string format;

    bool active;

    float hoverAlpha;
    float activeAlpha;
    float lerpedValue;
};

class CSliderInt : public CObject
{
public:
    CSliderInt(std::string name, int *value, int min, int max, std::string format = "{}", std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetMin(min);
        SetMax(max);
        SetFormat(format);
        SetTooltip(tooltip);
        SetActive(false);
        SetHoverAlpha(0.0f);
        SetActiveAlpha(0.0f);
        SetLerpedValue(0.0f);
    }

    void Render() override;

    int *GetValue() const
    {
        return value;
    }

    void SetValue(int *value)
    {
        this->value = value;
    }

    int GetMin() const
    {
        return min;
    }

    void SetMin(int min)
    {
        this->min = min;
    }

    int GetMax() const
    {
        return max;
    }

    void SetMax(int max)
    {
        this->max = max;
    }

    std::string GetFormat() const
    {
        return format;
    }

    void SetFormat(std::string format)
    {
        this->format = format;
    }

    bool GetActive() const
    {
        return active;
    }

    void SetActive(bool active)
    {
        this->active = active;
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

    float GetLerpedValue() const
    {
        return lerpedValue;
    }

    void SetLerpedValue(float lerpedValue)
    {
        this->lerpedValue = lerpedValue;
    }

private:
    int *value;
    int min;
    int max;
    std::string format;

    bool active;

    float hoverAlpha;
    float activeAlpha;
    float lerpedValue;
};
