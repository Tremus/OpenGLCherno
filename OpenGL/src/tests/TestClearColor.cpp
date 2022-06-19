#include "TestClearColor.h"
#include <GL/glew.h>
#include "../Renderer.h"
#include "imgui/imgui.h"

test::TestClearColor::TestClearColor()
	: m_ClearColour{ 0.2f, 0.3f, 0.8f, 1.0f }
{
}

test::TestClearColor::~TestClearColor()
{
}

void test::TestClearColor::onUpdate(float deltaTime) {}

void test::TestClearColor::onRender()
{
	GLCall(glClearColor(m_ClearColour[0], m_ClearColour[1], m_ClearColour[2], m_ClearColour[3]));
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void test::TestClearColor::onImGuiRender()
{
	ImGui::ColorPicker4("Clear Color", m_ClearColour);
}
