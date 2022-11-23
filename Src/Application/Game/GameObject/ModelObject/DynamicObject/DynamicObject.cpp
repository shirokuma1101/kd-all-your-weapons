#include "DynamicObject.h"

#include "Application/Game/Scene/GameScene/GameScene.h"

void DynamicObject::PreUpdate()
{
    m_isEquipping = false;
    m_selection = Selection::NoSelected;
}

void DynamicObject::Update(float delta_time)
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    m_weight = (*jm)[m_name]["expand"]["weight"];

    ModelObject::Update(delta_time);
    if (!mm->IsLoaded(m_name)) return;

    // Modelロード時のみ実行
    if (!m_isLoaded) {
        m_isLoaded = true;
        physx_helper::PutToSleep(m_pRigidActor);
    }

    //TODO: 当たり判定、destruction用のモデルロードと破壊判定時のモデル置き換え、physxに登録処理を追加、sand smokeを追加

    if (m_isEquipping) {
        m_pRigidActor->setGlobalPose(physx::PxTransform(physx_helper::ToPxMat44(m_transform.matrix)));
        physx_helper::PutToSleep(m_pRigidActor);
        m_isFirstImpulse = true;
    }
    else {
        if (physx_helper::IsSleeping(m_pRigidActor)) {
            if (m_isFirstImpulse) {
                physx_helper::AddForce(m_pRigidActor, m_force, physx::PxForceMode::eIMPULSE);
                m_isFirstImpulse = false;
            }
        }
        if (m_pRigidActor && m_pRigidActor->is<physx::PxRigidDynamic>()) {
            SetMatrix(physx_helper::ToMatrix(m_pRigidActor->getGlobalPose()));
        }
    }

    if (m_spRigidActorHolder) {
        m_spRigidActorHolder->Update();
    }
    
    // SceneがGameの場合はGameSceneにキャストする
    if (Application::Instance().GetGameSystem()->GetScene()->GetSceneType() != Scene::SceneType::Game) return;
    // GameSceneと確定しているのでstatic
    auto game_scene = std::static_pointer_cast<GameScene>(Application::WorkInstance().GetGameSystem()->GetScene());
    game_scene->GetEnemyObjects();
}

void DynamicObject::DrawOpaque()
{
    auto rim_light = DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().GetRimLightCB().Get();

    switch (m_selection) {
    case DynamicObject::Selection::NoSelected:
        rim_light->rimPower = 0.f;
        break;
    case DynamicObject::Selection::Equippable:
        rim_light->rimColor = { 0.f, 1.f, 0.f };
        rim_light->rimPower = 1.f;
        break;
    case DynamicObject::Selection::NotEquippable:
        rim_light->rimColor = { 1.f, 0.f, 0.f };
        rim_light->rimPower = 1.f;
        break;
    default:
        break;
    }

    DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().GetRimLightCB().Write();
    ModelObject::DrawOpaque();
    rim_light->rimPower = 0.f;
    DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().GetRimLightCB().Write(true);
}
