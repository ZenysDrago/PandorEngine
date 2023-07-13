#include "pch.h"

#include <PhysX/PxPhysicsAPI.h>

#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Core/GameObject.h>
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/SceneWindow.h>
#endif

#include <Components/Transform.h>
#include <Components/Rigidbody.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>
#include <Components/SphereCollider.h>
#include <Components/MeshCollider.h>
#include <Components/Constraint.h>

#include <Render/Camera.h>

#include <Physic/RaycastHit.h>

#include <Resources/PhysicMaterial.h>

class ContactModifyCallback : public physx::PxContactModifyCallback
{
	void onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 count)
	{
		//PrintWarning("Test");
	}
};

// Collision Callback
class CollisionCallback : public physx::PxSimulationEventCallback
{
public:
	CollisionCallback() {}
	~CollisionCallback() {}

	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override
	{
		PrintError("Constraint Break ");
	}
	virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override
	{
		PrintError("Wake");
	}
	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override
	{
		PrintError("Sleep");
	}
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
	{
		auto actor1Go = Core::App::Get().physic->GetColliderWithShape(pairs->shapes[0]);
		auto actor2Go = Core::App::Get().physic->GetColliderWithShape(pairs->shapes[1]);
		if (!actor1Go || !actor2Go)
			return;
		collides[actor1Go] = actor2Go;
	}
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
	{
		auto actor1Go = Core::App::Get().physic->GetColliderWithShape(pairs->triggerShape);
		auto actor2Go = Core::App::Get().physic->GetColliderWithShape(pairs->otherShape);

		if (!actor1Go || !actor2Go)
			return;

		if (!triggers.count(actor1Go))
			triggers[actor1Go] = actor2Go;
		else
			triggers.erase(actor1Go);
	}
	virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override
	{
		PrintError("Advance");
	}

	std::unordered_map<Component::Collider*, Component::Collider*> collides;
	std::unordered_map<Component::Collider*, Component::Collider*> triggers;
private:

};
CollisionCallback collisionCallback = CollisionCallback();
ContactModifyCallback contactModifyCallback = ContactModifyCallback();


Math::Quaternion Core::Wrapper::WrapperPhysic::ToQuaternion(const physx::PxQuat& quat)
{
	return Math::Quaternion(quat.x, quat.y, quat.z, quat.w);
}

physx::PxQuat Core::Wrapper::WrapperPhysic::ToPxQuat(const Math::Quaternion& quat)
{
	auto newQuat = quat.GetNormal();
	return physx::PxQuat(newQuat.x, newQuat.y, newQuat.z, newQuat.w);
}

Math::Vector3 Core::Wrapper::WrapperPhysic::ToVector3(const physx::PxVec3& vector)
{
	return { vector.x, vector.y, vector.z };
}

physx::PxVec3 Core::Wrapper::WrapperPhysic::ToPxVec3(const Math::Vector3& vector)
{
	return { vector.x, vector.y, vector.z };
}

physx::PxTransform Core::Wrapper::WrapperPhysic::ToPhysXTransform(Component::Transform* transform)
{
	if (!transform)
		return physx::PxTransform(physx::PxIdentity);
	auto worlpos = ToPxVec3(transform->GetWorldPosition());
	auto quat = ToPxQuat(transform->GetWorldRotation());
	return physx::PxTransform(worlpos, quat);
}

void Core::Wrapper::WrapperPhysic::ToTransform(physx::PxTransform* physXtransform, Component::Transform* transform)
{
	auto pos = Math::Vector3(physXtransform->p.x, physXtransform->p.y, physXtransform->p.z);
	auto rot = Math::Quaternion(physXtransform->q.x, physXtransform->q.y, physXtransform->q.z, physXtransform->q.w);
	transform->SetWorldPosition(pos);
	transform->SetWorldRotation(rot);
}

// ========================================= Physic Manager ========================================== //

physx::PxFilterFlags contactReportFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	// let triggers through
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where 
	// the filtermask of A contains the ID of B and vice versa.
	//if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	pairFlags = physx::PxPairFlag::eSOLVE_CONTACT | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT
		| physx::PxPairFlag::eNOTIFY_TOUCH_FOUND
		| physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS
		| physx::PxPairFlag::eNOTIFY_CONTACT_POINTS
		| physx::PxPairFlag::eMODIFY_CONTACTS;

	return physx::PxFilterFlag::eDEFAULT;
}


Core::Wrapper::WrapperPhysic::PhysicManager::~PhysicManager()
{
	Delete();
}

void Core::Wrapper::WrapperPhysic::PhysicManager::Initialize()
{
	m_defaultAllocatorCallback = new physx::PxDefaultAllocator();
	m_defaultErrorCallback = new physx::PxDefaultErrorCallback();
	m_toleranceScale = new physx::PxTolerancesScale();
	// init PhysX
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_defaultAllocatorCallback, *m_defaultErrorCallback);
	if (!m_foundation) throw("PxCreateFoundation failed!");
#ifndef NDEBUG
	m_pvd = PxCreatePvd(*m_foundation);
	m_transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_pvd->connect(*m_transport, physx::PxPvdInstrumentationFlag::eALL);
	m_toleranceScale->length = 1;        // typical length of an object
	m_toleranceScale->speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, *m_toleranceScale, true, m_pvd);
#else
	m_toleranceScale->length = 1;        // typical length of an object
	m_toleranceScale->speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, *m_toleranceScale);
#endif
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	physx::PxU32 numCores = sys_info.dwNumberOfProcessors;
	m_dispatcher = physx::PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
	m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.6f);
	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, physx::PxCookingParams(m_physics->getTolerancesScale()));
	PxInitExtensions(*m_physics, m_pvd);

	if (!m_cooking)
		PrintError("PxCreateCooking failed!");
	PrintLog("Physic Initialized");
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateScene()
{
	collisionCallback = CollisionCallback();
	m_materials.clear();
	if (m_scene)
		m_scene->release();

	ClearLists();

	physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.bounceThresholdVelocity = 1.f;
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	// Set the simulation event callback for the scene
	sceneDesc.simulationEventCallback = &collisionCallback;
	sceneDesc.contactModifyCallback = &contactModifyCallback;

	m_scene = m_physics->createScene(sceneDesc);
#ifdef _DEBUG
	m_scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	m_scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
#endif

	m_sceneClient = m_scene->getScenePvdClient();
	if (m_sceneClient)
	{
		m_sceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		m_sceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		m_sceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	PrintLog("Scene Created");
}

void Core::Wrapper::WrapperPhysic::PhysicManager::DeleteScene()
{
	if (m_scene)
	{
		m_scene->release();
		m_scene = nullptr;
	}

	collisionCallback = CollisionCallback();

	PrintLog("Scene Deleted");
}


void Core::Wrapper::WrapperPhysic::PhysicManager::Delete()
{

	PxCloseExtensions();
	// Release the PhysX objects
	if (m_cooking) {
		m_cooking->release();
		m_cooking = nullptr;
	}

	if (m_defaultMaterial) {
		m_defaultMaterial->release();
		m_defaultMaterial = nullptr;
	}

	if (m_dispatcher) {
		m_dispatcher->release();
		m_dispatcher = nullptr;
	}

	if (m_physics) {
		m_physics->release();
		m_physics = nullptr;
	}

	if (m_pvd) {
		m_pvd->release();
		m_pvd = nullptr;
	}

	if (m_transport) {
		m_transport->release();
		m_transport = nullptr;
	}

	if (m_foundation) {
		m_foundation->release();
		m_foundation = nullptr;
	}

	// Deallocate the memory for the objects created using new
	delete m_toleranceScale;
	m_toleranceScale = nullptr;

	delete m_defaultErrorCallback;
	m_defaultErrorCallback = nullptr;

	delete m_defaultAllocatorCallback;
	m_defaultAllocatorCallback = nullptr;

	PrintLog("Physic Cleaned up");

}

void Core::Wrapper::WrapperPhysic::PhysicManager::Update()
{
	if (Core::App::Get().GetGameState() != GameState::Play || Core::App::Get().timeScale <= 0)
		return;

	if (!m_scene) {
		PrintError("Scene Not Initialized");
		return;
	}

	// Update PhysX scene
	for (auto& [rb, body] : m_rigidbodies)
	{
		if (!rb) {
			continue;
		}
		physx::PxTransform globalPos = body->getGlobalPose();
		globalPos.p = ToPxVec3(rb->gameObject->transform->GetWorldPosition());
		globalPos.q = ToPxQuat(rb->gameObject->transform->GetWorldRotation());
		body->setGlobalPose(globalPos);
	}
	for (auto& [col, body] : m_staticbodies)
	{
		if (!col)
			continue;

		physx::PxTransform globalPos = body->getGlobalPose();
		globalPos.p = ToPxVec3(col->gameObject->transform->GetWorldPosition());
		globalPos.q = ToPxQuat(col->gameObject->transform->GetWorldRotation());
		body->setGlobalPose(globalPos);
	}

	auto prevCollides = collisionCallback.collides;
	collisionCallback.collides.clear();
	auto prevTriggers = collisionCallback.triggers;


	// Simulate physic
	float deltaTime = Core::Wrapper::WrapperUI::GetDeltaTime();
	float timeStep = std::fminf(0.016f, deltaTime);
	// Update the simulation
	m_scene->simulate(timeStep);
	m_scene->fetchResults(true);

	for (auto& trigger : collisionCallback.triggers)
	{
		if (!prevTriggers.count(trigger.first))
		{
			trigger.first->gameObject->OnTriggerEnter(trigger.second);
			trigger.second->gameObject->OnTriggerEnter(trigger.first);
		}
		else
		{
			trigger.first->gameObject->OnTriggerStay(trigger.second);
			trigger.second->gameObject->OnTriggerStay(trigger.first);
		}
	}

	for (auto& trigger : prevTriggers)
	{
		if (!collisionCallback.triggers.count(trigger.first))
		{
			trigger.first->gameObject->OnTriggerExit(trigger.second);
			trigger.second->gameObject->OnTriggerExit(trigger.first);
		}
	}

	// Check for collisions entering, stay and exit
	for (auto& collide : collisionCallback.collides)
	{
		if (!prevCollides.count(collide.first))
		{
			collide.first->gameObject->OnCollisionEnter(collide.second);
			collide.second->gameObject->OnCollisionEnter(collide.first);
		}
		else
		{
			collide.first->gameObject->OnCollisionStay(collide.second);
			collide.second->gameObject->OnCollisionStay(collide.first);
		}
	}
	for (auto& collide : prevCollides)
	{
		if (!collisionCallback.collides.count(collide.first))
		{
			collide.first->gameObject->OnCollisionExit(collide.second);
			collide.second->gameObject->OnCollisionExit(collide.first);
		}
	}
	for (auto& [rb, body] : m_rigidbodies)
	{
		if (!rb || !body)
			continue;

		physx::PxTransform globalPos = body->getGlobalPose();
		rb->gameObject->transform->SetWorldTransform(ToVector3(globalPos.p), ToQuaternion(globalPos.q), false);
	}
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateRigidbody(Component::Rigidbody* rb)
{
	if (m_rigidbodies.count(rb))
		return;
	m_rigidbodies[rb] = m_physics->createRigidDynamic(ToPhysXTransform(rb->gameObject->transform));
	rb->m_body = m_rigidbodies[rb];
	rb->SetKinematic(rb->isKinematic);
	rb->m_body->setSleepThreshold(0.01f);
	m_scene->addActor(*rb->m_body);
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateCube(Component::BoxCollider* box)
{
	auto finalMat = GetMaterial(box->p_physicalMaterial);
	auto newExtent = box->m_extent * box->gameObject->transform->GetWorldScale();
	physx::PxBoxGeometry geometry(ToPxVec3(newExtent));

	auto shape = m_physics->createShape(geometry, *finalMat, true);
	box->m_shape = shape;
	if (box->p_isTrigger) {
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	Component::Rigidbody* rigidbody = box->p_rigidbody.lock().get();
	if (rigidbody) {
		if (!m_rigidbodies.count(rigidbody)) {
			CreateRigidbody(rigidbody);
		}
		rigidbody->m_body->attachShape(*shape);
	}
	else
	{
		auto physXTransform = ToPhysXTransform(box->gameObject->transform);
		auto staticBody = m_physics->createRigidStatic(physXTransform);
		staticBody->attachShape(*shape);
		AddStatic(box, staticBody);
		m_scene->addActor(*staticBody);
	}
	AddCollider(box->m_shape, box);
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateSphere(Component::SphereCollider* sphere)
{
	auto finalMat = GetMaterial(sphere->p_physicalMaterial);
	physx::PxSphereGeometry geometry(sphere->gameObject->transform->GetWorldScale().x * sphere->m_radius);
	auto shape = m_physics->createShape(geometry, *finalMat, true);
	sphere->m_shape = shape;
	if (sphere->p_isTrigger) {
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}

	Component::Rigidbody* rigidbody = sphere->p_rigidbody.lock().get();
	if (rigidbody) {
		if (!m_rigidbodies.count(rigidbody)) {
			CreateRigidbody(rigidbody);
		}
		rigidbody->m_body->attachShape(*shape);
	}
	else
	{
		auto physXTransform = ToPhysXTransform(sphere->gameObject->transform);
		auto staticBody = m_physics->createRigidStatic(physXTransform);
		staticBody->attachShape(*shape);
		AddStatic(sphere, staticBody);
		m_scene->addActor(*staticBody);
	}
	AddCollider(sphere->m_shape, sphere);
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateCapsule(Component::CapsuleCollider* capsule)
{
	auto finalMat = GetMaterial(capsule->p_physicalMaterial);
	Vector3 scale = capsule->gameObject->transform->GetWorldScale();
	physx::PxCapsuleGeometry geometry(capsule->m_radius * scale.x, capsule->m_height * scale.y);

	auto shape = m_physics->createShape(geometry, *finalMat, true);
	capsule->m_shape = shape;
	if (capsule->p_isTrigger) {
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}

	Component::Rigidbody* rigidbody = capsule->p_rigidbody.lock().get();
	if (rigidbody) {
		if (!m_rigidbodies.count(rigidbody)) {
			CreateRigidbody(rigidbody);
		}
		rigidbody->m_body->attachShape(*shape);
	}
	else
	{
		auto physXTransform = ToPhysXTransform(capsule->gameObject->transform);
		auto staticBody = m_physics->createRigidStatic(physXTransform);
		staticBody->attachShape(*shape);
		AddStatic(capsule, staticBody);
		m_scene->addActor(*staticBody);
	}
	AddCollider(capsule->m_shape, capsule);
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateMesh(Component::MeshCollider* meshCollider)
{
	auto finalMat = GetMaterial(meshCollider->p_physicalMaterial);
	using namespace physx;
	if (!meshCollider->p_mesh)
		return;
	auto physXTransform = ToPhysXTransform(meshCollider->gameObject->transform);

	Component::Rigidbody* rigidbody = meshCollider->p_rigidbody.lock().get();
	auto scale = meshCollider->gameObject->transform->GetWorldScale();
	if (meshCollider->m_isConvex)
	{
		PxConvexMesh* convexMesh = nullptr;
		if (!m_convexMesh.count(meshCollider->p_mesh)) {
			// If convex mesh not found, then create it
			PxConvexMeshDesc convexDesc;

			convexDesc.points.count = (physx::PxU32)meshCollider->p_mesh->m_positions.size();
			convexDesc.points.stride = sizeof(Math::Vector3);
			convexDesc.points.data = &meshCollider->p_mesh->m_positions[0];
			convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eDISABLE_MESH_VALIDATION | PxConvexFlag::eFAST_INERTIA_COMPUTATION;

			PxDefaultMemoryOutputStream buf;
			PxConvexMeshCookingResult::Enum result;

			if (!m_cooking->cookConvexMesh(convexDesc, buf, &result)) {
				PrintError("Error With Convex Mesh : %s", meshCollider->p_mesh->GetPath().c_str());
				return;
			}
			PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			convexMesh = m_physics->createConvexMesh(input);
			m_convexMesh[meshCollider->p_mesh] = convexMesh;
		}
		else
		{
			convexMesh = m_convexMesh[meshCollider->p_mesh];
		}

		auto geometry = PxConvexMeshGeometry(convexMesh);
		geometry.scale = ToPxVec3(scale);
		PxShape* convexShape = m_physics->createShape(geometry, *finalMat, true);

		meshCollider->m_shape = convexShape;
		if (meshCollider->p_isTrigger) {
			convexShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			convexShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		if (rigidbody) {
			if (!m_rigidbodies.count(rigidbody)) {
				CreateRigidbody(rigidbody);
			}
			rigidbody->m_body->attachShape(*convexShape);
		}
		else
		{
			auto physXTransform = ToPhysXTransform(meshCollider->gameObject->transform);
			auto staticBody = m_physics->createRigidStatic(physXTransform);
			AddStatic(meshCollider, staticBody);
			staticBody->attachShape(*convexShape);
			m_scene->addActor(*staticBody);
		}
		AddCollider(meshCollider->m_shape, meshCollider);

	}
	else
	{
		physx::PxTriangleMesh* triangleMesh = nullptr;
		if (!m_triangleMesh.count(meshCollider->p_mesh)) {
			PxTriangleMeshDesc meshDesc;
			meshDesc.points.count = (physx::PxU32)meshCollider->p_mesh->m_positions.size();
			meshDesc.points.stride = sizeof(Math::Vector3);
			meshDesc.points.data = &meshCollider->p_mesh->m_positions[0];

			meshDesc.triangles.count = (physx::PxU32)meshCollider->p_mesh->m_indicesPositions.size() / 3;
			meshDesc.triangles.stride = 3 * sizeof(uint32_t);
			meshDesc.triangles.data = &meshCollider->p_mesh->m_indicesPositions[0];

			PxDefaultMemoryOutputStream writeBuffer;
			PxTriangleMeshCookingResult::Enum result;
			bool status = m_cooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
			if (!status)
			{
				// Erreur de création du mesh convexe
				PrintError("Error With Triangle Mesh : %s", meshCollider->p_mesh->GetPath().c_str());
				return;
			}
			PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
			triangleMesh = m_physics->createTriangleMesh(readBuffer);
			m_triangleMesh[meshCollider->p_mesh] = triangleMesh;
		}
		else
		{
			triangleMesh = m_triangleMesh[meshCollider->p_mesh];
		}
		auto geometry = PxTriangleMeshGeometry(triangleMesh);
		geometry.scale = PxMeshScale(ToPxVec3(scale));
		PxShape* triangleShape = m_physics->createShape(geometry, *finalMat, true);

		meshCollider->m_shape = triangleShape;
		if (meshCollider->p_isTrigger) {
			triangleShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			triangleShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		if (rigidbody) {
			if (!m_rigidbodies.count(rigidbody)) {
				CreateRigidbody(rigidbody);
			}
			rigidbody->m_body->attachShape(*triangleShape);
		}
		else
		{
			auto physXTransform = ToPhysXTransform(meshCollider->gameObject->transform);
			auto staticBody = m_physics->createRigidStatic(physXTransform);
			AddStatic(meshCollider, staticBody);
			staticBody->attachShape(*triangleShape);
			m_scene->addActor(*staticBody);
		}
		AddCollider(meshCollider->m_shape, meshCollider);
	}
}

bool Core::Wrapper::WrapperPhysic::PhysicManager::RayCast(const Math::Vector3& origin, const Math::Vector3& direction, float distanceMax, Physic::RaycastHit& hit)
{
	physx::PxRaycastBuffer hitBuffer;
	m_scene->raycast(ToPxVec3(origin), ToPxVec3(direction), distanceMax, hitBuffer);
	if (!hitBuffer.hasBlock)
		return false;
	hit = ToRayCastHit(hitBuffer);
	return true;
}
Physic::RaycastHit Core::Wrapper::WrapperPhysic::PhysicManager::ToRayCastHit(const physx::PxRaycastBuffer& hit)
{
	Physic::RaycastHit raycastHit;
	raycastHit.m_collider = GetColliderWithShape(hit.block.shape);
	raycastHit.m_hit = hit.hasBlock;
	raycastHit.m_distance = hit.block.distance;
	raycastHit.m_normal = ToVector3(hit.block.normal);
	raycastHit.m_position = ToVector3(hit.block.position);
	raycastHit.m_faceIndex = hit.block.faceIndex;
	return raycastHit;
}

Component::Collider* Core::Wrapper::WrapperPhysic::PhysicManager::GetColliderWithShape(physx::PxShape* shape)
{
	for (auto& s : m_colliderList)
	{
		if (s.second == shape)
			return s.first;
	}
	return nullptr;
}

physx::PxShape* Core::Wrapper::WrapperPhysic::PhysicManager::GetShapeWithCollider(Component::Collider* collider)
{
	if (m_colliderList.count(collider))
		return m_colliderList[collider];
	return nullptr;
}

void Core::Wrapper::WrapperPhysic::PhysicManager::AddCollider(physx::PxShape* shape, Component::Collider* collider)
{
	if (collider->gameObject)
		m_colliderList[collider] = shape;
	else
		PrintError("Failed to add Collider to list");
}

void Core::Wrapper::WrapperPhysic::PhysicManager::RemoveCollider(Component::Collider* collider)
{
	if (Core::App::Get().GetGameState() == GameState::Play && !collider->gameObject->GetScene()->IsPrefabScene()) {
		if (!collider->p_rigidbody.expired())
		{
			if (auto rb = m_rigidbodies[collider->p_rigidbody.lock().get()])
				rb->detachShape(*collider->m_shape);
		}

		// Supprimer de la liste des clés
		collisionCallback.collides.erase(collider);
		collisionCallback.triggers.erase(collider);

		// Supprimer de la liste des valeurs
		for (auto it = collisionCallback.collides.begin(); it != collisionCallback.collides.end();) {
			if (it->second == collider) {
				it = collisionCallback.collides.erase(it);
			}
			else {
				++it;
			}
		}

		for (auto it = collisionCallback.triggers.begin(); it != collisionCallback.triggers.end();) {
			if (it->second == collider) {
				it = collisionCallback.triggers.erase(it);
			}
			else {
				++it;
			}
		}


		if (m_colliderList.count(collider))
			m_colliderList.erase(collider);
		if (m_staticbodies.count(collider))
			m_staticbodies.erase(collider);
	}
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateMaterial(std::string name, float staticFriciton, float dynamicFriction, float restituion)
{
	m_materials[name] = m_physics->createMaterial(staticFriciton, dynamicFriction, restituion);
}

void Core::Wrapper::WrapperPhysic::PhysicManager::UpdateMaterial(std::string name, float staticFriciton, float dynamicFriction, float restituion)
{
	if (m_materials.count(name)) {
		m_materials[name]->setStaticFriction(std::max(staticFriciton, 0.f));
		m_materials[name]->setDynamicFriction(std::max(dynamicFriction, 0.f));
		m_materials[name]->setRestitution(std::max(restituion, 0.f));
	}
}

void Core::Wrapper::WrapperPhysic::PhysicManager::CreateJoint(Component::Constraint* fixedJoint)
{
	physx::PxRigidActor* actor = nullptr;
	physx::PxRigidActor* otherActor = nullptr;
	if (fixedJoint->p_rigidbody)
		actor = dynamic_cast<RigidDynamic*>(fixedJoint->p_rigidbody)->m_body;

	if (fixedJoint->p_otherRigidbody.lock())
		otherActor = dynamic_cast<RigidDynamic*>(fixedJoint->p_otherRigidbody.lock().get())->m_body;

	auto dif = fixedJoint->gameObject->transform->GetWorldPosition() - fixedJoint->p_otherRigidbody.lock()->gameObject->transform->GetWorldPosition();
	auto difQuat = fixedJoint->gameObject->transform->GetWorldRotation();
	physx::PxJoint* joint = NULL;
	switch (fixedJoint->m_type)
	{
	case Component::ConstraintType::Fixed:
	{
		auto firstTransform = physx::PxTransform(physx::PxIdentity);
		auto secondTransform = physx::PxTransform(ToPxVec3(dif), ToPxQuat(difQuat));
		joint = physx::PxFixedJointCreate(*m_physics, actor, firstTransform, otherActor, secondTransform);
		break;
	}
	case Component::ConstraintType::Distance:
	{
		auto firstTransform = physx::PxTransform(physx::PxIdentity);
		auto secondTransform = physx::PxTransform(physx::PxIdentity);
		joint = physx::PxDistanceJointCreate(*m_physics, actor, firstTransform, otherActor, secondTransform);
		auto distanceJoint = dynamic_cast<physx::PxDistanceJoint*>(joint);
		// Max Distance
		distanceJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
		float distanceFromPoint = fixedJoint->gameObject->transform->GetWorldPosition().GetDistanceFromPoint(fixedJoint->p_otherRigidbody.lock()->gameObject->transform->GetWorldPosition());
		distanceJoint->setMaxDistance(fixedJoint->m_setLimitToDistance ? distanceFromPoint : fixedJoint->m_distanceMinMax.y);

		// Min Distance
		distanceJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, true);
		distanceJoint->setMinDistance(fixedJoint->m_setLimitToDistance ? distanceFromPoint : fixedJoint->m_distanceMinMax.x);

		break;
	}
	case Component::ConstraintType::Spring:
	{
		auto firstTransform = physx::PxTransform(physx::PxIdentity);
		auto secondTransform = physx::PxTransform(physx::PxIdentity);
		joint = physx::PxDistanceJointCreate(*m_physics, actor, firstTransform, otherActor, secondTransform);
		auto distanceJoint = dynamic_cast<physx::PxDistanceJoint*>(joint);

		distanceJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, true);
		distanceJoint->setStiffness(fixedJoint->m_stifness);
		distanceJoint->setDamping(fixedJoint->m_damping);
		float distanceFromPoint = fixedJoint->gameObject->transform->GetWorldPosition().GetDistanceFromPoint(fixedJoint->p_otherRigidbody.lock()->gameObject->transform->GetWorldPosition());

		// Max Distance
		distanceJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
		distanceJoint->setMaxDistance(fixedJoint->m_setLimitToDistance ? distanceFromPoint : fixedJoint->m_distanceMinMax.y);


		// Min Distance
		distanceJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, true);
		//distanceJoint->setMinDistance(fixedJoint->m_setLimitToDistance ? distanceFromPoint : fixedJoint->m_distanceMinMax.x);
		distanceJoint->setMinDistance(fixedJoint->m_distanceMinMax.x);


		break;
	}
	case Component::ConstraintType::Revolute:
	{
		auto firstTransform = physx::PxTransform(physx::PxIdentity);
		auto secondTransform = physx::PxTransform(ToPxVec3(dif), ToPxQuat(difQuat));
		joint = physx::PxRevoluteJointCreate(*m_physics, actor, firstTransform, otherActor, secondTransform);
		auto revolute = dynamic_cast<physx::PxRevoluteJoint*>(joint);
		if (fixedJoint->m_enableLimit) {
			revolute->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);
			revolute->setLimit(physx::PxJointAngularLimitPair(fixedJoint->m_rotationalLimit.x * DEG2RAD, fixedJoint->m_rotationalLimit.y * DEG2RAD));
		}

		break;
	}
	case Component::ConstraintType::Spherical:
	{
		auto firstTransform = physx::PxTransform(physx::PxIdentity);
		auto secondTransform = physx::PxTransform(ToPxVec3(dif), ToPxQuat(difQuat));
		joint = physx::PxSphericalJointCreate(*m_physics, actor, firstTransform, otherActor, secondTransform);
		auto spherical = dynamic_cast<physx::PxSphericalJoint*>(joint);
		if (fixedJoint->m_enableLimit) {
			spherical->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);
			spherical->setLimitCone(physx::PxJointLimitCone(fixedJoint->m_rotationalLimit.x * DEG2RAD, fixedJoint->m_rotationalLimit.y * DEG2RAD, 0.01f));
		}
		break;
	}
	case Component::ConstraintType::Prismatic:
	{
		auto firstTransform = physx::PxTransform(physx::PxIdentity);
		auto secondTransform = physx::PxTransform(ToPxVec3(dif), ToPxQuat(difQuat));
		joint = physx::PxPrismaticJointCreate(*m_physics, actor, firstTransform, otherActor, secondTransform);
		auto prismatic = dynamic_cast<physx::PxPrismaticJoint*>(joint);
		if (fixedJoint->m_enableLimit) {
			prismatic->setPrismaticJointFlag(physx::PxPrismaticJointFlag::eLIMIT_ENABLED, true);
			prismatic->setLimit(physx::PxJointLinearLimitPair(*m_toleranceScale, fixedJoint->m_distanceMinMax.x, fixedJoint->m_distanceMinMax.y, 0.01f));
		}
		break;
	}
	default:
		break;
	}
	joint->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);
	fixedJoint->m_joint = joint;
}

void Core::Wrapper::WrapperPhysic::PhysicManager::RemoveRigidbody(Component::Rigidbody* rb)
{
	if (Core::App::Get().GetGameState() == GameState::Play && !rb->gameObject->GetScene()->IsPrefabScene()) {
		if (rb->m_body && rb->m_body->getNbShapes() != 0)
		{
			std::vector<physx::PxShape*> shapes;
			shapes.resize(rb->m_body->getNbShapes());
			rb->m_body->getShapes(&shapes[0], (physx::PxU32)shapes.size());
			for (auto& shape : shapes)
			{
				if (auto collider = GetColliderWithShape(shape))
				{
					collider->gameObject->RemoveComponent(collider);
				}
			}
		}
		if (m_rigidbodies.count(rb))
			m_rigidbodies.erase(rb);
	}
}

void Core::Wrapper::WrapperPhysic::PhysicManager::AddStatic(Component::Collider* col, physx::PxRigidStatic* body)
{
	m_staticbodies[col] = body;
}

void Core::Wrapper::WrapperPhysic::PhysicManager::ClearLists()
{
	m_rigidbodies.clear();
	m_colliderList.clear();
	m_staticbodies.clear();
}

physx::PxMaterial* Core::Wrapper::WrapperPhysic::PhysicManager::GetMaterial(Resources::PhysicMaterial* material)
{
	if (!material)
		return m_defaultMaterial;
	if (m_materials.count(material->GetPath()))
	{
		return m_materials[material->GetPath()];
	}
	else
	{
		if (auto m = ResourcesManager::Get()->GetOrLoad<Resources::PhysicMaterial>(material->GetPath()))
		{
			m->CreateMaterial();
			return m_materials[material->GetPath()];
		}
		else
		{
			return m_defaultMaterial;
		}
	}
}

void Core::Wrapper::WrapperPhysic::PhysicManager::DestroyJoint(Component::Constraint* fixedJoint)
{
	if (fixedJoint->m_joint)
	{
		fixedJoint->m_joint->release();
	}
}

// ========================================= RigidDynamic ========================================== //

Core::Wrapper::WrapperPhysic::RigidDynamic::RigidDynamic()
{

}

Core::Wrapper::WrapperPhysic::RigidDynamic::~RigidDynamic()
{

}

void Core::Wrapper::WrapperPhysic::RigidDynamic::AddForce(const Math::Vector3& force)
{
	if (!m_body)
		return;
	m_body->addForce(ToPxVec3(force));
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::ClearForce()
{
	m_body->clearForce();
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::WakeUp()
{
	m_body->wakeUp();
}

Math::Vector3 Core::Wrapper::WrapperPhysic::RigidDynamic::GetPosition()
{
	auto pos = m_body->getGlobalPose().p;
	return ToVector3(pos);
}

Math::Quaternion Core::Wrapper::WrapperPhysic::RigidDynamic::GetRotation()
{
	auto rot = m_body->getGlobalPose().q;
	return ToQuaternion(rot);
}

Math::Vector3 Core::Wrapper::WrapperPhysic::RigidDynamic::GetAngularVelocity()
{
	auto velocity = m_body->getAngularVelocity();
	return ToVector3(velocity);
}

Math::Vector3 Core::Wrapper::WrapperPhysic::RigidDynamic::GetVelocity()
{
	auto velocity = m_body->getLinearVelocity();
	return ToVector3(velocity);
}

float Core::Wrapper::WrapperPhysic::RigidDynamic::GetMass()
{
	return m_body->getMass();
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetPosition(const Math::Vector3& position)
{
	if (!m_body)
		return;
	auto globalPos = m_body->getGlobalPose();
	globalPos.p = ToPxVec3(position);
	m_body->setGlobalPose(globalPos);
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetRotation(const Math::Quaternion& rotation)
{
	if (!m_body)
		return;
	auto globalPos = m_body->getGlobalPose();
	globalPos.q = ToPxQuat(rotation);
	m_body->setGlobalPose(globalPos);
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetAngularVelocity(const Math::Vector3& angular)
{
	if (m_body)
		m_body->setAngularVelocity(ToPxVec3(angular));
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetVelocity(const Math::Vector3& velocity)
{
	if (m_body)
		m_body->setLinearVelocity(ToPxVec3(velocity));
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetMass(float mass)
{
	if (m_body)
	{
		if (mass != 0.0f) {
			physx::PxRigidBodyExt::updateMassAndInertia(*m_body, mass);
			//m_body->setMass(mass);
		}
		else {
			mass = 0.001f;
			physx::PxRigidBodyExt::updateMassAndInertia(*m_body, mass);
			//m_body->setMass(mass);
		}

	}
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetGravityEnable(bool enable)
{
	if (m_body)
		m_body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !enable);
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::FixRotation(bool fixRotationX, bool fixRotationY, bool fixRotationZ)
{
	if (!m_body)
		return;
	m_body->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, fixRotationX);
	m_body->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, fixRotationY);
	m_body->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, fixRotationZ);

}

void Core::Wrapper::WrapperPhysic::RigidDynamic::SetKinematic(bool isKinematic)
{
	if (!m_body)
		return;
	m_body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::Destroy()
{
	if (!m_body)
		return;
	if (m_body->isReleasable())
		m_body->release();
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::Detach(Component::Collider* shape)
{
	/*if (shape && shape->m_shape && m_body)
	{
		m_body->detachShape(*shape->m_shape);
	}*/
}

void Core::Wrapper::WrapperPhysic::RigidDynamic::AddTorque(const Math::Vector3& torque)
{
	if (m_body)
		m_body->addTorque(ToPxVec3(torque));
}

void Core::Wrapper::WrapperPhysic::Joint::SetMaxDistance(float max)
{
	if (!m_joint)
		return;
	if (auto dis = m_joint->is<physx::PxDistanceJoint>())
	{
		dis->setMaxDistance(max);
	}
}

void Core::Wrapper::WrapperPhysic::Joint::SetMinDistance(float min)
{
	if (!m_joint)
		return;
	if (auto dis = m_joint->is<physx::PxDistanceJoint>())
	{
		dis->setMinDistance(min);
	}
}

void Core::Wrapper::WrapperPhysic::Joint::SetDamping(float damp)
{
	if (!m_joint)
		return;
	if (auto dis = m_joint->is<physx::PxDistanceJoint>())
	{
		dis->setDamping(damp);
	}
}

void Core::Wrapper::WrapperPhysic::Joint::SetStiffness(float stif)
{
	if (!m_joint)
		return;
	if (auto dis = m_joint->is<physx::PxDistanceJoint>())
	{
		dis->setStiffness(stif);
	}
}

void Core::Wrapper::WrapperPhysic::Joint::SetDriveVelocity(float velocity)
{
	if (!m_joint)
		return;
	if (auto revolute = m_joint->is<physx::PxRevoluteJoint>())
	{
		revolute->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		revolute->setDriveVelocity(velocity);
		revolute->setDriveGearRatio(0.5f);
	}
}

Core::Wrapper::WrapperPhysic::Shape::~Shape()
{
}

void Core::Wrapper::WrapperPhysic::Shape::SetScale(const Vector3& scale)
{
	if (!m_shape)
		return;
	switch (m_shape->getGeometryType())
	{
	case physx::PxGeometryType::eBOX:
	{
		physx::PxBoxGeometry box;
		m_shape->getBoxGeometry(box);
		box.halfExtents = ToPxVec3(scale);
		m_shape->setGeometry(box);
		break;
	}
	case physx::PxGeometryType::eSPHERE:
	{
		physx::PxSphereGeometry sphere;
		m_shape->getSphereGeometry(sphere);
		sphere.radius = scale.x; // assuming uniform scaling
		m_shape->setGeometry(sphere);
		break;
	}
	case physx::PxGeometryType::eCAPSULE:
	{
		physx::PxCapsuleGeometry capsule;
		m_shape->getCapsuleGeometry(capsule);
		capsule.halfHeight = scale.x; // assuming scaling along the y-axis
		capsule.radius = scale.y; // assuming uniform scaling
		m_shape->setGeometry(capsule);
		break;
	}
	case physx::PxGeometryType::eCONVEXMESH:
	{
		physx::PxConvexMeshGeometry mesh;
		m_shape->getConvexMeshGeometry(mesh);
		mesh.scale = ToPxVec3(scale);
		m_shape->setGeometry(mesh);
		break;
	}
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		physx::PxTriangleMeshGeometry mesh;
		m_shape->getTriangleMeshGeometry(mesh);
		mesh.scale = ToPxVec3(scale);
		m_shape->setGeometry(mesh);
		break;
	}
	// add more cases for other geometry types as needed
	default:
		break;
	}
}

void Core::Wrapper::WrapperPhysic::Shape::SetOffset(const Vector3& offset)
{
	if (!m_shape)
		return;
	m_shape->setLocalPose(physx::PxTransform(ToPxVec3(offset)));
}

void Core::Wrapper::WrapperPhysic::Shape::SetLocal(const Vector3& pos, const Quaternion& rot)
{
	if (!m_shape)
		return;
	m_shape->setLocalPose(physx::PxTransform(ToPxVec3(pos), ToPxQuat(rot)));
}

void Core::Wrapper::WrapperPhysic::Shape::Destroy(Component::Collider* collider)
{
	if (m_shape && m_shape->getActor())
		m_shape->getActor()->detachShape(*m_shape);
	if (m_shape && m_shape->isReleasable())
		m_shape->release();
}

void Core::Wrapper::WrapperPhysic::Shape::SetTrigger(bool trigger)
{
	if (!m_shape)
		return;
	m_shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !trigger);
	m_shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, trigger);
}
