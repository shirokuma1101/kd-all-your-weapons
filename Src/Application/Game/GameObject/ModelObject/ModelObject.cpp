#include "ModelObject.h"

void ModelObject::Init()
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    // 同名のモデル名の場合があるのでフラグをリセットする
    mm->LoadedOnlyOnceReset(m_name);
    if (!mm->AsyncLoad(m_name)) return;

    // 行列計算
    m_transform = game_object_helper::ToTransform((*jm)[m_name]["transform"]);
    m_transform.Composition();

    // プリミティブな形状の当たり判定を作成
    CreatePrimitiveCollision();

    // 移動量を計算するためのクラスにセット
    SetRigidActorHolder(m_pRigidActor);
}

void ModelObject::Update(float delta_time)
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    // モデルのロード時
    if (!m_spModel && mm->IsLoadedOnlyOnce(m_name)) {
        // モデルを取得
        m_spModel = mm->CopyData(m_name);

        // モデルの当たり判定を作成
        CreateModelCollision();
    }
    // モデルのロード
    if (!mm->IsLoaded(m_name)) return;

    // 行列の計算
    if ((*jm)[m_name]["transform"]["fixed"]) {
        // 固定
        m_transform = game_object_helper::ToTransform((*jm)[m_name]["transform"]);
        m_transform.Composition();
        if (m_pRigidActor) {
            m_pRigidActor->setGlobalPose(physx::PxTransform(
                physx_helper::ToPxVec3(m_transform.position),
                physx_helper::ToPxQuat(Math::Quaternion::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation)))
            ));
        }
    }
    else {
        // 当たり判定
        m_transform.position.y += -constant::fG * delta_time;
        Collision();
    }
}

void ModelObject::DrawOpaque()
{
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    if (mm->IsLoaded(m_name) && m_spModel) {
        DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().DrawModel(*m_spModel, m_transform.matrix, { m_collisionNodeName });
    }
}

void ModelObject::CreatePrimitiveCollision()
{
    auto& pm = Application::Instance().GetGameSystem()->GetPhysXManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    if (!(*jm)[m_name].count("collision")) return;

    // PhysX
    if ((*jm)[m_name]["collision"].count("physx")) {
        if ((*jm)[m_name]["collision"]["physx"]["type"] == "static") {
            m_pRigidActor = pm->CreateStatic(m_transform.position, m_transform.rotation);
        }
        else if ((*jm)[m_name]["collision"]["physx"]["type"] == "dynamic") {
            m_pRigidActor = pm->CreateDynamic(m_transform.position, m_transform.rotation);
        }
        bool has_mesh = false;
        for (const auto& e : (*jm)[m_name]["collision"]["physx"]["shapes"]) {
            const auto& type       = e.items().begin().key();
            const auto& properties = e.items().begin().value();
            if (type == "sphere") {
                auto sphere   = pm->Sphere(properties["radius"]);
                auto position = properties.count("position") ? game_object_helper::ToVector3(properties["position"]) : Math::Vector3::Zero;
                physx_helper::AttachShape(&m_pRigidActor, &sphere, physx::PxTransform(physx_helper::ToPxVec3(position)));
            }
            else if (type == "capsule") {
                auto capsule  = pm->Capsule(properties["radius"], properties["half_height"]);
                auto position = properties.count("position") ? game_object_helper::ToVector3(properties["position"]) : Math::Vector3::Zero;
                auto rotation = properties.count("rotation") ? game_object_helper::ToVector3(properties["rotation"]) : Math::Vector3::Zero;
                physx_helper::AttachShape(&m_pRigidActor, &capsule, physx::PxTransform(physx_helper::ToPxVec3(position), physx_helper::ToPxQuat(Math::Quaternion::CreateFromYawPitchRoll(convert::ToRadians(rotation)))));
            }
            else if (type == "box") {
                auto box = pm->Box(game_object_helper::ToVector3(properties["half_extent"]));
                auto position = properties.count("position") ? game_object_helper::ToVector3(properties["position"]) : Math::Vector3::Zero;
                auto rotation = properties.count("rotation") ? game_object_helper::ToVector3(properties["rotation"]) : Math::Vector3::Zero;
                physx_helper::AttachShape(&m_pRigidActor, &box, physx::PxTransform(physx_helper::ToPxVec3(position), physx_helper::ToPxQuat(Math::Quaternion::CreateFromYawPitchRoll(convert::ToRadians(rotation)))));
            }
            else if (type == "mesh" || type == "convex") {
                has_mesh = true;
            }
        }
        if (!has_mesh) {
            pm->AddActor(m_pRigidActor);
        }
    }
    // Passive
    if ((*jm)[m_name]["collision"].count("passive")) {
        m_upDefaultCollider = std::make_unique<DefaultCollider>();
        for (const auto& e : (*jm)[m_name]["collision"]["passive"]) {
            const auto& type       = e.items().begin().key();
            const auto& properties = e.items().begin().value();
            if (type == "sphere") {
                auto dct = properties.count("default_collision_type") ? game_object_helper::ToDefaultCollisionType(properties["default_collision_type"]) : game_object_helper::DefaultCollisionTypeNone;
                auto center = properties.count("center") ? m_transform.position + game_object_helper::ToVector3(properties["center"]) : m_transform.position;
                auto sphere = collision::BoundingSphere(Math::Vector3::Zero, properties["radius"]);
                m_upDefaultCollider->AddCollisionShape(
                    std::make_shared<CollisionSphere<game_object_helper::DefaultCollisionType>>(dct, sphere)
                );
            }
        }
    }
}

void ModelObject::CreateModelCollision()
{
    auto& pm = Application::Instance().GetGameSystem()->GetPhysXManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    if (!(*jm)[m_name].count("collision")) return;

    // PhysX
    if ((*jm)[m_name]["collision"].count("physx")) {
        for (const auto& e : (*jm)[m_name]["collision"]["physx"]["shapes"]) {
            const auto& type       = e.items().begin().key();
            const auto& properties = e.items().begin().value();
            if (type == "convex") {
                if (properties.count("collision_node_name")) {
                    auto node = m_spModel->GetData()->FindNode(m_collisionNodeName = properties["collision_node_name"]);
                    if (!node || !node->m_spMesh) continue;
                    auto convex = pm->ConvexMesh(pm->ToPxConvexMesh(node->m_spMesh->GetVertexPositions()));
                    physx_helper::AttachShape(&m_pRigidActor, &convex);
                }
                else {
                    for (const auto& node : m_spModel->GetDataNodes()) {
                        if (!node.m_spMesh) continue;
                        auto convex = pm->ConvexMesh(pm->ToPxConvexMesh(node.m_spMesh->GetVertexPositions()));
                        physx_helper::AttachShape(&m_pRigidActor, &convex);
                    }
                }
            }
            else if (type == "mesh") {
                if (properties.count("collision_node_name")) {
                    auto node = m_spModel->GetData()->FindNode(m_collisionNodeName = properties["collision_node_name"]);
                    if (!node || !node->m_spMesh) continue;
                    auto transform = game_object_helper::ToTransform((*jm)[m_name]["transform"]);
                    auto mesh = pm->TriangleMesh(
                        pm->ToPxTriangleMesh(node->m_spMesh->GetVertexPositions(), node->m_spMesh->GetFaces()),
                        transform.scale
                    );
                    physx_helper::AttachShape(&m_pRigidActor, &mesh);
                }
                else {
                    for (const auto& node : m_spModel->GetDataNodes()) {
                        if (!node.m_spMesh) continue;
                        auto transform = game_object_helper::ToTransform((*jm)[m_name]["transform"]);
                        auto mesh = pm->TriangleMesh(
                            pm->ToPxTriangleMesh(node.m_spMesh->GetVertexPositions(), node.m_spMesh->GetFaces()),
                            transform.scale
                        );
                        physx_helper::AttachShape(&m_pRigidActor, &mesh);
                    }
                }
            }
        }
        pm->AddActor(m_pRigidActor);
    }
    // Passive
    if ((*jm)[m_name]["collision"].count("passive")) {
        for (const auto& e : (*jm)[m_name]["collision"]["passive"]) {
            const auto& type       = e.items().begin().key();
            const auto& properties = e.items().begin().value();
            if (type == "mesh") {
                auto dct = properties.count("default_collision_type") ? game_object_helper::ToDefaultCollisionType(properties["default_collision_type"]) : game_object_helper::DefaultCollisionTypeNone;
                m_collisionNodeName = properties.count("collision_node_name") ? properties["collision_node_name"].get<std::string>() : "";
                m_upDefaultCollider->AddCollisionShape(
                    std::make_shared<CollisionModel<game_object_helper::DefaultCollisionType>>(dct, m_spModel, m_collisionNodeName)
                );
            }
        }
    }
}

bool ModelObject::Collision()
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    bool is_ground = false;

    if (!(*jm)[m_name].count("collision") || !(*jm)[m_name]["collision"].count("active")) {
        return false;
    }

    std::list<collision::Result> results;
    auto rays = ToRays((*jm)[m_name]["collision"]["active"]);
    auto spheres = ToSpheres((*jm)[m_name]["collision"]["active"]);

    for (const auto& e : Application::Instance().GetGameSystem()->GetScene()->GetGameObjects()) {
        const auto& collider = e->GetCollider();
        if (!collider) continue;
        // 地面との判定 (光線)
        for (const auto& ray : rays) {
            if (collider->Intersects(game_object_helper::DefaultCollisionTypeBump, e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    m_transform.position += result->direction * result->depth;
                    is_ground = true;
                }
                results.clear();
            }
        }
        // 壁との判定 (球)
        for (const auto& sphere : spheres) {
            if (collider->Intersects(game_object_helper::DefaultCollisionTypeBump, e->GetTransform().matrix, sphere, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    m_transform.position += result->direction * result->depth;
                }
                results.clear();
            }
        }
    }

    // 地面がマイナスになることはない
    if (m_transform.position.y < 0) {
        is_ground = true;
        m_transform.position.y = 0.f;
    }

    return is_ground;
}
