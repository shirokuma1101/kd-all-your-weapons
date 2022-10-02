#pragma once

#include "ExternalDependencies/Effekseer/EffekseerManager.h"
#include "ExternalDependencies/Input/InputManager.h"
#include "ExternalDependencies/PhysX/PhysXManager.h"

#include "System/Asset/AssetManager/AssetManager.h"
#include "System/GameObject/GameObject.h"

class ModelObject : public GameObject
{
public:

    ModelObject(std::string_view name)
        : GameObject()
        , m_name(name)
    {}

    virtual void Init() override {
        auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
        auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

        if (!mm->AsyncLoad(m_name)) return;

        // Create transform
        m_transform = ToTransform((*jm)[m_name]["transform"]);
        m_transform.Composition();

        CreatePrimitiveCollision();

        SetRigidActorHolder(m_pRigidActor);
    }
    
    virtual void Update(float delta_time) override {
        
        auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
        auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

        // モデルのロード
        if (mm->IsLoadedOnlyOnce(m_name)) {
            m_spModel = mm->CopyData(m_name);
            CreateModelCollision();
        }
        if (!mm->IsLoaded(m_name)) return;

        Collision();

        // 行列
        if ((*jm)[m_name]["transform"]["fixed"]) {
            m_transform = ToTransform((*jm)[m_name]["transform"]);
            m_transform.Composition();
            if (m_pRigidActor) {
                m_pRigidActor->setGlobalPose(physx::PxTransform(
                    physx_helper::ToPxVec3(m_transform.position),
                    physx_helper::ToPxQuat(Math::Quaternion::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation)))
                ));
            }
        }
    }

    virtual void DrawOpaque() override {
        auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

        if (mm->IsLoaded(m_name) && m_spModel) {
            DirectX11System::Instance().GetShaderManager()->m_standardShader.DrawModel(*m_spModel, m_transform.matrix);
        }
    }

    virtual bool GetIsDynamic() const final {
        return m_pRigidActor && m_pRigidActor->is<physx::PxRigidDynamic>();
    }

    virtual void SetModel(std::shared_ptr<KdModelWork> model) noexcept final {
        m_spModel = model;
    }

    virtual void SetRigidActorHolder(physx::PxRigidActor* actor) final {
        if (!actor) return;
        m_spRigidActorHolder = std::make_shared<physx_helper::RigidActorHolder>(actor);
    }
    
protected:

    void CreatePrimitiveCollision() {
        auto& pm = Application::Instance().GetGameSystem()->GetPhysXManager();
        auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

        if (!(*jm)[m_name].count("collision")) return;
        if ((*jm)[m_name]["collision"].count("physx")) {

            if ((*jm)[m_name]["collision"]["physx"]["type"] == "static") {
                m_pRigidActor = pm->CreateStatic(m_transform.position, m_transform.rotation);
            }
            else if ((*jm)[m_name]["collision"]["physx"]["type"] == "dynamic") {
                m_pRigidActor = pm->CreateDynamic(m_transform.position, m_transform.rotation);
            }
            else {
                assert::RaiseAssert("");
            }

            bool has_mesh = false;
            for (const auto& e : (*jm)[m_name]["collision"]["physx"]["shapes"]) {
                const auto& first_key = e.items().begin().key();
                const auto& first_value = e.items().begin().value();
                if (first_key == "sphere") {
                    auto sphere = pm->Sphere(first_value["radius"]);
                    physx_helper::AttachShape(&m_pRigidActor, &sphere);
                }
                else if (first_key == "capsule") {
                    auto capsule = pm->Capsule(first_value["radius"], first_value["half_height"]);
                    physx_helper::AttachShape(&m_pRigidActor, &capsule);
                }
                else if (first_key == "box") {
                    auto box = pm->Box(ToVector3(first_value["half_extent"]));
                    physx_helper::AttachShape(&m_pRigidActor, &box);
                }
                else if (first_key == "mesh" || first_key == "convex") {
                    has_mesh = true;
                }
            }
            if (!has_mesh) {
                pm->AddActor(m_pRigidActor);
            }
        }
        if ((*jm)[m_name]["collision"].count("passive")) {
            m_upDefaultCollider = std::make_unique<Collider<DefaultCollisionType>>();
            for (const auto& e : (*jm)[m_name]["collision"]["passive"]) {
                const auto& first_key = e.items().begin().key();
                const auto& first_value = e.items().begin().value();
                if (first_key == "sphere") {
                    const auto sphere = ToSpheres((*jm)[m_name]["collision"]["passive"]);
                    m_upDefaultCollider->AddCollisionShape(std::make_shared<CollisionSphere<DefaultCollisionType>>(DefaultCollisionType::Bump, *sphere.begin()));
                }
                else if (first_key == "mesh") {
                    std::string node_name;
                    if (first_value.count("node_name")) {
                        node_name = first_value;
                    }
                    m_upDefaultCollider->AddCollisionShape(std::make_shared<CollisionModel<DefaultCollisionType>>(DefaultCollisionType::Bump, m_spModel, node_name));
                }
            }
        }
    }

    void CreateModelCollision() {
        auto& pm = Application::Instance().GetGameSystem()->GetPhysXManager();
        auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

        if (!(*jm)[m_name].count("collision")) return;
        if ((*jm)[m_name]["collision"].count("physx")) {
            for (const auto& e : (*jm)[m_name]["collision"]["physx"]["shapes"]) {
                const auto& first_key = e.items().begin().key();
                const auto& first_value = e.items().begin().value();
                if (first_key == "convex") {
                    if (first_value.count("node_name")) {
                        auto node = m_spModel->GetData()->FindNode(first_value["node_name"]);
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
                else if (first_key == "mesh") {
                    if (first_value.count("node_name")) {
                        auto node = m_spModel->GetData()->FindNode(first_value["node_name"]);
                        if (!node || !node->m_spMesh) continue;
                        auto transform = ToTransform((*jm)[m_name]["transform"]);
                        auto mesh = pm->TriangleMesh(
                            pm->ToPxTriangleMesh(node->m_spMesh->GetVertexPositions(), node->m_spMesh->GetFaces()),
                            transform.scale
                        );
                        physx_helper::AttachShape(&m_pRigidActor, &mesh);
                    }
                    else {
                        for (const auto& node : m_spModel->GetDataNodes()) {
                            if (!node.m_spMesh) continue;
                            auto transform = ToTransform((*jm)[m_name]["transform"]);
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
        if ((*jm)[m_name]["collision"].count("passive")) {
            m_upDefaultCollider = std::make_unique<Collider<DefaultCollisionType>>();
            for (const auto& e : (*jm)[m_name]["collision"]["passive"]) {
                const auto& first_key = e.items().begin().key();
                const auto& first_value = e.items().begin().value();
                if (first_key == "mesh") {
                    std::string node_name;
                    if (first_value.count("node_name")) {
                        node_name = first_value;
                    }
                    m_upDefaultCollider->AddCollisionShape(std::make_shared<CollisionModel<DefaultCollisionType>>(DefaultCollisionType::Bump, m_spModel, node_name));
                }
            }
        }
    }

    void Collision() {
        auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

        if (!(*jm)[m_name].count("collision")) return;
        if (!(*jm)[m_name]["collision"].count("active")) return;

        std::list<collision::Result> ray_results;
        std::list<collision::Result> results;
        auto rays    = ToRays((*jm)[m_name]["collision"]["active"]);
        auto spheres = ToSpheres((*jm)[m_name]["collision"]["active"]);
        //for (const auto& e : Application::Instance().GetGameSystem()->GetGameObjects()) {
        //	const auto& collider = e->GetCollider();
        //	if (!collider) continue;
        //	// 光線
        //	for (const auto& ray : rays) {
        //		if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, ray, &results) && results.size()) {
        //			auto result = collision::GetNearest(results);
        //			m_transform.position = result.position;
        //			m_transform.position.y += ray.range;
        //		}
        //		results.clear();
        //	}
        //	// 球
        //	for (const auto& sphere : spheres) {
        //		if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, sphere, &results) && results.size()) {
        //			auto result = collision::GetNearest(results);
        //			m_transform.position += result.position;
        //		}
        //		results.clear();
        //	}
        //}
        // 当たり判定
        for (const auto& e : Application::Instance().GetGameSystem()->GetGameObjects()) {
            const auto& collider = e->GetCollider();
            if (!collider) continue;
            // 地面との判定 (光線)
            for (const auto& ray : rays) {
                if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, ray, &ray_results)) {
                }
            }
            // 壁との判定 (球)
            for (const auto& sphere : spheres) {
                if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, sphere, &results)) {
                    if (auto result = collision::GetNearest(results); result) {
                        m_transform.position += result->position;
                    }
                }
                results.clear();
            }
        }

        if (auto result = collision::GetFarthest(ray_results); result) {
            if (result->overlap) {
                m_transform.position = result->position;
            }
        }

    }

    const std::string                               m_name;
    std::shared_ptr<KdModelWork>                    m_spModel            = nullptr;
    physx::PxRigidActor*                            m_pRigidActor        = nullptr;
    std::shared_ptr<physx_helper::RigidActorHolder> m_spRigidActorHolder = nullptr;

};
