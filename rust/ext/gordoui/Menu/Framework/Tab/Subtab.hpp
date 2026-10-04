#pragma once

class CSubtab : public CObject
{
public:
    CSubtab(std::string name, std::string hint = "")
    {
        SetName(name);
        SetHint(hint);
        subtabHash = std::hash<std::string>()(name);
    }

    void Render() override;

    bool ShouldShow()
    {
        return GetParent<CTab>()->GetSubtabHash() == GetSubtabHash();
    }

    std::size_t GetSubtabHash() const
    {
        return subtabHash;
    }

    float GetSubtabAlpha() const
    {
        return subtabAlpha;
    }

    void SetSubtabAlpha(float Alpha)
    {
        subtabAlpha = Alpha;
    }

    std::string GetHint() const
    {
        return mHint;
    }

    void SetHint(std::string hint)
    {
        mHint = hint;
    }
private:
    std::size_t subtabHash = 0;

    std::string mHint = "";

    float subtabAlpha = 0.0f;
};
