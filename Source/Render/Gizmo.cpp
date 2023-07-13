#include <pch.h>

#include <Render\Gizmo.h>
#include <Resources/Model.h>
#include <Resources/ResourcesManager.h>
#include <Core/GameObject.h>
#include <Components/MeshComponent.h>
#include <Render/Camera.h>
#include <Math/Maths.h>
#include <Core/Scene.h>
#include <Core/App.h>
#include <Core/SceneManager.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/PrefabWindow.h>
#include <EditorUI/Inspector.h>

Render::Gizmo::~Gizmo()
{
}

void Render::Gizmo::CreateModel()
{
	m_model = ResourcesManager::Get()->Create<Resources::Model>(ENGINEPATH"Models/PositionGizmo.obj");
	m_model->Display(false);
	m_model = ResourcesManager::Get()->Create<Resources::Model>(ENGINEPATH"Models/RotateGizmo.obj");
	m_model->Display(false);
	m_model = ResourcesManager::Get()->Create<Resources::Model>(ENGINEPATH"Models/ScaleGizmo.obj");
	m_model->Display(false);

	if (m_model->HasBeenSent())
		m_gizmo = m_model->ToGameObject(false);
}

void Render::Gizmo::Inputs()
{
	if (WrapperUI::IsAnyItemActive() 
		|| !Core::SceneManager::Get()->GetCurrentScene()->IsPrefabScene() && !Core::SceneManager::Get()->GetCurrentScene()->GetEditorCamera()->IsVisible() 
		|| Core::SceneManager::Get()->GetCurrentScene()->IsPrefabScene() && !Core::SceneManager::Get()->GetCurrentScene()->GetEditorCamera()->IsVisiblePrefab())
		return;
	if (!m_gizmo || WrapperUI::IsKeyPressed(Key::Key_Q) && !WrapperUI::IsMouseDown(MouseButton::Right))
	{
		m_model = ResourcesManager::Get()->Find<Resources::Model>(ENGINEPATH"Models/PositionGizmo.obj");
		if (m_gizmo) {
			m_tempPos = m_gizmo->transform->GetLocalPosition();
			m_tempScale = m_gizmo->transform->GetLocalScale();
			if (!m_models[0] && m_model->HasBeenSent()) {
				m_models[0] = m_model->ToGameObject(false);
			}
			m_gizmo = m_models[0];
			m_gizmo->transform->SetLocalPosition(m_tempPos);
			m_gizmo->transform->SetLocalScale(m_tempScale);
		}
		else
		{
			if (!m_models[0] && m_model->HasBeenSent()) {
				m_models[0] = m_model->ToGameObject(false);
			}
			m_gizmo = m_models[0];
		}

		m_GizModel = GizmoModels::POSITION;

	}
	if (WrapperUI::IsKeyPressed(Key::Key_W) && !WrapperUI::IsMouseDown(MouseButton::Right))
	{
		m_model = ResourcesManager::Get()->Find<Resources::Model>(ENGINEPATH"Models/RotateGizmo.obj");
		m_tempPos = m_gizmo->transform->GetLocalPosition();
		m_tempScale = m_gizmo->transform->GetLocalScale();
		if (!m_models[1] && m_model->HasBeenSent()) {
			m_models[1] = m_model->ToGameObject(false);
		}
		m_gizmo = m_models[1];
		m_gizmo->transform->SetLocalPosition(m_tempPos);
		m_gizmo->transform->SetLocalScale(m_tempScale);

		m_GizModel = GizmoModels::ROTATION;
	}
	if (WrapperUI::IsKeyPressed(Key::Key_E) && !WrapperUI::IsMouseDown(MouseButton::Right))
	{
		m_model = ResourcesManager::Get()->Find<Resources::Model>(ENGINEPATH"Models/ScaleGizmo.obj");
		m_tempPos = m_gizmo->transform->GetLocalPosition();
		m_tempScale = m_gizmo->transform->GetLocalScale();
		if (!m_models[2] && m_model->HasBeenSent()) {
			m_models[2] = m_model->ToGameObject(false);
		}
		m_gizmo = m_models[2];
		m_gizmo->transform->SetLocalPosition(m_tempPos);
		m_gizmo->transform->SetLocalScale(m_tempScale);

		m_GizModel = GizmoModels::SCALE;

	}
}

float DistCircleBetweenTwoPoint(Vector3 newPoint, Vector3 firstClick, Math::MathUtils::Circle c, Math::MathUtils::Raycast mouseRay)
{
	newPoint = ClosestDistanceLineCircle(mouseRay, c);
	Vector3 newDist = (newPoint - c.center);
	float angle = newDist.AngleBetweenVector(firstClick - c.center, c.orientation);

	return angle;
}

void Render::Gizmo::Draw(Component::Transform* trans, bool sceneWindow /*= true*/)
{
	Inputs();

	if (m_gizmo != nullptr)
	{
		m_gizmo->DrawSelfAndChild(true);
		m_gizmo->transform->SetLocalPosition(trans->GetWorldPosition());
		if (m_GizModel == GizmoModels::SCALE || GetGizmoMode() == GizmoMode::Local)
			m_gizmo->transform->SetLocalRotation(trans->GetWorldRotation());
		else if (!WrapperUI::IsMouseDown(MouseButton::Left))
			m_gizmo->transform->SetLocalRotation(Quaternion::Identity());


		Vector3 camPos = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetTransform()->GetWorldPosition();
		Vector3 distanceToCamera = camPos - m_gizmo->transform->GetLocalPosition();

		float calculatedDistance = std::sqrt(distanceToCamera.x * distanceToCamera.x + distanceToCamera.y * distanceToCamera.y + distanceToCamera.z * distanceToCamera.z);
		float scaleFactor = (m_objectSize * calculatedDistance) / (Core::App::Get().GetEditorUIManager().GetSceneWindow().GetWindowSize().x * std::tan(Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->fov / 2)) * m_scaleFactorAdjustment;

		if (scaleFactor < m_minimScaleFactor)
			scaleFactor = m_minimScaleFactor;

		m_gizmo->transform->SetLocalScale(scaleFactor);

		for (int i = 0; i < 3; i++)
		{
			m_CircleAxis[i].radius = scaleFactor * 100;
			m_CircleAxis[i].center = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->GetWorldPosition();
		}

		m_CircleAxis[0].orientation = m_gizmo->transform->GetRightVector();
		m_CircleAxis[1].orientation = m_gizmo->transform->GetUpVector();
		m_CircleAxis[2].orientation = m_gizmo->transform->GetForwardVector();

		auto mousePos = sceneWindow ? Core::App::Get().GetEditorUIManager().GetSceneWindow().GetMousePosition() : Core::App::Get().GetEditorUIManager().GetPrefabWindow().GetMousePosition();

		m_mouseRay.origin = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->UnProject(mousePos);
		m_mouseRay.direction = m_mouseRay.origin - Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->UnProject({ mousePos, 100 });

		if (auto pickGiz = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->pickedGizmo)
		{
			//First Click
			if (WrapperUI::IsMouseClicked(MouseButton::Left) &&
				(sceneWindow && Core::App::Get().GetEditorUIManager().GetSceneWindow().IsHovered()
					|| !sceneWindow && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsHovered()))
			{
				switch (m_GizModel)
				{
				case Render::POSITION:
				{
					m_gizmoCenter = m_gizmo->transform->GetLocalPosition();
					m_RayAxis[Axis::X].origin = m_gizmoCenter;
					m_RayAxis[Axis::X].direction = m_gizmo->transform->GetRightVector() * m_lenght;
					m_dist.x = Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::X]);
					m_intersectPoint[Axis::X] = m_RayAxis[Axis::X].origin + m_RayAxis[Axis::X].direction * m_RayAxis[Axis::X].scale;

					m_RayAxis[Axis::Y].origin = m_gizmoCenter;
					m_RayAxis[Axis::Y].direction = m_gizmo->transform->GetUpVector() * m_lenght;
					m_dist.y = Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Y]);
					m_intersectPoint[Axis::Y] = m_RayAxis[Axis::Y].origin + m_RayAxis[Axis::Y].direction * m_RayAxis[Axis::Y].scale;

					m_RayAxis[Axis::Z].origin = m_gizmoCenter;
					m_RayAxis[Axis::Z].direction = m_gizmo->transform->GetForwardVector() * m_lenght;
					m_dist.z = Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Z]);
					m_intersectPoint[Axis::Z] = m_RayAxis[Axis::Z].origin + m_RayAxis[Axis::Z].direction * m_RayAxis[Axis::Z].scale;

					break;
				}
				case Render::SCALE:
				{
					m_tempScale = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->GetLocalScale();
					m_gizmoCenter = m_gizmo->transform->GetLocalPosition();

					m_RayAxis[Axis::X].origin = m_gizmoCenter;
					m_RayAxis[Axis::X].direction = m_gizmo->transform->GetRightVector() * m_lenght;
					m_dist.x = Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::X]);
					m_intersectPoint[Axis::X] = m_RayAxis[Axis::X].origin + m_RayAxis[Axis::X].direction * m_RayAxis[Axis::X].scale;

					m_RayAxis[Axis::Y].origin = m_gizmoCenter;
					m_RayAxis[Axis::Y].direction = m_gizmo->transform->GetUpVector() * m_lenght;
					m_dist.y = Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Y]);
					m_intersectPoint[Axis::Y] = m_RayAxis[Axis::Y].origin + m_RayAxis[Axis::Y].direction * m_RayAxis[Axis::Y].scale;

					m_RayAxis[Axis::Z].origin = m_gizmoCenter;
					m_RayAxis[Axis::Z].direction = m_gizmo->transform->GetForwardVector() * m_lenght;
					m_dist.z = Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Z]);
					m_intersectPoint[Axis::Z] = m_RayAxis[Axis::Z].origin + m_RayAxis[Axis::Z].direction * m_RayAxis[Axis::Z].scale;

					break;
				}
				case Render::ROTATION:
				{
					m_tempRot = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->GetWorldRotation();
					if (pickGiz->GetName() == "xaxisgizrot") {
						m_firstClick = ClosestDistanceLineCircle(m_mouseRay, m_CircleAxis[Axis::X]);
						m_axis = Axis::X;

					}
					else if (pickGiz->GetName() == "yaxisgizrot") {
						m_firstClick = ClosestDistanceLineCircle(m_mouseRay, m_CircleAxis[Axis::Y]);
						m_axis = Axis::Y;
					}
					else if (pickGiz->GetName() == "zaxisgizrot") {
						m_firstClick = ClosestDistanceLineCircle(m_mouseRay, m_CircleAxis[Axis::Z]);
						m_axis = Axis::Z;
					}

					break;
				}
				default:
					break;
				}

			}

			if (WrapperUI::IsMouseDown(MouseButton::Left) &&
				(sceneWindow && Core::App::Get().GetEditorUIManager().GetSceneWindow().IsHovered()
					|| !sceneWindow && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsHovered()))
			{
				m_mouseRay.origin = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->UnProject(mousePos);
				m_mouseRay.direction = m_mouseRay.origin - Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->UnProject({ mousePos, 100 });

				switch (m_GizModel)
				{
				case Render::POSITION:
				{
					if (pickGiz->GetName() == "xaxisgiz")
					{
						Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::X]);
						Vector3 tempIntersectPoint = m_RayAxis[Axis::X].origin + m_RayAxis[Axis::X].direction * m_RayAxis[Axis::X].scale;
						Vector3 NewPosition = m_gizmoCenter - m_intersectPoint[Axis::X] + tempIntersectPoint;
						Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldPosition(NewPosition);
					}
					else if (pickGiz->GetName() == "yaxisgiz")
					{
						Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Y]);
						Vector3 tempIntersectPoint = m_RayAxis[Axis::Y].origin + m_RayAxis[Axis::Y].direction * m_RayAxis[Axis::Y].scale;
						Vector3 NewPosition = m_gizmoCenter - m_intersectPoint[Axis::Y] + tempIntersectPoint;
						Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldPosition(NewPosition);
					}
					else if (pickGiz->GetName() == "zaxisgiz")
					{
						Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Z]);
						Vector3 tempIntersectPoint = m_RayAxis[Axis::Z].origin + m_RayAxis[Axis::Z].direction * m_RayAxis[Axis::Z].scale;
						Vector3 NewPosition = m_gizmoCenter - m_intersectPoint[Axis::Z] + tempIntersectPoint;
						Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldPosition(NewPosition);
					}

					break;
				}
				case Render::ROTATION:
				{
					if (pickGiz->GetName() == "xaxisgizrot" && m_axis == Axis::X)
					{
						m_angle = DistCircleBetweenTwoPoint(m_newPoint, m_firstClick, m_CircleAxis[Axis::X], m_mouseRay);
						if (GetGizmoMode() == GizmoMode::World)
						{
							m_gizmo->transform->SetLocalRotation(Quaternion::Identity() * Quaternion::AngleAxis(m_angle, Vector3::Right()));
							Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldRotation(Quaternion::AngleAxis(m_angle, Vector3::Right()) * m_tempRot);
						}
						else
							Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldRotation(m_tempRot * Quaternion::AngleAxis(m_angle, Vector3::Right()));
					}
					else if (pickGiz->GetName() == "yaxisgizrot" && m_axis == Axis::Y)
					{
						m_angle = DistCircleBetweenTwoPoint(m_newPoint, m_firstClick, m_CircleAxis[Axis::Y], m_mouseRay);
						if (GetGizmoMode() == GizmoMode::World)
						{
							m_gizmo->transform->SetLocalRotation(Quaternion::Identity() * Quaternion::AngleAxis(m_angle, Vector3::Up()));
							Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldRotation(Quaternion::AngleAxis(m_angle, Vector3::Up()) * m_tempRot);
						}
						else
							Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldRotation(m_tempRot * Quaternion::AngleAxis(m_angle, Vector3::Up()));
					}
					else if (pickGiz->GetName() == "zaxisgizrot" && m_axis == Axis::Z)
					{
						m_angle = DistCircleBetweenTwoPoint(m_newPoint, m_firstClick, m_CircleAxis[Axis::Z], m_mouseRay);
						if (GetGizmoMode() == GizmoMode::World)
						{
							m_gizmo->transform->SetLocalRotation(Quaternion::Identity() * Quaternion::AngleAxis(m_angle, Vector3::Forward()));
							Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldRotation(Quaternion::AngleAxis(m_angle, Vector3::Forward()) * m_tempRot);
						}
						else
							Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldRotation(m_tempRot * Quaternion::AngleAxis(m_angle, Vector3::Forward()));
					}

					break;
				}
				case Render::SCALE:
				{

					if (pickGiz->GetName() == "xaxisgizscale")
					{
						Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::X]);
						Vector3 tempIntersectPoint = m_RayAxis[Axis::X].origin + m_RayAxis[Axis::X].direction * m_RayAxis[Axis::X].scale;
						Math::Vector3 NewScale = m_tempScale - m_gizmo->transform->GetLocalRotation().GetInverse() * (m_intersectPoint[Axis::X] - tempIntersectPoint);
						Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldScale(NewScale);
					}
					else if (pickGiz->GetName() == "yaxisgizscale")
					{
						Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Y]);
						Vector3 tempIntersectPoint = m_RayAxis[Axis::Y].origin + m_RayAxis[Axis::Y].direction * m_RayAxis[Axis::Y].scale;
						Math::Vector3 NewScale = m_tempScale - m_gizmo->transform->GetLocalRotation().GetInverse() * (m_intersectPoint[Axis::Y] - tempIntersectPoint);
						Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldScale(NewScale);
					}
					else if (pickGiz->GetName() == "zaxisgizscale")
					{
						Math::MathUtils::ClosestDistanceBetweenLines(m_mouseRay, m_RayAxis[Axis::Z]);
						Vector3 tempIntersectPoint = m_RayAxis[Axis::Z].origin + m_RayAxis[Axis::Z].direction * m_RayAxis[Axis::Z].scale;
						Math::Vector3 NewScale = m_tempScale - m_gizmo->transform->GetLocalRotation().GetInverse() * (m_intersectPoint[Axis::Z] - tempIntersectPoint);
						Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->transform->SetWorldScale(NewScale);
					}

					break;
				}
				default:
					break;
				}

			}
			else if (WrapperUI::IsMouseReleased(MouseButton::Left) &&
				(sceneWindow && Core::App::Get().GetEditorUIManager().GetSceneWindow().IsHovered()
					|| !sceneWindow && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsHovered()))
			{
				if (m_GizModel == GizmoModels::ROTATION && GetGizmoMode() != GizmoMode::Local)
					m_gizmo->transform->SetLocalRotation(Quaternion::Identity());
			}
		}
	}

}

void Render::Gizmo::Delete()
{
	for (int i = 0; i < 3; i++)
	{
		if (m_models[i])
		{
			delete m_models[i];
			m_models[i] = nullptr;
		}
	}
}

void Render::Gizmo::SetGizmoMode(Render::GizmoMode val)
{
	m_gizmoMode = val;
}
