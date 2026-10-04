#pragma once

class CBaseWindow : public CObject
{
public:
    void SetTabHash(std::size_t tabHash)
    {
        this->tabHash = tabHash;
    }

    std::size_t GetTabHash() const
    {
        return tabHash;
    }

    bool HasBlockingObject() const
    {
        return blockingObject != nullptr;
    }

    void SetBlockingObject(CObject *o)
    {
        blockingObject = o;
    }

    void ClearBlockingObject()
    {
        blockingObject = nullptr;
    }

    CObject *GetBlockingObject()
    {
        return blockingObject;
    }
private:
    std::size_t tabHash = 0;
    CObject *blockingObject;
};

class CWindow : public CBaseWindow
{
public:
    CWindow(std::string name, Vector2 position, Vector2 size, std::string tld = "")
    {
        SetName(name);
        SetPosition(position);
        SetSize(size);
        SetTld(tld);
        SetBlockingObject(nullptr);
        SetOpened(true);
        SetAlpha(1.0f);
        SetDragging(false);
        SetDragOffset({0.0f, 0.0f});

        SetKey(VK_INSERT);
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

    float GetWindowAlpha() const
    {
        return windowAlpha;
    }

    void SetWindowAlpha(float windowAlpha)
    {
        this->windowAlpha = windowAlpha;
    }

    bool GetDragging() const
    {
        return dragging;
    }

    void SetDragging(bool dragging)
    {
        this->dragging = dragging;
    }

    Vector2 GetDragOffset() const
    {
        return dragOffset;
    }

    void SetDragOffset(const Vector2 &dragOffset)
    {
        this->dragOffset = dragOffset;
    }

    std::string GetTld() const
    {
        return tld;
    }

    void SetTld(std::string tld)
    {
        this->tld = tld;
    }

    int GetKey() const
    {
        return key;
    }

    void SetKey(int key)
    {
        this->key = key;
    }

private:
    std::string tld;

    bool opened;
    int key;
    float windowAlpha;

    bool dragging;
    Vector2 dragOffset;
};
