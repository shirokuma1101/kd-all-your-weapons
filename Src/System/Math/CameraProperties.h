#pragma once

#include "ExternalDependencies/Math/Camera.h"

class CameraProperties
{
public:

    CameraProperties() = default;
    CameraProperties(const Math::Matrix& camera_matrix, float fov, float aspect, float near_clipping_distance, float far_clipping_distance)
        : m_camera(camera_matrix, fov, aspect, near_clipping_distance, far_clipping_distance)
    {}

    void SetMatrix(const Math::Matrix& camera_matrix) {
        m_camera.SetMatrix(camera_matrix);
    }

    void SetProjection(float fov, float aspect, float near_clipping_distance, float far_clipping_distance) {
        m_camera.SetProjection(fov, aspect, near_clipping_distance, far_clipping_distance);
    }

    void SetToShader() {
        auto camera_cb = DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Get();
        camera_cb->view       = m_camera.viewMatrix;
        camera_cb->projection = m_camera.projection.matrix;
        camera_cb->position   = m_camera.cameraMatrix.Translation();

        DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Write();
    }

private:

    Camera m_camera;

};
