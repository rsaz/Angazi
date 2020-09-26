#include "Precompiled.h"
#include "Editor.h"

#include "Component.h"
#include "GameWorld.h"
#include "GameObject.h"
#include "ImGui/Inc/imgui.h"
#include "Service.h"
#include "Angazi.h"

using namespace Angazi;

Editor::Editor(GameWorld& world)
	:mWorld(world)
{
	mDebugLogHandlerId = Core::OnDebugLog += [this](auto str) {mEditorLogView.AddLog(str.c_str()); };
}

Editor::~Editor()
{
	Core::OnDebugLog -= mDebugLogHandlerId;
}

void Editor::Show()
{
	ShowMenuBar();
	ShowModals();
	ShowMainWindowWithDockSpace();

	ImGui::Begin("Style");
	ImGui::ShowStyleEditor();
	ImGui::End();

	ShowWorldView();
	ShowInspectorView();
	ShowLogView();


}

void Editor::ShowWorldView()
{
	ImGui::Begin("Scene Hierarchy");
	if (ImGui::TreeNode("Services"))
	{
		for (auto& service : mWorld.mServices)
		{
			if (ImGui::Selectable(service->GetMetaClass()->GetName(), service.get() == mSelectedService))
			{
				mSelectedService = service.get();
				mSelectedGameObject = nullptr;
			}
		}
		ImGui::TreePop();
	}
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Game Objects"))
	{
		for (auto gameObject : mWorld.mUpdateList)
		{
			if (ImGui::Selectable(gameObject->GetName().c_str(), gameObject == mSelectedGameObject))
			{
				mSelectedGameObject = gameObject;
				mSelectedService = nullptr;
			}
		}
		ImGui::TreePop();
	}
	ImGui::End();
}

void Editor::ShowInspectorView()
{
	ImGui::Begin("Inspector");
	if (mSelectedService)
	{
		mSelectedService->ShowInspectorProperties();
	}
	else if (mSelectedGameObject)
	{
		ImGui::Text(mSelectedGameObject->GetName().c_str());
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Text("Components");
		ImGui::NewLine();
		for (auto& component : mSelectedGameObject->mComponents)
		{
			component->ShowInspectorProperties();
			//auto metaClass = component->GetMetaClass();
			//if (ImGui::CollapsingHeader(metaClass->GetName(), ImGuiTreeNodeFlags_DefaultOpen))
			//{
			//	for (size_t i = 0; i < metaClass->GetFieldCount(); i++)
			//	{
			//		auto metaField = metaClass->GetField(i);
			//		if (metaField->GetMetaType() == Core::Meta::GetMetaType<Math::Vector3>())
			//		{
			//			auto data = (float*)metaField->GetFieldInstance(component.get());
			//			ImGui::DragFloat3(metaField->GetName(), data);
			//		}
			//	}
			//}
		}
	}
	ImGui::End();
}

void Editor::ShowLogView()
{
	mEditorLogView.Draw("Log");
}

bool Editor::ShouldQuit() const
{
	return mRequestQuit;
}

void Editor::ShowMenuBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.95f);
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ShowFileMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ShowEditMenu();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleVar(1);

}

void Editor::ShowFileMenu()
{
	if (ImGui::MenuItem("New Scene", "Ctrl+N", false, true))
		showMessageBoxNew= true;
	if (ImGui::MenuItem("Open Scene", "Ctrl+O", false, true))
		showMessageBoxOpen = true;
	if (ImGui::MenuItem("Save", "Ctrl+S", false, true))
		Save();
	if (ImGui::MenuItem("Save As..", "Ctrl+Shit+S", false, true))
		SaveAs();
	if (ImGui::MenuItem("Exit", nullptr, false))
		showMessageBoxExit = true;

	ImGui::Separator();
}

void Editor::ShowEditMenu()
{

}

void Editor::ShowMainWindowWithDockSpace()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	const ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("MainWindow", nullptr, windowFlags);
	ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	ImGui::End();

	ImGui::PopStyleVar(3);
}

void Editor::ShowModals()
{
	if (showMessageBoxExit)
	{
		ImGui::OpenPopup("Quit?");
		showMessageBoxExit = false;
	}
	if (showMessageBoxNew)
	{
		ImGui::OpenPopup("New?");
		showMessageBoxNew = false;
	}
	if (showMessageBoxOpen)
	{
		ImGui::OpenPopup("Open?");
		showMessageBoxOpen = false;
	}


	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Quit?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to save\nbefore exiting?");
		ImGui::Separator();

		if (ImGui::Button("Yes", ImVec2(50, 0)))
		{
			Save();
			Angazi::MainApp().Quit();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(50, 0)))
		{
			ImGui::CloseCurrentPopup();
			Angazi::MainApp().Quit();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(50, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("New?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to save\nbefore creating a new scene?");
		ImGui::Separator();

		if (ImGui::Button("Yes", ImVec2(50, 0)))
		{
			Save();
			New();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(50, 0)))
		{
			ImGui::CloseCurrentPopup();
			New();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(50, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Open?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to save\nbefore opening a new scene?");
		ImGui::Separator();

		if (ImGui::Button("Yes", ImVec2(50, 0)))
		{
			Save();
			Open();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(50, 0)))
		{
			ImGui::CloseCurrentPopup();
			Open();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(50, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Editor::New()
{
	mWorld.UnloadScene();
}

void Editor::Open()
{
	char filePath[MAX_PATH]{};
	const char* title = "Open Scene";
	const char* filter = "Json Files (*.json)\0*.json";
	if (MainApp().OpenFileDialog(filePath, title, filter))
	{
		std::filesystem::path openPath = filePath;
		if (std::filesystem::exists(openPath))
		{
			mWorld.UnloadScene();
			mWorld.LoadScene(openPath);
		}
	}
}

void Editor::Save()
{
	if (std::filesystem::exists(mWorld.mSceneFilePath))
		mWorld.SaveScene();
	else
		SaveAs();
}

void Editor::SaveAs()
{
	char filePath[MAX_PATH]{};
	const char* title = "Save Scene";
	const char* filter = "Json Files (*.json)\0*.json";
	if (MainApp().SaveFileDialog(filePath, title, filter))
	{
		std::filesystem::path savePath = filePath;
		if (savePath.extension().empty())
			savePath += ".json";
		mWorld.SaveScene(savePath);
	}
}

void Editor::Exit()
{
	ImGui::OpenPopup("Quit?");
	Angazi::MainApp().Quit();
}
