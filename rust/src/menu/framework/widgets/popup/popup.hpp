#pragma once

class CPopup : public CObject
{
public:
    CPopup()
    {
        SetOpened(false);
        SetOpenAlpha(0.0f);
    }

    void Render() override;

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

private:
    bool opened;
    float openAlpha;
};
