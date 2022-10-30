#pragma once

#include "ExternalDependencies/Asset/Json/JsonData.h"
#include "ExternalDependencies/ImGui/ImGuiHelper.h"
#include "ExternalDependencies/Math/Collider.h"
#define TRANSFORM_ROTATION_USE_EULER
#include "ExternalDependencies/Math/Transform.h"

#include "System/Math/CameraProperties.h"
#include "System/Math/Collision.h"

enum DefaultCollisionType {
    None   = 0,
    Bump   = 1 << 0,
    Attack = 1 << 1,
    Road   = 1 << 2,
};

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:

    GameObject()
        : m_isObjectAlive(true)
        , m_isAlive(true)
    {}
    virtual ~GameObject() {}


    /**************************************************
    * 処理
    **************************************************/

    /* 初期化 */
    virtual void Init() {}

    /* 前処理 */
    virtual void PreUpdate() {}
    /* 処理 */
    virtual void Update(float delta_time) {
        delta_time; // C4100 'identifier' : unreferenced formal parameter
    }

    /* 透明描画 */
    virtual void DrawTransparent() {}
    /* 不透明描画 */
    virtual void DrawOpaque() {}
    /* 2D描画 */
    virtual void DrawSprite() {}
    /* フォント描画 */
    virtual void DrawFont() {}

    /* ImGui処理 */
    virtual void ImGuiUpdate() {}


    /**************************************************
    * アクセサ
    **************************************************/

    /* オブジェクト生死 */
    virtual bool IsObjectAlive() const noexcept final {
        return m_isObjectAlive;
    }
    /* 生死 */
    virtual bool IsAlive() const noexcept final {
        return m_isAlive;
    }
    /* Transform取得 */
    virtual const Transform& GetTransform() const noexcept final {
        return m_transform;
    }
    /* Matrix取得 */
    virtual const DirectX::SimpleMath::Matrix& GetMatrix() const noexcept {
        return m_transform.matrix;
    }
    /* DefaultCollider取得 */
    virtual std::unique_ptr<Collider<DefaultCollisionType>>& GetCollider() noexcept final {
        return m_upDefaultCollider;
    }

    /* 行列を設定 */
    virtual void SetMatrix(const Math::Matrix& mat) noexcept final {
        m_transform.matrix = mat;
    }


    /**************************************************
    * 変換
    **************************************************/
    
    /* jsonからRay情報に変換 */
    virtual std::list<collision::Ray> ToRays(const JsonData::Json& json) const final {
        std::list<collision::Ray> rays;
        collision::Ray ray;
        for (const auto& e : json) {
            const auto& first_key = e.items().begin().key();
            const auto& first_value = e.items().begin().value();
            if (first_key != "ray") continue;
            ray.position  = first_value.count("position") ? m_transform.position + ToVector3(first_value["position"]) : m_transform.position;
            ray.direction = ToVector3(first_value["direction"]);
            ray.range     = first_value["range"].get<float>();
            rays.push_back(ray);
        }
        return rays;
    }
    /* jsonからSphere情報に変換 */
    virtual std::list<collision::BoundingSphere> ToSpheres(const JsonData::Json& json) const final {
        std::list<collision::BoundingSphere> spheres;
        collision::BoundingSphere sphere;
        for (const auto& e : json) {
            const auto& first_key = e.items().begin().key();
            const auto& first_value = e.items().begin().value();
            if (first_key != "sphere") continue;
            sphere.Center = first_value.count("center") ? m_transform.position + ToVector3(first_value["center"]) : m_transform.position;
            sphere.Radius = first_value["radius"].get<float>();
            spheres.push_back(sphere);
        }
        return spheres;
    }
    /* jsonとModelからMesh情報に変換 */
    virtual std::list<std::shared_ptr<KdMesh>> ToMeshes(const JsonData::Json& json, const std::shared_ptr<KdModelWork>& model) const final {
        std::list<std::shared_ptr<KdMesh>> meshes;
        for (const auto& e : json) {
            const auto& first_key = e.items().begin().key();
            const auto& first_value = e.items().begin().value();
            if (first_key != "mesh") continue;
            const auto& node = model->GetData()->FindNode(first_value);
            if (!node || !node->m_spMesh) continue;
            if (const auto& mesh = node->m_spMesh; mesh) {
                meshes.push_back(mesh);
            }
        }
        if (!meshes.size()) {
            for (const auto& e : model->GetData()->GetOriginalNodes()) {
                if (const auto& mesh = e.m_spMesh; mesh) {
                    meshes.push_back(mesh);
                }
            }
        }
        return meshes;
    }

    /* jsonからVector3に変換 */
    static Math::Vector3 ToVector3(const JsonData::Json& json) {
        size_t size = json.size();
        if (size == 1) {
            return Math::Vector3(json.get<float>());
        }
        if (size == 3) {
            return Math::Vector3(json[0].get<float>(), json[1].get<float>(), json[2].get<float>());
        }
        return Math::Vector3::Zero;
    }
    /* jsonからTransformに変換 */
    static Transform ToTransform(const JsonData::Json& json) {
        auto position = json.count("position") ? ToVector3(json["position"]) : Math::Vector3::Zero;
        auto rotation = json.count("rotation") ? ToVector3(json["rotation"]) : Math::Vector3::Zero;
        auto scale    = json.count("scale")    ? ToVector3(json["scale"])    : Math::Vector3::One;
        return Transform(position, rotation, scale);
    }
    /* stringからDefaultCollisionTypeに変換 */
    static DefaultCollisionType ToDefaultCollisionType(std::string_view str) {
        auto dct = DefaultCollisionType::None;
        if (str == "bump") {
            dct = DefaultCollisionType::Bump;
        }
        if (str == "attack") {
            dct = DefaultCollisionType::Attack;
        }
        if (str == "road") {
            dct = DefaultCollisionType::Road;
        }
        return dct;
    }

protected:

    bool                                            m_isObjectAlive     = false;
    bool                                            m_isAlive           = false;
    Transform                                       m_transform;
    std::unique_ptr<Collider<DefaultCollisionType>> m_upDefaultCollider = nullptr;
    
};
