#include "pch.h"

#include <EditorUI/ResourcesWindow.h>

#include <Resources/ResourcesManager.h>
#include <Core/App.h>

EditorUI::ResourcesWindow::ResourcesWindow()
{
}

EditorUI::ResourcesWindow::~ResourcesWindow()
{
}

void EditorUI::ResourcesWindow::Draw()
{
	if (!p_open)
		return;
	if (WrapperUI::Begin("Resources", &p_open))
	{
		WrapperUI::Text("Nb Resources : %d", Core::App::Get().resourcesManager->GetAllResources().size());
		static TextFilter filter;
		filter.Draw("##");
		static TableFlags flags = (TableFlags)((int)TableFlags::ScrollY | (int)TableFlags::RowBg | (int)TableFlags::BordersOuter | (int)TableFlags::BordersV | (int)TableFlags::Resizable | (int)TableFlags::Reorderable | (int)TableFlags::Hideable);
		if (WrapperUI::BeginTable("table_scrolly", 3, flags))
		{
			WrapperUI::TableSetupScrollFreeze(0, 1); // Make top row always visible
			WrapperUI::TableSetupColumn("Type", TableColumnFlags::None);
			WrapperUI::TableSetupColumn("Name", TableColumnFlags::None);
			WrapperUI::TableSetupColumn("Path", TableColumnFlags::None);
			WrapperUI::TableHeadersRow();
			int row = 0; 
			for (auto res : Core::App::Get().resourcesManager->GetAllResources()) {
				if (!filter.PassFilter(res.second->GetPath().c_str()))
					continue;
				WrapperUI::TableNextRow();
				for (int column = 0; column < 3; column++)
				{
					WrapperUI::TableSetColumnIndex(column);
					switch (column)
					{
					case 0:
						WrapperUI::Text("%s", typeid(*res.second).name());
						if (WrapperUI::IsItemHovered())
						{
							WrapperUI::BeginTooltip();
							WrapperUI::Text("Should Be Loaded : %d", res.second->ShouldBeLoaded());
							WrapperUI::Text("Loaded : %d", res.second->IsLoaded());
							WrapperUI::Text("Has Been Sent : %d", res.second->HasBeenSent());
							WrapperUI::EndTooltip();
						}
						break;
					case 1:
						WrapperUI::TextUnformatted(res.second->GetName().c_str());
						break;
					case 2:
						WrapperUI::TextUnformatted(res.second->GetPath().c_str());
						break;
					}
				}
				row++;
			}
			WrapperUI::EndTable();
		}

	}
	WrapperUI::End();
}
