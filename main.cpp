#ifndef _DEBUG
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "misc\util.h"
#include "GUI\GUI.h"
#include "Updater\update.h"
#include "GUI/macro.h"
#include <thread>
#include <format>

LPVOID ptrController;
LPVOID asyncThreadPointer;
LPVOID ptrMacros;
extern UCHAR rumble[2]{};
extern std::string Version = "PCXSenseBeta0.3";


VOID CALLBACK getRumble(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData)
{
	rumble[0] = SmallMotor;
	rumble[1] = LargeMotor;
}

void zeroOutputReport() {
	unsigned char outputHID[547]{};
	if (reinterpret_cast<controller*>(ptrController)->bluetooth) {
		ZeroMemory(outputHID, 547);

		outputHID[0] = 0x31;
		outputHID[1] = 0x02;
		outputHID[2] = 0x03 | 0x04 | 0x08;
		outputHID[3] = 0x55;

		const UINT32 crc = computeCRC32(outputHID, 74);

		outputHID[74] = (crc & 0x000000FF);
		outputHID[75] = ((crc & 0x0000FF00) >> 8UL);
		outputHID[76] = ((crc & 0x00FF0000) >> 16UL);
		outputHID[77] = ((crc & 0xFF000000) >> 24UL);

		WriteFile(reinterpret_cast<controller*>(ptrController)->deviceHandle, outputHID, 547, NULL, NULL);
	}
	else {
		ZeroMemory(outputHID, 547);

		outputHID[0] = 0x02;
		outputHID[1] = 0x03 | 0x04 | 0x08;
		outputHID[2] = 0x55;

		WriteFile(reinterpret_cast<controller*>(ptrController)->deviceHandle, outputHID, 64, NULL, NULL);
	}
}

extern BOOL WINAPI exitFunction(_In_ DWORD dwCtrlType) {
	reinterpret_cast<std::thread*>(asyncThreadPointer)->~thread();
	zeroOutputReport();
	saveMacros(*reinterpret_cast<std::vector<Macros>*>(ptrMacros));

	//Cleanup	
	vigem_target_remove(reinterpret_cast<controller*>(ptrController)->client, reinterpret_cast<controller*>(ptrController)->emulateX360);
	vigem_target_free(reinterpret_cast<controller*>(ptrController)->emulateX360);
	vigem_disconnect(reinterpret_cast<controller*>(ptrController)->client);
	vigem_free(reinterpret_cast<controller*>(ptrController)->client);
	_exit(NULL);
	return TRUE;
}

						
int main() {
#ifndef _DEBUG
	autoUpdater();
#endif
	//Initialize Fake Controller
	controller x360Controller{};
	std::vector<Macros> Macro;
	loadMacros(Macro);
	ptrMacros = &Macro;

	ptrController = &x360Controller;

	x360Controller.client = vigem_alloc();

	SetProcessShutdownParameters(2, 0);

	SetConsoleCtrlHandler(exitFunction, TRUE);

	if (initializeFakeController(x360Controller.emulateX360, x360Controller.target, x360Controller.client) != 0) return -1;

	std::thread asyncOutputReport(sendOutputReport, std::ref(x360Controller));
	asyncOutputReport.detach();

	asyncThreadPointer = &asyncOutputReport;
	std::thread(GUI, std::ref(x360Controller),std::ref(Macro)).detach();
	std::thread(asyncMacro, std::ref(x360Controller),std::ref(Macro)).detach();

#if _DEBUG
	std::thread(asyncDataReport, std::ref(x360Controller)).detach(); // Displays controller info
#endif
	vigem_target_x360_register_notification(x360Controller.client, x360Controller.emulateX360, &getRumble, ptrController);


	while (true) {

		XInputGetState(0, &x360Controller.ControllerState);

		getInputReport(x360Controller);
		
		vigem_target_x360_update(x360Controller.client, x360Controller.emulateX360, *reinterpret_cast<XUSB_REPORT*>(&x360Controller.ControllerState.Gamepad));
	}

	return 0;
}

