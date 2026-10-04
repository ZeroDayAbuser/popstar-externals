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
        animHoverItems.at(index) = anim;
    }

    float GetAnimItemAtHover(std::size_t index)
    {
        return animHoverItems.at(index);
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
        animActiveItems.at(index) = anim;
    }

    float GetAnimItemAtActive(std::size_t index)
    {
        return animActiveItems.at(index);
    }
private:
    int *value = nullptr;
    std::vector<std::string> items;
    std::vector<float> animHoverItems;
    std::vector<float> animActiveItems;

    bool opened;
    float hoverAlpha = 0.0f;
    float activeAlpha = 0.0f;
};
