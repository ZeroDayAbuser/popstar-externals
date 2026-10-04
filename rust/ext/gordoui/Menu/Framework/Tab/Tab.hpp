#pragma once

class CTab : public CBaseWindow
{
public:
    CTab(const std::string &header)
    {
        SetName(header);
        tabHash = std::hash<std::string>()(header);
        SetAnim(1.0f);
        SetTabAlpha(0.0f);
        SetScrollState(false);
        SetScrollOffset(0.0f);
        SetScrollOffsetLerp(0.0f);
        subtabHash = 0;
    }

    void Render() override;

    std::size_t GetTabHash() const
    {
        return tabHash;
    }

    bool ShouldShow()
    {
        return GetParent<CBaseWindow>()->GetTabHash() == GetTabHash();
    }

    float GetAnim() const
    {
        return anim;
    }

    void SetAnim(float anim)
    {
        this->anim = anim;
    }

    float GetTabAlpha() const
    {
        return tabAlpha;
    }

    void SetTabAlpha(float tabAlpha)
    {
        this->tabAlpha = tabAlpha;
    }

    bool GetScrollState() const
    {
        return scrollState;
    }

    void SetScrollState(bool scrollState)
    {
        this->scrollState = scrollState;
    }

    float GetScrollOffset() const
    {
        return scrollOffset;
    }

    void SetScrollOffset(float scrollOffset)
    {
        this->scrollOffset = scrollOffset;
    }

    float GetScrollOffsetLerp() const
    {
        return scrollOffsetLerp;
    }

    void SetScrollOffsetLerp(float scrollOffsetLerp)
    {
        this->scrollOffsetLerp = scrollOffsetLerp;
    }

    std::size_t GetSubtabHash() const
    {
        return subtabHash;
    }

    void SetSubtabHash(std::size_t hash)
    {
        this->subtabHash = hash;
    }

private:
    std::size_t tabHash;
    std::size_t subtabHash;
    bool hasSubtabs;
    float anim;
    float tabAlpha;

    float scrollOffset;
    float scrollOffsetLerp;
    bool scrollState;
};
