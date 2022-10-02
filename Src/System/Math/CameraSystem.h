#pragma once

#include "ExternalDependencies/Math/Camera.h"

class CameraSystem
{
public:

    CameraSystem() = default;
    CameraSystem(const Math::Matrix& camera_matrix, float fov, float aspect, float near_clipping_distance, float far_clipping_distance)
        : m_camera(camera_matrix, fov, aspect, near_clipping_distance, far_clipping_distance)
    {}

    void SetMatrix(const Math::Matrix& camera_matrix) {
        m_camera.SetMatrix(camera_matrix);
    }

    void SetProjection(float fov, float aspect, float near_clipping_distance, float far_clipping_distance) {
        m_camera.SetProjection(fov, aspect, near_clipping_distance, far_clipping_distance);
    }

    void SetToShader() {
        auto cb_camera = DirectX11System::Instance().GetShaderManager()->m_cb7_Camera.Get();
        
        // カメラ座標をセット
        cb_camera->CamPos = m_camera.cameraMatrix.Translation();
        // ビュー行列をセット
        cb_camera->mView = m_camera.viewMatrix;
        // 射影行列をセット
        cb_camera->mProj = m_camera.projection.matrix;
        cb_camera->mProjInv = cb_camera->mProj.Invert();

        // カメラ情報(ビュー行列、射影行列)を、シェーダの定数バッファへ書き込む
        DirectX11System::Instance().GetShaderManager()->m_cb7_Camera.Write();
    }

private:

    Camera m_camera;

};
