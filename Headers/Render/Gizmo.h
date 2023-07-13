#pragma once

namespace Resources
{
	class Model;
	class Shader;
}

namespace Core
{
	class GameObject;
}

namespace Component
{
	class Transform;
}

namespace Render
{
	enum Axis
	{
		X,
		Y,
		Z
	};

	enum class GizmoMode
	{
		World,
		Local,
	};

	enum GizmoModels
	{
		POSITION,
		ROTATION,
		SCALE,
	};

	class PANDOR_API Gizmo
	{
	private:

		Resources::Model*  m_model;
		Core::GameObject* m_models[3];
		Core::GameObject*  m_gizmo = nullptr;

		float m_scaleFactorAdjustment = 0.4f;
		float m_minimScaleFactor = 1.0f;
		float m_objectSize = 70;
		float m_lenght = 100.f;
		float m_angle;

		Math::MathUtils::Raycast m_mouseRay;
		Math::MathUtils::Raycast m_RayAxis[3];
		Math::MathUtils::Circle  m_CircleAxis[3];

		Vector3    m_tempPos;
		Vector3    m_tempScale;
		Quaternion m_tempRot;

		Vector3 m_dist;
		Vector3 m_newPoint;
		Vector3 m_firstClick;
		Vector3 m_intersectPoint[3];
		Vector3 m_gizmoCenter;

		GizmoMode m_gizmoMode = GizmoMode::World;
		GizmoModels m_GizModel;
		Axis        m_axis;


	public:
		bool clickedGizmo;

	public:
		~Gizmo();
		void CreateModel();
		void Draw(Component::Transform* trans, bool sceneWindow = true);
		void Inputs();
		Core::GameObject* GetGizmo() { return m_gizmo; }
		void Delete();

		Render::GizmoMode GetGizmoMode() const { return m_gizmoMode; }
		void SetGizmoMode(Render::GizmoMode val);
	};
}