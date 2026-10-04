#pragma once

class CConfirmationButton : public CObject
{
public:
    CConfirmationButton(std::string name, std::function<void()> onAcceptCallback, std::function<void()> onDeclineCallback,
                        std::string confirmationMessage, std::string tooltip = "")
    {
        SetName(name);
        SetAcceptCallback(onAcceptCallback);
        SetDeclineCallback(onDeclineCallback);
        SetConfirmationMessage(confirmationMessage);
        SetTooltip(tooltip);

        SetOpened(false);

        SetHoverAlpha(0.0f);
        SetActiveAlpha(0.0f);
        SetClickedAlpha(0.0f);

        SetAcceptHoverAlpha(0.0f);
        SetAcceptActiveAlpha(0.0f);
        SetDeclineHoverAlpha(0.0f);
        SetDeclineActiveAlpha(0.0f);
    }

    void Render() override;

    void SetAcceptCallback(const std::function<void()> &callback)
    {
        this->acceptCallback = callback;
    }

    void SetDeclineCallback(const std::function<void()> &callback)
    {
        this->declineCallback = callback;
    }

    void TriggerAcceptCallback()
    {
        if (acceptCallback)
        {
            acceptCallback();
        }
    }

    void TriggerDeclineCallback()
    {
        if (declineCallback)
        {
            declineCallback();
        }
    }

    std::string GetConfirmationMessage() const
    {
        return confirmationMessage;
    }

    void SetConfirmationMessage(std::string confirmationMessage)
    {
        this->confirmationMessage = confirmationMessage;
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

    float GetClickedAlpha() const
    {
        return clickedAlpha;
    }

    void SetClickedAlpha(float clickedAlpha)
    {
        this->clickedAlpha = clickedAlpha;
    }

    float GetActiveAlpha() const
    {
        return activeAlpha;
    }

    void SetActiveAlpha(float activeAlpha)
    {
        this->activeAlpha = activeAlpha;
    }

    float GetAcceptHoverAlpha() const
    {
        return acceptHoverAlpha;
    }

    void SetAcceptHoverAlpha(float acceptHoverAlpha)
    {
        this->acceptHoverAlpha = acceptHoverAlpha;
    }

    float GetAcceptActiveAlpha() const
    {
        return acceptActiveAlpha;
    }

    void SetAcceptActiveAlpha(float acceptActiveAlpha)
    {
        this->acceptActiveAlpha = acceptActiveAlpha;
    }

    float GetDeclineHoverAlpha() const
    {
        return declineHoverAlpha;
    }

    void SetDeclineHoverAlpha(float declineHoverAlpha)
    {
        this->declineHoverAlpha = declineHoverAlpha;
    }

    float GetDeclineActiveAlpha() const
    {
        return declineActiveAlpha;
    }

    void SetDeclineActiveAlpha(float declineActiveAlpha)
    {
        this->declineActiveAlpha = declineActiveAlpha;
    }

private:
    std::function<void()> acceptCallback;
    std::function<void()> declineCallback;

    std::string confirmationMessage;

    bool opened;

    float acceptHoverAlpha;
    float acceptActiveAlpha;

    float declineHoverAlpha;
    float declineActiveAlpha;

    float hoverAlpha;
    float clickedAlpha;
    float activeAlpha;
};
