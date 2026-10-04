#pragma once

class CContainer : public CObject
{
public:
    CContainer(std::string name, bool forceContentHeight = false, float contentHeightFraction = 0.0f)
    {
        SetName(name);
        SetScrollState(false);
        SetScrollOffset(0.0f);
        SetScrollOffsetLerp(0.0f);
        SetForcedContentHeight(forceContentHeight);
        SetForcedHeightFraction(contentHeightFraction);
    }

    void Render() override;

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

    bool GetForcedContentHeight() const
    {
        return forcedContentHeight;
    }

    void SetForcedContentHeight(bool forcedContentHeight)
    {
        this->forcedContentHeight = forcedContentHeight;
    }

    float GetForcedHeightFraction() const
    {
        return forcedHeightFraction;
    }

    void SetForcedHeightFraction(float forcedHeightFraction)
    {
        this->forcedHeightFraction = forcedHeightFraction;
    }
private:
    bool forcedContentHeight;
    float forcedHeightFraction;

    float scrollOffset;
    float scrollOffsetLerp;
    bool scrollState;
};
