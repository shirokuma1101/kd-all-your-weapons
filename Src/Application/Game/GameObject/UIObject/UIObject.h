#pragma once

#include "System/GameObject/GameObject.h"

class UIObject : public GameObject
{

protected:
    
    virtual Math::Vector2 GetPositionFromPercentage(float left_based_x_percentage, float top_based_y_percentage) const {
        // 画面中央を(0, 0)とするためにViewportから補正値を計算
        auto v = DirectX11System::Instance().GetViewport();
        return { v.Width * convert::ToUndoPercent(left_based_x_percentage) - convert::ToHalf(v.Width), -v.Height * convert::ToUndoPercent(top_based_y_percentage) + convert::ToHalf(v.Height) };
    }

    virtual float GetSizePercentageForHDWidthBased() const {
        auto v = DirectX11System::Instance().GetViewport();
        return v.Width / Window::HD.first;
    }
    
};
