#pragma once
#include "PandorAPI.h"

#include <Math/Maths.h>
#include <unordered_map>
#include <string>

namespace physx
{
	class PxVec3;
	class PxQuat;
	class PxTransform;
	class PxActor;
	class PxDefaultAllocator;
	class PxDefaultErrorCallback;
	class PxDefaultCpuDispatcher;
	class PxTolerancesScale;
	class PxPvd;
	class PxPvdTransport;
	class PxPvdSceneClient;
	class PxFoundation;
	class PxTriangleMesh;
	class PxConvexMesh;
	class PxJoint;

	class PxScene;
	class PxPhysics;
	class PxMaterial;
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxCooking;
	struct PxRaycastHit;
	template<typename HitType> struct PxHitBuffer;
	typedef PxHitBuffer<PxRaycastHit> PxRaycastBuffer;
	class PxShape;
}
namespace Core
{
	class GameObject;
}

namespace Component
{
	class Transform;
	class Rigidbody;
	class BoxCollider;
	class SphereCollider;
	class CapsuleCollider;
	class MeshCollider;
	class Collider;
	class Constraint;
}
namespace Physic
{
	class RaycastHit;
}
namespace Resources
{
	class PhysicMaterial;
	class Mesh;
}
namespace Core::Wrapper::WrapperPhysic {


	PANDOR_API Math::Vector3 ToVector3(const physx::PxVec3& vector);
	PANDOR_API physx::PxVec3 ToPxVec3(const Math::Vector3& vector);

	PANDOR_API Math::Quaternion ToQuaternion(const physx::PxQuat& quat);
	PANDOR_API physx::PxQuat ToPxQuat(const Math::Quaternion& vector);

	PANDOR_API physx::PxTransform ToPhysXTransform(Component::Transform* transform);
	PANDOR_API void ToTransform(physx::PxTransform* physXtransform, Component::Transform* transform);

	class PANDOR_API PhysicManager
	{
	private:
		std::unordered_map<Component::Collider*, physx::PxShape*> m_colliderList;
		std::unordered_map<Component::Rigidbody*, physx::PxRigidDynamic*> m_rigidbodies;
		std::unordered_map<Component::Collider*, physx::PxRigidStatic*> m_staticbodies;
		std::unordered_map<std::string, physx::PxMaterial*> m_materials;
		std::unordered_map<Resources::Mesh*, physx::PxConvexMesh*> m_convexMesh;
		std::unordered_map<Resources::Mesh*, physx::PxTriangleMesh*> m_triangleMesh;

		physx::PxDefaultAllocator* m_defaultAllocatorCallback;
		physx::PxDefaultErrorCallback* m_defaultErrorCallback;
		physx::PxDefaultCpuDispatcher* m_dispatcher = NULL;
		physx::PxTolerancesScale* m_toleranceScale;
		physx::PxPvd* m_pvd = NULL;
		physx::PxPvdTransport* m_transport = NULL;
		physx::PxPvdSceneClient* m_sceneClient = NULL;
		physx::PxFoundation* m_foundation = NULL;

		physx::PxScene* m_scene = NULL;
		physx::PxPhysics* m_physics = NULL;

		physx::PxMaterial* m_defaultMaterial;
		physx::PxCooking* m_cooking;

		friend class RigidDynamic;
	private:

		Physic::RaycastHit ToRayCastHit(const physx::PxRaycastBuffer& hit);

		void AddCollider(physx::PxShape* actor, Component::Collider* collider);

	public:
		~PhysicManager();

		void Initialize();
		void CreateScene(); 
		void DeleteScene();

		void ClearLists();

		void Update();
		void Delete();

		void CreateRigidbody(Component::Rigidbody* rb);
		// Cube
		void CreateCube(Component::BoxCollider* box);
		// Sphere
		void CreateSphere(Component::SphereCollider* sphere);
		// Capsule
		void CreateCapsule(Component::CapsuleCollider* capsule);

		void CreateMesh(Component::MeshCollider* meshCollider);

		void CreateJoint(Component::Constraint* fixedJoint);

		void DestroyJoint(Component::Constraint* fixedJoint);

		physx::PxMaterial* GetMaterial(Resources::PhysicMaterial* material);

		void CreateMaterial(std::string name, float staticFriciton, float dynamicFriction, float restituion);

		void UpdateMaterial(std::string name, float staticFriciton, float dynamicFriction, float restituion);

		bool RayCast(const Math::Vector3& origin, const Math::Vector3& direction, float distanceMax, Physic::RaycastHit& hit);

		void RemoveCollider(Component::Collider* gameObject);

		void RemoveRigidbody(Component::Rigidbody* rb);

		void AddStatic(Component::Collider* col, physx::PxRigidStatic* body);

		void RemoveStatic(Component::Collider* col);

		Component::Collider* GetColliderWithShape(physx::PxShape* shape);

		physx::PxShape* GetShapeWithCollider(Component::Collider* collider);
	};

	enum class ShapeType
	{
		Box,
		Sphere,
		Capsule,
		Mesh,
	};

	class PANDOR_API Shape
	{
	private:
		physx::PxShape* m_shape;
	public:
		~Shape();
		void SetScale(const Vector3& scale);
		virtual void SetOffset(const Vector3& offset);
		void SetLocal(const Vector3& pos, const Quaternion& rot);
		void SetTrigger(bool trigger);

		void Destroy(Component::Collider* collider);

		friend PhysicManager;
		friend class RigidDynamic;
	};

	class PANDOR_API RigidDynamic
	{
	private:
		physx::PxRigidDynamic* m_body = nullptr;

	public:
		RigidDynamic();
		~RigidDynamic();

		void Destroy();
		void Detach(Component::Collider* shape);

		void ResetBody() { m_body = nullptr; }

		void AddForce(const Math::Vector3& force);
		void AddTorque(const Math::Vector3& torque);
		void ClearForce();
		void WakeUp();

		// Getters
		Math::Vector3 GetPosition();
		Math::Quaternion GetRotation();
		Math::Vector3 GetAngularVelocity();
		Math::Vector3 GetVelocity();
		float GetMass();

		// Setters
		void SetPosition(const Math::Vector3& position);
		void SetRotation(const Math::Quaternion& rotation);
		virtual void SetAngularVelocity(const Math::Vector3& angular);
		virtual void SetVelocity(const Math::Vector3& velocity);
		virtual void SetMass(float mass);
		void SetGravityEnable(bool enable);
		void FixRotation(bool fixRotationX, bool fixRotationY, bool fixRotationZ);
		void SetKinematic(bool isKinematic);

		bool BodyInitialized() { return m_body != nullptr; }

		friend PhysicManager;
	};

	class PANDOR_API Joint
	{
	private:
		physx::PxJoint* m_joint;
	public:
		bool Initalized() { return m_joint != nullptr; }
		// Distance
		void SetMaxDistance(float max);
		void SetMinDistance(float min);
		void SetDamping(float damp);
		void SetStiffness(float stif);

		void SetDriveVelocity(float velocity);

		friend PhysicManager;
	};
}