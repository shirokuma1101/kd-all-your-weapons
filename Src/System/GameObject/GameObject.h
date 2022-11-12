#pragma once

#include "ExternalDependencies/Asset/Json/JsonData.h"
#include "ExternalDependencies/ImGui/ImGuiHelper.h"
#include "ExternalDependencies/Math/Collider.h"

#include "System/Math/CameraProperties.h"
#include "System/Math/Collision.h"

#include "GameObjectHelper.h"

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:

    using DefaultCollider = Collider<game_object_helper::DefaultCollisionType>;

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
    virtual void Update(float /* delta_time */) {}

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
    virtual const DirectX::SimpleMath::Matrix& GetMatrix() const noexcept final {
        return m_transform.matrix;
    }
    /* DefaultCollider取得 */
    virtual const std::unique_ptr<DefaultCollider>& GetCollider() const noexcept final {
        return m_upDefaultCollider;
    }

    /* 行列を設定 */
    virtual void SetMatrix(const Math::Matrix& mat) noexcept final {
        m_transform.matrix = mat;
    }


    /**************************************************
    * 変換
    **************************************************/
    
    /* jsonからRay情報に変換 !例外処理は無し */
    virtual collision::Ray ToRay(const JsonData::Json& json) const final {
        const auto& properties = json.items().begin().value();
        return collision::Ray(
            properties.count("position") ? m_transform.position + game_object_helper::ToVector3(properties["position"]) : m_transform.position,
            game_object_helper::ToVector3(properties["direction"]),
            properties["range"].get<float>()
        );
    }
    /* jsonからSphere情報に変換 !例外処理は無し */
    virtual collision::BoundingSphere ToSphere(const JsonData::Json& json) const final {
        const auto& properties = json.items().begin().value();
        return collision::BoundingSphere(
            properties.count("center") ? m_transform.position + game_object_helper::ToVector3(properties["center"]) : m_transform.position,
            properties["radius"].get<float>()
        );
    }
    
    /* jsonからRay情報リストに変換 */
    virtual std::list<collision::Ray> ToRays(const JsonData::Json& json) const final {
        std::list<collision::Ray> rays;
        for (const auto& e : json) {
            const auto& type = e.items().begin().key();
            if (type != "ray") continue;
            rays.push_back(ToRay(e));
        }
        return rays;
    }
    /* jsonからSphere情報リストに変換 */
    virtual std::list<collision::BoundingSphere> ToSpheres(const JsonData::Json& json) const final {
        std::list<collision::BoundingSphere> spheres;
        for (const auto& e : json) {
            const auto& type = e.items().begin().key();
            if (type != "sphere") continue;
            spheres.push_back(ToSphere(e));
        }
        return spheres;
    }
    

protected:

    bool                             m_isObjectAlive     = false;   // オブジェクト生死
    bool                             m_isAlive           = false;   // 生死
    Transform                        m_transform;                   // Transform
    std::unique_ptr<DefaultCollider> m_upDefaultCollider = nullptr; // DefaultCollider
    
};
