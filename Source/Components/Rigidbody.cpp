#include "pch.h"

#include <Components/Rigidbody.h>
#include <Core/GameObject.h>

#include <Core/App.h>

Component::Rigidbody::Rigidbody()
{

}

Component::Rigidbody::~Rigidbody()
{
}

void Component::Rigidbody::Awake()
{
	Core::App::Get().physic->CreateRigidbody(this);
}

void Component::Rigidbody::Start()
{
}

void Component::Rigidbody::ShowInInspector()
{
	if (WrapperUI::DragFloat("Mass", &mass, 0.1f, 0.01f))
		SetMass(mass);
	if (WrapperUI::Checkbox("Use Gravity", &useGravity))
		SetGravityEnable(useGravity);
	if (WrapperUI::Checkbox("Is Kinematic", &isKinematic))
		this->SetKinematic(isKinematic);
	WrapperUI::Dummy(Vector2(2, 2));
	WrapperUI::TextUnformatted("Fix Rotation:");
	bool change = false;
	change |= WrapperUI::Checkbox("X", &fixedRotationX);
	WrapperUI::SameLine();
	change |= WrapperUI::Checkbox("Y", &fixedRotationY);
	WrapperUI::SameLine();
	change |= WrapperUI::Checkbox("Z", &fixedRotationZ);
	if (change)
		this->FixRotation(fixedRotationX, fixedRotationY, fixedRotationZ);
	if (Core::App::Get().GetGameState() == Core::GameState::Play) {
		WrapperUI::Separator();
		if (WrapperUI::DragFloat3("Velocity", &velocity.x, 0.1f))
			SetVelocity(velocity);
		if (WrapperUI::DragFloat3("Angular Velocity", &angularVelocity.x, 0.1f))
			SetAngularVelocity(angularVelocity);
	}
}

void Component::Rigidbody::GameUpdate()
{
	if (BodyInitialized()) {
		this->velocity = GetVelocity();
		this->angularVelocity = GetAngularVelocity();
	}
}

void Component::Rigidbody::SetParameters()
{
	if (BodyInitialized())
	{
		this->SetMass(mass);
		this->SetGravityEnable(useGravity);
		this->FixRotation(fixedRotationX, fixedRotationY, fixedRotationZ);
		this->SetKinematic(isKinematic);
	}
}

void Component::Rigidbody::OnDestroy()
{
	Core::App::Get().physic->RemoveRigidbody(this);
	Destroy();
}

std::ostream& Component::Rigidbody::operator<<(std::ostream& os)
{
	os << mass << "\n";
	os << useGravity << "\n";
	os << isKinematic << "\n";
	os << fixedRotationX << "\n";
	os << fixedRotationY << "\n";
	os << fixedRotationZ << "\n";
	return os;
}

void Component::Rigidbody::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		mass = std::stof(line);

	if (getline(sceneFile, line) && line != "end")
		useGravity = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		isKinematic = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		fixedRotationX = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		fixedRotationY = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		fixedRotationZ = (bool)std::atoi(line.c_str());

	while (getline(sceneFile, line) && line != "end") {}
}

Component::BaseComponent* Component::Rigidbody::Clone() const
{
	auto rb = new Rigidbody(*this);
	rb->ResetBody();
	return rb;
}

