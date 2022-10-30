#pragma once

#include "System/GameObject/GameObject.h"

class UIObject : public GameObject
{

protected:
    
    static Math::Vector2 GetPositionFromPercentage(float left_based_x_percentage, float top_based_y_percentage) {
        // 画面中央を(0, 0)とするためにViewportから補正値を計算
        auto v = DirectX11System::Instance().GetViewport();
        return { v.Width * convert::ToUndoPercent(left_based_x_percentage) - convert::ToHalf(v.Width), -v.Height * convert::ToUndoPercent(top_based_y_percentage) + convert::ToHalf(v.Height) };
    }

    static float GetSizePercentageForHDWidthBased() {
        auto v = DirectX11System::Instance().GetViewport();
        return v.Width / Window::HD.first;
    }
    
};
