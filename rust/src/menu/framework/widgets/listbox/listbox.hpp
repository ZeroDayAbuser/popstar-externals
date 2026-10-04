#pragma once

class CListbox : public CObject
{
public:
    CListbox(std::string name, int *value, std::vector<std::string> items, int maxItems = 10, std::string tooltip = "")
    {
        SetName(name);
        SetValue(value);
        SetItems(items);
        SetHoverAlpha(0.0f);
        SetMaxItems(maxItems);
        SetTooltip(tooltip);
        SetScroll(0.0f);
        SetScrollInterp(0.0f);

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

        animHoverItems.resize(items.size());
        animActiveItems.resize(items.size());
    }

    int GetMaxItems() const
    {
        return maxItems;
    }

    void SetMaxItems(int maxItems)
    {
        this->maxItems = maxItems;
    }

    float GetHoverAlpha() const
    {
        return hoverAlpha;
    }

    void SetHoverAlpha(float hoverAlpha)
    {
        this->hoverAlpha = hoverAlpha;
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

    float GetScroll() const
    {
        return scroll;
    }

    void SetScroll(float scroll)
    {
        this->scroll = scroll;
    }

    float GetScrollInterp() const
    {
        return scrollInterp;
    }

    void SetScrollInterp(float scrollInterp)
    {
        this->scrollInterp = scrollInterp;
    }

    std::string GetFilter() const
    {
        return filter();
    }

    void SetFilter(std::function<std::string()> filter)
    {
        this->filter = filter;
    }

private:
    int *value = nullptr;
    int maxItems;
    std::vector<std::string> items;
    std::vector<float> animHoverItems;
    std::vector<float> animActiveItems;

    float scroll;
    float scrollInterp;

    std::function<std::string()> filter = []() { return ""; };


    float hoverAlpha = 0.0f;
};
