#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"

class Drawing
{
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool bDraw;

public:
	static void Active();
	static bool isActive();
	static void Draw();
	static void drawToolbar();
	static void drawSections();
	static void drawEntropyData();

	// Entropy Graphs	
	static void drawEntropyLegend();
	static void drawEntropyGraphs(const SectionEntropyData& entropyData, int blockWidth=4);

};

#endif
