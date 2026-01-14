#include "Drawing.h"
#include <string>
#include <algorithm>
#include <iostream>

LPCSTR Drawing::lpWindowName = "PE Entropy";
ImVec2 Drawing::vWindowSize = { 750, 450 };
ImGuiWindowFlags Drawing::WindowFlags = 0;
bool Drawing::bDraw = true;

void Drawing::Active() { bDraw = true; }
bool Drawing::isActive() { return bDraw; }

using namespace EntropyEngine;

void Drawing::Draw()
{
	if (!isActive())
		return;

	ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints({ 550, 430 }, { FLT_MAX, FLT_MAX });
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin(lpWindowName, &bDraw, WindowFlags);

	auto windowSize = ImGui::GetWindowSize();
	drawToolbar();
	ImGui::Separator();

	static float leftPanelRatio = 0.15f;
	static float leftPanelWidth = windowSize.x * leftPanelRatio;

	ImGui::BeginChild("##Sections", { leftPanelWidth, 0 }, true, 0);
	drawSections();
	ImGui::EndChild();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
	ImGui::SameLine();
	auto oldCursor = ImGui::GetCursorPos();
	ImGui::SetCursorPos({ oldCursor.x + 2.5f, oldCursor.y });
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SetCursorPos(oldCursor);
	ImGui::SameLine();

	float splitterHeight = windowSize.y - ImGui::GetCursorPos().y - 8;
	ImGui::InvisibleButton("##vsplitter", { 4.0f, splitterHeight });
	if (ImGui::IsItemActive()) {
		leftPanelWidth += ImGui::GetIO().MouseDelta.x;
		leftPanelRatio = leftPanelWidth / windowSize.x;
	}
	leftPanelWidth = std::clamp(windowSize.x * leftPanelRatio, 0.15f * windowSize.x, 0.50f * windowSize.x);
	ImGui::SameLine();
	ImGui::PopStyleVar();

	ImGui::BeginChild("##EntropyData", { 0, 0 }, true, 0);
	drawEntropyData();
	ImGui::EndChild();

	ImGui::End();
}

void Drawing::drawToolbar()
{
	auto windowSize = ImGui::GetWindowSize();
	static int blockSizeType = 0;
	static std::vector<int> blockSizes = { 128, 256, 512, 1024 };
	
	ImGui::Text("Settings: "); ImGui::SameLine();
	ImGui::Checkbox("Dark Mode", &Settings::darkMode);
	ImGui::SameLine();
	ImGui::Checkbox("Show Console", &Settings::showConsole);
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::Combo(("Block Size"), &blockSizeType, ("128B - Fine\0" "256B - Balanced\0" "512B - Smooth\0" "1024 - Coarse\0"));
	Settings::blockSize = blockSizes[blockSizeType];

	ImGui::Text("File: "); ImGui::SameLine();
	ImGui::PushItemWidth(windowSize.x - 18.7f * ImGui::GetStyle().ItemSpacing.x);
	if (ImGui::InputText("##FileDirectory", Settings::fileDirectoryBuf, IM_ARRAYSIZE(Settings::fileDirectoryBuf))) {
		Settings::fileDirectory = std::string(Settings::fileDirectoryBuf);
	}
	ImGui::SameLine();

	if (ImGui::Button("...")) EntropyEngine::browseForFile();
	ImGui::SameLine();
	if (ImGui::Button("Scan")) EntropyEngine::scan();
}

void Drawing::drawSections()
{
	ImGui::Text("Sections");
	ImGui::Separator();
	if (!Variables::sections.empty()) {
		bool allSectionsSelected = (Variables::selectedSectionIndex == -2);
		if (ImGui::Selectable("All Sections", allSectionsSelected)) {
			Variables::selectedSectionIndex = -2;
		}
	}
	for (int i = 0; i < Variables::sections.size(); i++) {
		if (ImGui::Selectable(Variables::sections[i].name.c_str(), Variables::selectedSectionIndex == i))
			Variables::selectedSectionIndex = i;
	}
}

void Drawing::drawEntropyLegend()
{
	ImGui::Text("Entropy Color Legend:");
	ImGui::SameLine();
	ImVec2 legendSize(200, 5);
	auto pos = ImGui::GetCursorScreenPos();
	auto drawList = ImGui::GetWindowDrawList();

	for (int i = 0; i < (int)legendSize.x; i++) {
		float t = i / legendSize.x;
		drawList->AddLine({ pos.x + i, pos.y }, { pos.x + i, pos.y + legendSize.y }, ImColor({ t, 0.f, 1.f - t, 1.f }));
	}

	ImGui::Dummy(legendSize);
	ImGui::SameLine();
	pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos({ pos.x - legendSize.x - 5, pos.y });
	ImGui::Text("0");
	ImGui::SameLine();
	pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos({ pos.x + legendSize.x - 25, pos.y });
	ImGui::Text("8");
}

void Drawing::drawEntropyGraphs(const SectionEntropyData& entropyData, int blockWidth)
{
	if (entropyData.blocks.empty()) return;

	int n = static_cast<int>(entropyData.blocks.size());
	std::vector<float> heat(n), bars(n), line(n);
	for (int i = 0; i < n; ++i) heat[i] = bars[i] = line[i] = static_cast<float>(entropyData.blocks[i].entropy);

	auto windowSize = ImGui::GetWindowSize();
	static int selectedBlockIdx = -1;
	static int lastRva = -1;
	if (lastRva != entropyData.sectionRva) {
		selectedBlockIdx = -1;
		lastRva = entropyData.sectionRva;
	}

	if (ImPlot::BeginPlot("Entropy Line Graph###line", { -1, windowSize.y * 0.5f }, ImPlotFlags_None)) {
		ImPlot::SetupAxis(ImAxis_X1, "Block");
		ImPlot::SetupAxis(ImAxis_Y1, "Entropy", ImPlotAxisFlags_Lock);
		ImPlot::SetupAxesLimits(entropyData.sectionRva, entropyData.sectionRva+entropyData.sectionSize, 0, 8);

		for (int i = 0; i < n - 1; ++i) {
			double x[2] = { static_cast<double>(entropyData.blocks[i].rva), static_cast<double>(entropyData.blocks[i + 1].rva) };
			double y[2] = { entropyData.blocks[i].entropy, entropyData.blocks[i + 1].entropy };
			float t = static_cast<float>(entropyData.blocks[i].entropy / 8.0f);
			ImPlot::PushStyleColor(ImPlotCol_Line, { t, 0.f, 1.f - t, 1.f });
			ImPlot::PlotLine("", x, y, 2, 0, entropyData.sectionSize);
			ImPlot::PopStyleColor();
		}

		static bool mouseDown = false;
		static ImPlotPoint mouseDownPos;
		if (ImPlot::IsPlotHovered()) {
			ImPlotPoint mouse = ImPlot::GetPlotMousePos();

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				mouseDown = true;
				mouseDownPos = mouse;
			}

			int blockIdx = std::clamp(static_cast<int>((mouse.x - entropyData.sectionRva) / Settings::blockSize), 0, n - 1);
			auto& block = entropyData.blocks[blockIdx];

			ImGui::BeginTooltip();
			ImGui::Text("VA: 0x%llX", block.rva + Variables::imageBase);
			ImGui::Text("RVA: 0x%llX", block.rva);
			ImGui::Text("Entropy: %.3f", block.entropy);
			ImGui::Text("Size: %u bytes", block.size);
			ImGui::EndTooltip();

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && mouseDown) {
				float dx = mouse.x - mouseDownPos.x;
				float dy = mouse.y - mouseDownPos.y;
				if (sqrtf(dx * dx + dy * dy) < 10.0f) selectedBlockIdx = blockIdx;
				mouseDown = false;
			}
		}

		if (selectedBlockIdx != -1) {
			auto& block = entropyData.blocks[selectedBlockIdx];
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f, { 1,1,1,1 }, 2.0f, { 1,1,1,1 });
			double x = block.rva, y = block.entropy;
			ImPlot::PlotScatter("##Selected Block", &x, &y, 1, ImPlotScatterFlags_None, 0, 1);
		}
	}
	ImPlot::EndPlot();

	ImGui::Separator();
	if (selectedBlockIdx != -1) {
		auto& block = entropyData.blocks[selectedBlockIdx];
		ImGui::Text("Selected Block Info:");
		ImGui::Text("Entropy: %.3f", block.entropy);
		ImGui::Text("Size: %u bytes", block.size);
		ImGui::Text("Relative Virtual Address (Hex): "); ImGui::SameLine();
		char rvaBuffer[32]; snprintf(rvaBuffer, sizeof(rvaBuffer), "0x%llX", block.rva);
		ImGui::SetNextItemWidth(150); ImGui::InputText("##RVA (hex)", rvaBuffer, sizeof(rvaBuffer), ImGuiInputTextFlags_ReadOnly);

		ImGui::Text("Virtual Address (Hex): "); ImGui::SameLine();
		char vaBuffer[32]; snprintf(vaBuffer, sizeof(vaBuffer), "0x%llX", Variables::imageBase + block.rva);
		ImGui::SetNextItemWidth(150); ImGui::InputText("##VA (hex)", vaBuffer, sizeof(vaBuffer), ImGuiInputTextFlags_ReadOnly);
	}
}

void Drawing::drawEntropyData()
{
	ImGui::Text("Entropy Data");
	ImGui::Separator();

	if (Variables::errorType != 99) {
		EntropyEngine::drawError();
		return;
	}
	if (Variables::selectedSectionIndex == -1) {
		ImGui::PushStyleColor(ImGuiCol_Text, { 1,1,0,1 });
		ImGui::Text("Select a section!");
		ImGui::PopStyleColor();
		return;
	}

	static int lastSection = -1;
	static int lastBlockSize = -1;
	static SectionEntropyData entropyData;
	std::string sectionName = EntropyEngine::selectedSectionName();
	ImGui::Text("Current Section: %s", sectionName.c_str());

	drawEntropyLegend();

	if (lastSection != Variables::selectedSectionIndex || lastBlockSize != Settings::blockSize) {
		if (Variables::selectedSectionIndex == -2) {
			entropyData = EntropyEngine::getAllSectionsEntropy();
		}
		else {
			entropyData = EntropyEngine::getCurrentSectionData();
		}
		lastSection = Variables::selectedSectionIndex;
		lastBlockSize = Settings::blockSize;
	}

	drawEntropyGraphs(entropyData);
}
