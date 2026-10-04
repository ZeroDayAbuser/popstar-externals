#pragma once

class CSeperator : public CObject
{
public:
    CSeperator(const std::string &header = "")
    {
        SetName(header);
    }

    void Render() override;
};
