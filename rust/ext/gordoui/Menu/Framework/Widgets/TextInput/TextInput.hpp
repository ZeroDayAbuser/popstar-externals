#pragma once

class CTextInput : public CObject
{
public:
    CTextInput(std::string name, std::string *value, bool hashed = false, bool useNameAsPreview = false, std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetHashed(hashed);
        SetUseNameAsPreview(useNameAsPreview);
        SetTooltip(tooltip);
        SetActive(false);
        SetCursorPos(0);
        SetSelectionAnchor(0);

        SetHoverAlpha(0.0f);
        SetActiveAlpha(0.0f);
        SetNotEmptyAlpha(0.0f);
    }

    void Render() override;

    std::string* GetValue() const
    {
        return value;
    }

    void SetValue(std::string* value)
    {
        this->value = value;
    }

    bool GetHashed() const
    {
        return hashed;
    }

    void SetHashed(bool hashed)
    {
        this->hashed = hashed;
    }

    bool GetUseNameAsPreview() const
    {
        return useNameAsPreview;
    }

    void SetUseNameAsPreview(bool useNameAsPreview)
    {
        this->useNameAsPreview = useNameAsPreview;
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

    float GetNotEmptyAlpha() const
    {
        return notEmptyAlpha;
    }

    void SetNotEmptyAlpha(float notEmptyAlpha)
    {
        this->notEmptyAlpha = notEmptyAlpha;
    }

    bool GetActive() const
    {
        return active;
    }

    void SetActive(bool active)
    {
        this->active = active;
    }

    std::size_t GetCursorPos() const
    {
        return cursorPos;
    }

    void SetCursorPos(std::size_t cursorPos)
    {
        this->cursorPos = cursorPos;
    }

    std::size_t GetSelectionAnchor() const
    {
        return selectionAnchor;
    }

    void SetSelectionAnchor(std::size_t selectionAnchor)
    {
        this->selectionAnchor = selectionAnchor;
    }

private:
    std::string *value;
    bool hashed;
    bool useNameAsPreview;

    bool active;
    std::size_t cursorPos = 0;
    std::size_t selectionAnchor = 0;

    float hoverAlpha = 0.0f;
    float activeAlpha = 0.0f;
    float notEmptyAlpha = 0.0f;
};
