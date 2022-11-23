#pragma once

class GravityGun
{
public:

private:
};

// Gravity Gun
//if (mm->IsLoaded("gravity_gun")) {
//    auto rotation_matrix = Math::Matrix::CreateFromYawPitchRoll(
//        convert::ToRadians(m_angle + 180.f),
//        convert::ToRadians(0.f),
//        convert::ToRadians(m_transform.rotation.z)
//    );
//    auto translation_matrix = Math::Matrix::CreateTranslation(
//        m_transform.position.x,
//        m_transform.position.y - (*jm)[m_name]["expand"]["status"]["height"].get<float>(),
//        m_transform.position.z
//    );

//    static auto spModel = mm->CopyData("gravity_gun"); // MN: ここだけ

//    DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().DrawModel(
//        *spModel,
//        Math::Matrix::CreateScale(2.0f)
//        * Math::Matrix::CreateRotationY(convert::ToRadians(90.f))
//        * Math::Matrix::CreateRotationX(convert::ToRadians(-30.f))
//        * Math::Matrix::CreateTranslation(0.f, -0.05f, -0.1f)
//        * m_gravityGunMatrix
//        * rotation_matrix
//        * translation_matrix
//    );
//}

//auto right_hand = m_spModel->FindWorkNode("mixamorig:RightHandMiddle1");
//if (right_hand) {
//    m_gravityGunMatrix = Math::Matrix::CreateScale(100.f) * right_hand->m_worldTransform;
//}
