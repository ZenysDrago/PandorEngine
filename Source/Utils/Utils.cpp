#include "pch.h"
#include <regex>
#include <Utils\Utils.h>
#include <random>
#include <Render/Camera.h>
#include <Resources/Model.h>
#include <Resources/Mesh.h>
#include <Components/Transform.h>

void Utils::ToPath(std::string& path)
{
	std::replace(path.begin(), path.end(), '\\', '/');
	path = std::regex_replace(path, std::regex("//"), "/");
	std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) { return tolower(c); });
	std::size_t found = path.find("assets/");
	if (found != std::string::npos) {
		path = path.substr(found);
	}
}

float Utils::RandomFloat(float min, float max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(min, max);

	return dis(gen);
}

Math::Vector3 Utils::RandomPointInSphere(float radius)
{
	// Initialisation du générateur de nombres aléatoires
	static std::mt19937_64 generator(time(nullptr));

	// Génération de deux nombres aléatoires entre 0 et 1
	std::uniform_real_distribution<float> distribution(0.0, 1.0);
	float u = distribution(generator);
	float v = distribution(generator);

	// Calcul de deux angles aléatoires en radians
	float theta = 2.0f * PI * u;
	float phi = acosf(2.0f * v - 1.0f);

	// Calcul de la position aléatoire à l'intérieur de la sphère
	return { radius * sinf(phi) * cosf(theta), radius * sinf(phi) * sinf(theta), radius * cosf(phi) };
}



Math::Vector3 Utils::RandomDirection3D(float angle, Vector3 axis)
{
	angle *= DEG2RAD;

	// Calculate the cosine of half the cone angle
	float cosHalfAngle = cos(angle / 2.0f);

	// Generate two random numbers between -1 and 1
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
	float u1 = dis(gen);
	float u2 = dis(gen);

	// Ensure the generated direction lies within the cone by rejecting
	// directions with a cosine greater than the cosine of half the cone angle
	while (u1 * u1 + u2 * u2 >= 1.0f || u2 < cosHalfAngle)
	{
		u1 = dis(gen);
		u2 = dis(gen);
	}

	// Calculate the direction vector
	float sinHalfAngle = sqrt(1.0f - cosHalfAngle * cosHalfAngle);
	Vector3 v1 = axis.GetNormalized();
	Vector3 v2 = v1.Cross(Vector3(u1, u2, sqrt(1.0f - u1 * u1 - u2 * u2))).GetNormalized();
	Vector3 direction = (v1 * cosHalfAngle + v2 * sinHalfAngle).GetNormalized();

	return direction;
}


void Utils::GetRandomPositionAndDirection(float radius, float angle, Vector3& position, Vector3& direction)
{
	auto randomDirection = Utils::RandomPointInSphere(radius).GetNormalized();

	// Convert cone angle from degrees to radians
	float coneAngleRad = angle * DEG2RAD;

	// Calculate the maximum y coordinate based on the cone angle
	float maxY = std::cosf(coneAngleRad);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	// Clamp the random direction to the cone boundaries
	randomDirection.z = Arithmetics::Lerp(maxY, 1.f, dis(gen));
	randomDirection.Normalize();

	direction =  randomDirection;
}

Math::Vector3 Utils::RandomDirectionCone(Vector3 axis, float topRadius, float apexAngle)
{
	float angle = (apexAngle * 10.f) / 360.f;
	Math::Vector3 randomdir = Math::Vector3(
		Utils::RandomFloat(-angle, angle),
		Utils::RandomFloat(-angle, angle),
		Utils::RandomFloat(-angle, angle)
	);
	return axis + randomdir * 1.5f;
}

Math::Vector3 Utils::RandomPointOnCircle(float radius, Vector3 axis)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI);

	float angle = angleDist(gen);
	float x = radius * std::cos(angle);
	float y = radius * std::sin(angle);

	Math::Vector3 point = axis * x + axis.Cross(Math::Vector3(0.0f, 0.0f, 1.0f)) * y;
	point.Normalize();
	return point;
}

Math::Vector3 Utils::RandomPointOnRectangle(Vector3 scale)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> disX(-scale.x/2.f, scale.x / 2.f);
	std::uniform_real_distribution<float> disY(-scale.y / 2.f, scale.y / 2.f);

	Vector3 randomPos;
	randomPos.x = disX(gen);
	randomPos.y = disY(gen);

	return randomPos;
}

std::vector<float> Utils::GetDemiSphereVertices(float radius, bool up)
{
	std::vector<Math::Vector3> v;
	std::vector<Math::Vector3> n;
	std::vector<float> vOut;
	float r = radius;
	int mLat = 10;
	int mLon = 20;

	for (int j = 0; j < mLat; ++j)
	{
		float theta0;
		float theta1;
		if (!up)
		{
			theta0 = ((j + 0) / (float)mLat) * PI + PI / 2;
			theta1 = ((j + 1) / (float)mLat) * PI + PI / 2;
		}
		else
		{
			theta0 = ((j + 0) / (float)mLat) * PI / 2;
			theta1 = ((j + 1) / (float)mLat) * PI / 2;
		}

		for (int i = 0; i < mLon; ++i)
		{
			float phi0 = ((i + 0) / (float)mLon) * 2.f * PI;
			float phi1 = ((i + 1) / (float)mLon) * 2.f * PI;

			Math::Vector3 c0 = Vector3::GetSphericalCoords(r, theta0, phi0);
			Math::Vector3 c1 = Vector3::GetSphericalCoords(r, theta0, phi1);
			Math::Vector3 c2 = Vector3::GetSphericalCoords(r, theta1, phi1);
			Math::Vector3 c3 = Vector3::GetSphericalCoords(r, theta1, phi0);

			v.push_back(c0);
			v.push_back(c1);
			v.push_back(c2);

			v.push_back(c0);
			v.push_back(c2);
			v.push_back(c3);

			n.push_back(c0.GetNormalized());
			n.push_back(c1.GetNormalized());
			n.push_back(c2.GetNormalized());

			n.push_back(c0.GetNormalized());
			n.push_back(c2.GetNormalized());
			n.push_back(c3.GetNormalized());
		}
	}

	for (int k = 0; k < v.size(); k++)
	{
		vOut.push_back(v[k].x);
		vOut.push_back(v[k].y);
		vOut.push_back(v[k].z);

		vOut.push_back(n[k].x);
		vOut.push_back(n[k].y);
		vOut.push_back(n[k].z);
	}

	return vOut;
}

std::vector<float> Utils::GetCylinderVertices(float radius, float height)
{
	int res = 20;
	std::vector<Math::Vector3> v;
	std::vector<Math::Vector3> n;
	std::vector<float> vOut;
	float r = radius;
	int mLon = 20;

	float theta0 = ((10 + 0) / (float)20) * PI;
	float theta1 = ((10 + 1) / (float)20) * PI;
	for (int i = 0; i < mLon; ++i)
	{
		float phi0 = ((i + 0) / (float)mLon) * 2.f * PI;
		float phi1 = ((i + 1) / (float)mLon) * 2.f * PI;

		Math::Vector3 c0 = Vector3::GetSphericalCoords(r, theta0, phi0);
		Math::Vector3 c1 = Vector3::GetSphericalCoords(r, theta0, phi1);
		Math::Vector3 c2 = Vector3::GetSphericalCoords(r, theta1, phi1);
		Math::Vector3 c3 = Vector3::GetSphericalCoords(r, theta1, phi0);

		c0.y += (height / 2);
		c1.y += (height / 2);
		c2.y -= (height / 2);
		c3.y -= (height / 2);

		v.push_back(c0);
		v.push_back(c1);
		v.push_back(c2);

		v.push_back(c0);
		v.push_back(c2);
		v.push_back(c3);

		n.push_back(c0.GetNormalized());
		n.push_back(c1.GetNormalized());
		n.push_back(c2.GetNormalized());

		n.push_back(c0.GetNormalized());
		n.push_back(c2.GetNormalized());
		n.push_back(c3.GetNormalized());
	}

	for (int k = 0; k < v.size(); k++)
	{
		vOut.push_back(v[k].x);
		vOut.push_back(v[k].y);
		vOut.push_back(v[k].z);

		vOut.push_back(n[k].x);
		vOut.push_back(n[k].y);
		vOut.push_back(n[k].z);
	}

	return vOut;
}

bool Utils::Sphere::isOnFrustum(const Render::Frustum& camFrustum, Component::Transform* transform)
{
	//Get global scale thanks to our transform
	const Vector3 globalScale = transform->GetWorldScale();

	//Get our global center with process it with the global model matrix of our transform
	const Vector3 globalCenter{ transform->GetModelMatrix().GetTransposed() * Vector4(center, 1.f) };

	//To wrap correctly our shape, we need the maximum scale scalar.
	const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

	//Max scale is assuming for the diameter. So, we need the half to apply it to our radius
	Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));


	return (globalSphere.isOnOrForwardPlane(camFrustum.planes[0]) &&
		globalSphere.isOnOrForwardPlane(camFrustum.planes[1]) &&
		globalSphere.isOnOrForwardPlane(camFrustum.planes[2]) &&
		globalSphere.isOnOrForwardPlane(camFrustum.planes[3]) &&
		globalSphere.isOnOrForwardPlane(camFrustum.planes[4]) &&
		globalSphere.isOnOrForwardPlane(camFrustum.planes[5]));

}

Utils::Sphere* Utils::Sphere::GenerateSphereBV(Resources::Mesh* mesh)
{
	return new Sphere((mesh->MaxAABB() + mesh->MinAABB()) * 0.5f, (mesh->MinAABB() - mesh->MaxAABB()).Length());
}

bool Utils::AABB::isOnFrustum(const Render::Frustum& camFrustum, Component::Transform* transform)
{
	//Get global scale thanks to our transform
	const Vector3 globalCenter{ transform->GetModelMatrix().GetTransposed() * Vector4(center, 1.f) };

	const Vector3 globalScale{ transform->GetWorldScale() };

	// Scaled orientation
	const Vector3 right = transform->GetRightVector() * extents.x;
	const Vector3 up = transform->GetUpVector() * extents.y;
	const Vector3 forward = transform->GetForwardVector() * extents.z;

	const float newIi = std::abs(Vector3{ 1.f, 0.f, 0.f }.Dot(right)) +
		std::abs(Vector3{ 1.f, 0.f, 0.f }.Dot(up)) +
		std::abs(Vector3{ 1.f, 0.f, 0.f }.Dot(forward));

	const float newIj = std::abs(Vector3{ 0.f, 1.f, 0.f }.Dot(right)) +
		std::abs(Vector3{ 0.f, 1.f, 0.f }.Dot(up)) +
		std::abs(Vector3{ 0.f, 1.f, 0.f }.Dot(forward));

	const float newIk = std::abs(Vector3{ 0.f, 0.f, 1.f }.Dot(right)) +
		std::abs(Vector3{ 0.f, 0.f, 1.f }.Dot(up)) +
		std::abs(Vector3{ 0.f, 0.f, 1.f }.Dot(forward));

	//We not need to divise scale because it's based on the half extention of the AABB
	const AABB globalAABB(globalCenter, globalScale.x * newIi, globalScale.y * newIj, globalScale.z * newIk);

	return (globalAABB.isOnOrForwardPlane(camFrustum.planes[0]) &&
		globalAABB.isOnOrForwardPlane(camFrustum.planes[1]) &&
		globalAABB.isOnOrForwardPlane(camFrustum.planes[2]) &&
		globalAABB.isOnOrForwardPlane(camFrustum.planes[3]) &&
		globalAABB.isOnOrForwardPlane(camFrustum.planes[4]) &&
		globalAABB.isOnOrForwardPlane(camFrustum.planes[5]));
}

bool Utils::AABB::isOnOrForwardPlane(const Math::MathUtils::Plane& plane) const
{

	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extents.x * std::abs(plane.normal.x) +
		extents.y * std::abs(plane.normal.y) + extents.z * std::abs(plane.normal.z);

	return -r <= plane.GetDistanceToPoint(center);
}

Utils::AABB* Utils::AABB::GenerateAABB(Resources::Mesh* mesh)
{
	return new AABB(mesh->MinAABB(), mesh->MaxAABB());
}

bool Utils::IsOnlyLetters(const char* str) {
	while (*str) {
		if (!std::isalpha(*str)) {
			return false;
		}
		str++;
	}
	return true;
}

std::string Utils::OpenFile(const char* filter, Core::Wrapper::WrapperWindow* window)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = window->GetWin32Window();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	return std::string();
}

std::string Utils::SaveFile(const char* filter, Core::Wrapper::WrapperWindow* window)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = window->GetWin32Window();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	return std::string();
}
