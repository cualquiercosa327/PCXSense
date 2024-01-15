#include "macro.h"
#include "GUI/functionality.h"
#include <fstream>

void saveMacros(const std::vector<Macros> Macro) {
	std::ofstream writeMacros("macros.txt");
	if(writeMacros.is_open()){
		for (Macros macro : Macro) {
			writeMacros << macro.Name << '\n';
			writeMacros << macro.buttonCombination << '\n';
			writeMacros << macro.input[0].ki.wVk << '\n';
			writeMacros << (char)macro.input[1].ki.wVk << '\n';
		}
		writeMacros.close();
	}
}

void loadMacros(std::vector<Macros>& Macro){
	std::ifstream loadMacros("macros.txt");
	if (loadMacros.is_open()) {
		char macroName[128]{};
		int input1, buttonCombination;
		char input2;

		while (loadMacros.good()) {
			Macros currentMacro{};
			ZeroMemory(macroName, 0);

			loadMacros.getline(macroName, 128);
			loadMacros >> buttonCombination;
			loadMacros >> input1;
			loadMacros >> input2;
			loadMacros.get();

			currentMacro.Name = macroName;
			currentMacro.buttonCombination = buttonCombination;
			currentMacro.input[0].ki.wVk = input1;
			currentMacro.input[1].ki.wVk = input2;
			currentMacro.input[0].type = INPUT_KEYBOARD;
			currentMacro.input[1].type = INPUT_KEYBOARD;

			Macro.push_back(currentMacro);
		}
		if (Macro.size() <= 3) Macro.pop_back(); //Error correction
		loadMacros.close();
	}
}

void macroEditor(bool& makerOpen, Macros& macro, const controller& x360Controller) {

	const static char alphabet[26] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	static std::string modifier;

	switch (macro.input[0].ki.wVk) { //Makes adding different modifiers easier
	case VK_CONTROL:
		modifier = "Control";
		break;
	case VK_MENU:
		modifier = "Alt";
		break;
	default:
		modifier = "None";
		break;
	}

	if(ImGui::Begin("Macro Maker", &makerOpen)){
		static int notificationTimer = 0;
		static bool notificationOpen = false;
		if (notificationTimer == 1000) notificationOpen = false;
		else notificationTimer++;

		ImGui::InputText("Macro name", &macro.Name);
		if (ImGui::BeginCombo("Key",std::format("{}",(char)macro.input[1].ki.wVk).c_str())) {
			for (int i = 0; i < 26; i++) {
				if (ImGui::Selectable(std::format("{}", alphabet[i]).c_str())) macro.input[1].ki.wVk = alphabet[i];
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Modifier", modifier.c_str())) {
			if (ImGui::Selectable("Control"))  macro.input[0].ki.wVk = VK_CONTROL;
			if (ImGui::Selectable("Alt"))  macro.input[0].ki.wVk = VK_MENU;
			if (ImGui::Selectable("None"))  macro.input[0].ki.wVk = 0;
			ImGui::EndCombo();
		}

		ImGui::Text("Set button combination");
		ImGui::SameLine();
		if (ImGui::SmallButton("Set")) {
			Sleep(3000);
			macro.buttonCombination = x360Controller.ControllerState.Gamepad.wButtons;
			std::cout << x360Controller.ControllerState.Gamepad.wButtons << '\n';
			notificationOpen = true;
			notificationTimer = 0;
		}
		if (ImGui::BeginItemTooltip()) {
			if (!notificationOpen) ImGui::Text("Press the buttons on the controller and wait 3 seconds");
			else ImGui::Text("Done");
			ImGui::EndTooltip();
		}

	}

	ImGui::End();
}