#include "Test.h"
#include "imgui/imgui.h"

test::TestMenu::TestMenu(Test*& currentTestPointer)
	: m_CurrentTest(currentTestPointer)
{
}
test::TestMenu::~TestMenu()
{

}

void test::TestMenu::OnImGuiRender()
{
	for (auto& t : m_Tests)
	{
		if (ImGui::Button(t.first.c_str()))
			m_CurrentTest = t.second();
	}
}
