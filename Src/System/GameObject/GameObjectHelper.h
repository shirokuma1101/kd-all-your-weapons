#pragma once

#define TRANSFORM_ROTATION_USE_EULER
#include "ExternalDependencies/Math/Transform.h"

namespace game_object_helper {

    enum DefaultCollisionType {
        DefaultCollisionTypeNone    = 0,      // 判定無
        DefaultCollisionTypeBump    = 1 << 0, // 判定有
        DefaultCollisionTypeDynamic = 1 << 1, // 動的
        DefaultCollisionTypeRoad    = 1 << 2, // 道路
    };

    
    /**************************************************
    * 変換
    **************************************************/

    /* jsonからVector2に変換 */
    inline Math::Vector2 ToVector2(const JsonData::Json& json) {
        size_t size = json.size();
        if (size == 1) {
            return Math::Vector2(json.get<float>());
        }
        else if (size == 2) {
            return Math::Vector2(json[0].get<float>(), json[1].get<float>());
        }
        return Math::Vector2::Zero;
    }
    /* jsonからVector3に変換 */
    inline Math::Vector3 ToVector3(const JsonData::Json& json) {
        size_t size = json.size();
        if (size == 1) {
            return Math::Vector3(json.get<float>());
        }
        else if (size == 3) {
            return Math::Vector3(json[0].get<float>(), json[1].get<float>(), json[2].get<float>());
        }
        return Math::Vector3::Zero;
    }
    /* jsonからTransformに変換 */
    inline Transform ToTransform(const JsonData::Json& json) {
        auto position = json.count("position") ? ToVector3(json["position"]) : Math::Vector3::Zero;
        auto rotation = json.count("rotation") ? ToVector3(json["rotation"]) : Math::Vector3::Zero;
        auto scale    = json.count("scale")    ? ToVector3(json["scale"])    : Math::Vector3::One;
        return Transform(position, rotation, scale);
    }
    /* jsonからDefaultCollisionTypeに変換 */
    inline DefaultCollisionType ToDefaultCollisionType(const JsonData::Json& json) {
        templates::UnderlyingTypeWrapperT<DefaultCollisionType> dct = DefaultCollisionTypeNone;
        for (const auto& e : json) {
            if (e == "bump") {
                dct |= DefaultCollisionTypeBump;
            }
            else if (e == "dynamic") {
                dct |= DefaultCollisionTypeDynamic;
            }
            else if (e == "road") {
                dct |= DefaultCollisionTypeRoad;
            }
        }
        return static_cast<DefaultCollisionType>(dct);
    }
    
}
