#include "Precompiled.h"
#include "ColliderComponent.h"

#include "ImGui/Inc/imgui.h"
#include "GameObject.h"
#include "TransformComponent.h"

using namespace Angazi;
using namespace Angazi::Graphics;

META_DERIVED_BEGIN(ColliderComponent, Component)
	META_FIELD_BEGIN
		META_FIELD(center, "Center")
		META_FIELD(extend, "Extend")
	META_FIELD_END
META_CLASS_END;

void Angazi::ColliderComponent::Initialize()
{
	mTransformComponent = GetGameObject().GetComponent<TransformComponent>();
}

void ColliderComponent::DebugUI()
{
	auto aabb = GetAABB();
	SimpleDraw::AddAABB(aabb.center, aabb.extend, Graphics::Colors::LightGreen);
}

void ColliderComponent::ShowInspectorProperties()
{
	if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Center", &center.x);
		ImGui::DragFloat3("Extend", &extend.x);
	}
}

Math::AABB Angazi::ColliderComponent::GetAABB() const
{
	// This is incorrect if we have orientation as well
	auto translation = mTransformComponent->position;
	return { translation + center,extend };
}
