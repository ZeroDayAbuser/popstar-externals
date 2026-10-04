#pragma once

namespace UI
{
    template <typename T>
    T *GetDesc(CObject *parent)
    {
        T *desc = nullptr;
        CObject *current = parent;

        while (current != nullptr)
        {
            T *nextParent = current->GetParent<T>();

            if (nextParent != nullptr)
            {
                desc = nextParent;
                current = reinterpret_cast<CObject *>(nextParent);
            }
            else
            {
                break;
            }
        }

        return desc;
    }

    bool InBounds(CObject *parent);

    inline bool BeingBlocked(CObject* parent)
    {
        CPopup *p = dynamic_cast<CPopup *>(parent);
        if (p)
            return false;

        if (!InBounds(parent))
            return true;

        auto parentWindow = GetDesc<CBaseWindow>(parent);
        if (!parentWindow)
            return false;

        return parentWindow->HasBlockingObject();
    }

    inline void SetBlockingObject(CObject* blockingObject, CObject* parent)
    {
        auto parentWindow = GetDesc<CBaseWindow>(parent);
        if (!parentWindow)
            return;

        parentWindow->SetBlockingObject(blockingObject);
    }

    inline void RenderAllTooltips(CObject *root)
    {
        if (!root)
            return;

        root->RenderToolTip();

        for (auto child : root->GetChildren())
        {
            RenderAllTooltips(child);
        }
    }

    Vector2 ClampToScreen(const Vector2 &position, const Vector2 &size);

    std::string WrapText(const std::string &text, float maxWidth);

    inline std::string ToLower(std::string input)
    {
        std::string loweredString;
        for (auto c : input)
            loweredString.push_back(std::tolower(c));

        return loweredString;
    }
}
