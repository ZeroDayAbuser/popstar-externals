#pragma once

class CDropdown : public CObject
{
public:
    CDropdown(std::string name, int *value, const std::vector<std::string> &items, std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetItems(items);
        SetOpened(false);
        SetHoverAlpha(0.0f);
        SetActiveAlpha(0.0f);
        SetTooltip(tooltip);
        animHoverItems.resize(items.size());
        animActiveItems.resize(items.size());
        scroll = 0.0f;
    }

    void Render() override;

    int *GetValue()
    {
        return value;
    }

    void SetValue(int *value)
    {
        this->value = value;
    }

    const std::vector<std::string> &GetItems() const
    {
        return items;
    }

    void SetItems(const std::vector<std::string> &items)
    {
        this->items = items;
    }

    bool GetOpened() const
    {
        return opened;
    }

    void SetOpened(bool opened)
    {
        this->opened = opened;
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

    std::vector<float> GetAnimItemsHover() const
    {
        return animHoverItems;
    }

    void SetAnimItemsHover(std::vector<float> animItems)
    {
        this->animHoverItems = animItems;
    }

    void SetAnimItemHoverAt(std::size_t index, float anim)
    {
        if (index >= animHoverItems.size()) return;
        animHoverItems[index] = anim;
    }

    float GetAnimItemAtHover(std::size_t index)
    {
        if (index >= animHoverItems.size()) return 0.0f;
        return animHoverItems[index];
    }

    std::vector<float> GetAnimItemsActive() const
    {
        return animActiveItems;
    }

    void SetAnimItemsActive(std::vector<float> animItems)
    {
        this->animActiveItems = animItems;
    }

    void SetAnimItemActiveAt(std::size_t index, float anim)
    {
        if (index >= animActiveItems.size()) return;
        animActiveItems[index] = anim;
    }

    float GetAnimItemAtActive(std::size_t index)
    {
        if (index >= animActiveItems.size()) return 0.0f;
        return animActiveItems[index];
    }
private:
    int *value = nullptr;
    std::vector<std::string> items;
    std::vector<float> animHoverItems;
    std::vector<float> animActiveItems;

    bool opened;
    float hoverAlpha = 0.0f;
    float activeAlpha = 0.0f;
    float scroll = 0.0f;
};
