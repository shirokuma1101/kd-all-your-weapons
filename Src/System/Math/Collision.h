#pragma once

#include "ExternalDependencies/Math/Collision.h"

namespace model_collision {
	inline bool Intersects(const KdMesh& mesh, const collision::Ray& target, const Math::Matrix& mat, collision::Result* res = nullptr) {
		memset(res, 0, sizeof(collision::Result));
		// 逆行列
		Math::Matrix  inv_mat = mat.Invert();
		// ワールド座標に変換
		Math::Vector3 inv_target_position = DirectX::XMVector3TransformCoord(target.position, inv_mat);
		Math::Vector3 inv_target_direction = DirectX::XMVector3TransformNormal(target.direction, inv_mat);
		// レイの方向ベクトルの長さ (逆行列に拡縮がある場合長さにも反映される)
		float         target_direction_length = inv_target_direction.Length();
		// レイの長さが変わる為、判定用の最大距離にも反映させる
		float         limit_range = target.range * target_direction_length;
		inv_target_direction.Normalize();


		/* BroadPhase AABB vs Ray */

		float aabb_dst = FLT_MAX;
		DirectX::BoundingBox aabb;
		mesh.GetBoundingBox().Transform(aabb, mat);
		if (!aabb.Intersects(target.position, target.direction, aabb_dst)) {
			return false;
		}
		if (aabb_dst > limit_range) {
			return false;
		}


		/* NarrowPhase Ray vs Faces */

		bool  hit = false;
		float closest_dist = FLT_MAX;

		const KdMeshFace* const           faces = &mesh.GetFaces()[0];
		size_t                            face_count = mesh.GetFaces().size();
		const std::vector<Math::Vector3>& vertex_positions = mesh.GetVertexPositions();

		for (size_t i = 0; i < face_count; ++i) {
			const UINT* const idx = faces[i].Idx;
			float dist = FLT_MAX;

			if (!DirectX::TriangleTests::Intersects(
				inv_target_position,
				inv_target_direction,
				vertex_positions[idx[0]],
				vertex_positions[idx[1]],
				vertex_positions[idx[2]],
				dist
			)) continue;

			if (dist <= limit_range) {
				hit = true;
				// 考慮されていた拡大率を元に戻す
				dist /= target_direction_length;
				if (dist < closest_dist) {
					closest_dist = dist;
				}
			}
		}

		if (!res) {
			return hit;
		}

		res->overlap = hit;
		if (hit) {
			res->position = target.position + target.direction * closest_dist;
			res->direction = target.direction * -1;
			res->depth = target.range - closest_dist;
		}
		return hit;
	}
	inline bool Intersects(const KdMesh& mesh, const collision::BoundingSphere& target, const Math::Matrix& mat, collision::Result* res = nullptr) {
		memset(res, 0, sizeof(collision::Result));
		// 逆行列
		Math::Matrix  inv_mat = mat.Invert();
		// ワールド座標に変換
		Math::Vector3 inv_sphere_position = DirectX::XMVector3TransformCoord(Math::Vector3(target.Center), inv_mat);


		/* BroadPhase AABB vs BoundingSphere */

		DirectX::BoundingBox aabb;
		mesh.GetBoundingBox().Transform(aabb, mat);
		if (!aabb.Intersects(target)) {
			return false;
		}


		/* NarrowPhase Sphere vs Faces */

		bool hit = false;
		Math::Vector3 nearest_point;
		Math::Vector3 final_position = inv_sphere_position;

		const KdMeshFace* const           faces = &mesh.GetFaces()[0];
		size_t                            face_count = mesh.GetFaces().size();
		const std::vector<Math::Vector3>& vertex_positions = mesh.GetVertexPositions();
		Math::Vector3                     scale(mat.Right().Length(), mat.Up().Length(), mat.Backward().Length());

		for (size_t i = 0; i < face_count; ++i) {
			Math::Vector3 near_point;
			collision::mesh::PointToTriangle(
				final_position,
				vertex_positions[faces[i].Idx[0]],
				vertex_positions[faces[i].Idx[1]],
				vertex_positions[faces[i].Idx[2]],
				&near_point
			);

			Math::Vector3 push_vector = final_position - near_point;
			push_vector *= scale;

			if (push_vector.LengthSquared() <= target.Radius * target.Radius) {
				hit = true;

				near_point = DirectX::XMVector3TransformCoord(near_point, mat);

				float push_vector_length = push_vector.Length();
				push_vector.Normalize();
				push_vector *= target.Radius - push_vector_length;
				push_vector = push_vector / scale;

				final_position += push_vector;
				nearest_point = near_point;
			}
		}

		if (!res) {
			return hit;
		}

		res->overlap = hit;
		if (hit) {
			res->position = DirectX::XMVector3TransformCoord(nearest_point, mat);
			res->direction = Math::Vector3(DirectX::XMVector3TransformCoord(final_position, mat)) - target.Center;
			res->depth = res->direction.Length();
			if (res->depth) {
				res->direction.Normalize();
			}
		}

		return hit;
	}
}

template<class T>
class CollisionModel : public ICollisionShape<T>
{
public:

	CollisionModel(T type, const std::shared_ptr<KdModelWork>& model, std::string_view name = "")
		: ICollisionShape<T>(type)
		, m_model(model)
		, m_nodeName(name)
	{}
	~CollisionModel() {}

	virtual bool Intersects(const collision::Ray& target, const Math::Matrix& mat, collision::Result* res = nullptr) const override {
		memset(res, 0, sizeof(collision::Result));
		if (!m_model) {
			return false;
		}

		const std::vector<KdModelData::Node>& data_nodes = m_model->GetDataNodes();
		const std::vector<KdModelWork::Node>& work_nodes = m_model->GetNodes();
		collision::Result                     result;
		collision::Result                     nearest_result;
		bool                                  hit = false;

		if (m_nodeName.empty()) {
			for (size_t i = 0; i < data_nodes.size(); ++i) {
				const auto& data_node = data_nodes[i];
				const auto& work_node = work_nodes[i];

				if (!data_node.m_spMesh) {
					return false;
				}

				if (!model_collision::Intersects(*data_node.m_spMesh, target, work_node.m_worldTransform * mat, &result)) continue;

				if (!res) {
					return true;
				}

				hit = true;
				if (result.depth >= nearest_result.depth) {
					nearest_result = result;
				}
			}
		}
		else {
			const auto data_node = m_model->FindDataNode(m_nodeName);
			const auto work_node = m_model->FindNode(m_nodeName);
			if (!data_node || !work_node) {
				return false;
			}
			if (!data_node->m_spMesh) {
				return false;
			}

			if (!model_collision::Intersects(*data_node->m_spMesh, target, work_node->m_worldTransform * mat, &result)) {
				return false;
			}

			if (!res) {
				return true;
			}

			hit = true;
			nearest_result = result;
		}

		if (!res) {
			return hit;
		}

		res->overlap = hit;
		if (hit) {
			*res = nearest_result;
		}

		return hit;
	}
	virtual bool Intersects(const DirectX::BoundingSphere& target, const Math::Matrix& mat, collision::Result* res = nullptr) const override {
		memset(res, 0, sizeof(collision::Result));
		if (!m_model) {
			return false;
		}

		const std::vector<KdModelData::Node>& data_nodes = m_model->GetDataNodes();
		const std::vector<KdModelWork::Node>& work_nodes = m_model->GetNodes();
		bool                                  hit = false;
		collision::Result                     result;
		collision::Result                     nearest_result;

		if (m_nodeName.empty()) {
			for (size_t i = 0; i < data_nodes.size(); ++i) {
				const auto& data_node = data_nodes[i];
				const auto& work_node = work_nodes[i];

				if (!data_node.m_spMesh) {
					return false;
				}

				if (!model_collision::Intersects(*data_node.m_spMesh, target, work_node.m_worldTransform * mat, &result)) continue;

				if (!res) {
					return true;
				}

				hit = true;
				if (result.depth >= nearest_result.depth) {
					nearest_result = result;
				}
			}
		}
		else {
			const auto data_node = m_model->FindDataNode(m_nodeName);
			const auto work_node = m_model->FindNode(m_nodeName);
			if (!data_node || !work_node) {
				return false;
			}
			if (!data_node->m_spMesh) {
				return false;
			}

			if (!model_collision::Intersects(*data_node->m_spMesh, target, work_node->m_worldTransform * mat, &result)) {
				return false;
			}

			if (!res) {
				return true;
			}

			hit = true;
			nearest_result = result;
		}

		if (!res) {
			return hit;
		}

		res->overlap = hit;
		if (hit) {
			*res = nearest_result;
		}

		return hit;
	}

private:

	std::shared_ptr<KdModelWork> m_model = nullptr;
	std::string                  m_nodeName;

};
