#pragma once

class CBaseObject
{
public:
    virtual ~CBaseObject() = default;

    std::string GetName() const
    {
        return name;
    }

    void SetName(const std::string &name)
    {
        this->name = name;
    }

    Vector2 GetPosition() const
    {
        return position;
    }

    void SetPosition(const Vector2 &position)
    {
        this->position = position;
    }

    Vector2 GetSize() const
    {
        return size;
    }

    void SetSize(const Vector2 &size)
    {
        this->size = size;
    }

    bool GetVisibleCondition() const
    {
        return visibleCondition();
    }

    void SetVisibleCondition(std::function<bool()> visibleCondition)
    {
        this->visibleCondition = visibleCondition;
    }

    std::string GetTooltip() const
    {
        return tooltip;
    }

    void SetTooltip(std::string tooltip)
    {
        this->tooltip = tooltip;
    }

    float GetTooltipAlpha() const
    {
        return tooltipAlpha;
    }

    void SetTooltipAlpha(float tooltipAlpha)
    {
        this->tooltipAlpha = tooltipAlpha;
    }

    float GetTooltipTime() const
    {
        return tooltipTime;
    }

    void SetTooltipTime(float tooltipTime)
    {
        this->tooltipTime = tooltipTime;
    }

    Vector2 GetTooltipPosition() const
    {
        return tooltipPosition;
    }

    void SetTooltipPosition(Vector2 tooltipPosition)
    {
        this->tooltipPosition = tooltipPosition;
    }

    bool GetHidden() const
    {
        return hidden;
    }

    void SetHidden(bool hidden)
    {
        this->hidden = hidden;
    }

    float GetAlpha() const
    {
        return alpha;
    }

    void SetAlpha(float alpha)
    {
        this->alpha = alpha;
    }

    bool Intersects(CBaseObject *obj)
    {
        return Rect{this->GetPosition(), this->GetSize()}.Intersects(obj->GetPosition(), obj->GetSize());
    }

    float GetContainedAlpha() const
    {
        return containedAlpha - 0.1f;
    }

    void SetContainedAlpha(float containedAlpha)
    {
        this->containedAlpha = containedAlpha;
    }

    bool GetOverrideSize() const
    {
        return overrideSize;
    }

    void SetOverrideSize(bool overrideSize)
    {
        this->overrideSize = overrideSize;
    }
private:
    std::string name;
    Vector2 position;
    Vector2 size;
    std::function<bool()> visibleCondition = []() { return true; };
    std::string tooltip;
    float tooltipTime;
    float tooltipAlpha;
    Vector2 tooltipPosition;

    float containedAlpha;

    bool hidden = false;
    float alpha = 1.0f;

    bool overrideSize = false;
};

class CObject : public CBaseObject
{
public:
    virtual void Render() = 0;

    void RenderToolTip();

    void UpdateState();

    template <typename T = CObject>
    T *GetParent()
    {
        return reinterpret_cast<T *>(parent);
    }

    void SetParent(CObject *parent)
    {
        this->parent = parent;
    }

    template <typename T, typename... Args>
    T *AddObject(Args &&...args)
    {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        p->SetParent(this);
        p->SetPosition(this->GetPosition());

        auto raw = p.get();
        children.emplace_back(std::move(p));
        return raw;
    }

    std::vector<CObject *> GetChildren() const
    {
        std::vector<CObject *> result;
        for (const auto &child : children)
        {
            result.push_back(child.get());
        }

        return result;
    }

    void RemoveChild(CObject *child)
    {
        children.erase(std::remove_if(children.begin(), children.end(),
                                         [child](const std::unique_ptr<CObject> &ptr) { return ptr.get() == child; }),
                          children.end());
    }

    void ClearChildren()
    {
        children.clear();
    }
private:
    CObject *parent = nullptr;
    std::vector<std::unique_ptr<CObject>> children;
};
