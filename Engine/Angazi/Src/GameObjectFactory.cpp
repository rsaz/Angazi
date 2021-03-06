#include "Precompiled.h"
#include "GameObjectFactory.h"

#include "Component.h"
#include "TransformComponent.h"

using namespace Angazi;

GameObject* GameObjectFactory::Create(GameObjectAllocator & allocator, std::filesystem::path templateFileName)
{
	auto gameObject = allocator.New();
	if (gameObject && std::filesystem::exists(templateFileName))
	{
		FILE *file = nullptr;
		fopen_s(&file, templateFileName.u8string().c_str(), "r");

		char readBuffer[65536];
		rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

		rapidjson::Document document;
		document.ParseStream(is);

		if (document.HasMember("GameObject") && document["GameObject"].IsObject())
		{
			auto jsonObject = document["GameObject"].GetObjectW();
			if (jsonObject.HasMember("Components") && jsonObject["Components"].IsObject())
			{
				auto components = jsonObject["Components"].GetObjectW();
				for (auto& component : components)
				{
					auto metaClass = Core::Meta::FindMetaClass(component.name.GetString());
					auto newComponent = gameObject->AddComponent(metaClass);
					metaClass->Deserialize(newComponent, component.value);
				}
			}
		}
		fclose(file);
	}
	if (gameObject && !gameObject->GetComponent<TransformComponent>())
		gameObject->AddComponent<TransformComponent>();
	return gameObject;
}

void GameObjectFactory::Destory(GameObjectAllocator & allocator, GameObject * gameObject)
{
	allocator.Delete(gameObject);
}
