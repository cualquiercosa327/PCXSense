#pragma once
#include "util.h"

#define DIRECTINPUT_VERSION 0x0800
#define DS_STATUS_BATTERY_CAPACITY 0xF
#define DS_STATUS_CHARGING 0xF0
#define DS_STATUS_CHARGING_SHIFT 4
#define DS_VENDOR_ID 0x054c
#define DS_PRODUCT_ID 0x0ce6


void asyncDataReport(LPDIRECTINPUTDEVICE8& controllerInterface) {

	hid_device_info* deviceInfo = hid_enumerate(DS_VENDOR_ID, DS_PRODUCT_ID);
	bool bluetooth = deviceInfo->interface_number == -1;
	hid_device* dualsense = hid_open(DS_VENDOR_ID, DS_PRODUCT_ID, deviceInfo->serial_number);
	hid_free_enumeration(deviceInfo);
	int bufferSize;

	if (bluetooth) bufferSize = 78;
	
	else bufferSize = 64;
	
	unsigned char* buffer = new unsigned char[bufferSize];

	std::cout << bluetooth << '\n';
	if (bluetooth) buffer[0] = 0x31;

	else buffer[0] = 0x01;
	
	while (true) {
		
		if (bluetooth) hid_get_input_report(dualsense, buffer, bufferSize);
		else hid_read(dualsense, buffer, bufferSize);
		
		Sleep(1);
		system("cls"); //Clear console
		int batteryLevel = (buffer[0x35 + bluetooth] & 0x0F)*12.5; // Hex 0x35 for USB to get Battery/Hex 0x36 for Bluetooth to get Battery , because if bluetooth == true then bluetooth == 1 then we can just add bluetooth to the hex of USB

		HRESULT result = controllerInterface->Poll();

		DIJOYSTATE2 joystick{};

		result = controllerInterface->GetDeviceState(sizeof(DIJOYSTATE2), &joystick);

		switch (joystick.rgdwPOV[0]) {
		case 0:
			DEBUG("Dpad Up");
			break;

		case 31500:
			DEBUG("Dpad Up and Dpad Left");
			break;

		case 4500:
			DEBUG("Dpad Up and Dpad Right");
			break;

		case 27000:
			DEBUG("Dpad Left");
			break;

		case 9000:
			DEBUG("Dpad Right");
			break;

		case 18000:
			DEBUG("Dpad Down");
			break;

		case 22500:
			DEBUG("Dpad Down and Dpad Left");
			break;

		case 13500:
			DEBUG("Dpad Down and Dpad Right");
			break;
		}

		if (joystick.rgbButtons[0]) DEBUG("Square Button\n");

		if (joystick.rgbButtons[1]) DEBUG("X Button\n");

		if (joystick.rgbButtons[2]) DEBUG("Circle Button\n");

		if (joystick.rgbButtons[3]) DEBUG("Triangle Button\n");

		if (joystick.rgbButtons[4]) DEBUG("L1 Button\n");

		if (joystick.rgbButtons[5]) DEBUG("R1 Button\n");

		if (joystick.rgbButtons[8]) DEBUG("Select Button\n");

		if (joystick.rgbButtons[9]) DEBUG("Start Button\n");

		if (joystick.rgbButtons[10]) DEBUG("L3 Button\n");

		if (joystick.rgbButtons[11]) DEBUG("R3 Button\n");

		if (joystick.rgbButtons[12]) DEBUG("Sony/Home Button\n");

		if (joystick.rgbButtons[13]) DEBUG("Toutchpad Click\n");

		if (joystick.rgbButtons[14]) DEBUG("Microphone Button\n");

		if (controllerInterface->GetDeviceState(sizeof(DIJOYSTATE2), &joystick) != DI_OK) {
			while (controllerInterface->GetDeviceState(sizeof(DIJOYSTATE2), &joystick) != DI_OK) { //Controller is being acquired in the main function so we only need to check GetDeviceState is DI_OK
				std::cout << "Failed to get Device State\n";
				std::cout << "Reconnecting";
				Sleep(500);
				std::cout << '.';
				Sleep(500);
				std::cout << '.';
				Sleep(500);
				std::cout << '.';
				Sleep(500);
				system("cls");
			}	
		}
		std::cout << "\nLeftJoystick Horizontal Value: " << joystick.lX << '\n';
		std::cout << "LeftJoystick Vertical Value: " << joystick.lY << '\n';
		std::cout << "RightJoystick Horizontal Value: " << joystick.lZ << '\n';
		std::cout << "RightJoystick Horizontal Value: " << joystick.lRz << '\n';
		std::cout << "Left Trigger Value: " << joystick.lRx / 257 << '\n';
		std::cout << "Right Trigger Value: " << joystick.lRy / 257 << '\n';
		std::cout << "Battery Level: " << batteryLevel << "%\n";

	}
	delete[] buffer;
	hid_close(dualsense);

}

int initializeFakeController(HINSTANCE& appHandle, IDirectInput8* ptrDirectInput, LPDIRECTINPUTDEVICE8& controllerInterface, XINPUT_STATE& ControllerState, PVIGEM_TARGET& emulateX360, VIGEM_ERROR& target, PVIGEM_CLIENT& client) {
	if (client == nullptr)
	{
		std::cerr << "Uh, not enough memory to do that?!" << std::endl;
		return -1;
	}

	const auto retval = vigem_connect(client);

	if (!VIGEM_SUCCESS(retval))
	{
		std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval << std::endl;
		return -1;
	}

	emulateX360 = vigem_target_x360_alloc();

	target = vigem_target_add(client, emulateX360);

	if (DirectInput8Create(appHandle, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&ptrDirectInput, NULL) != DI_OK) {
		std::cout << "Failed to create DI8 Device\n";
		return -1;
	}
	if (ptrDirectInput->CreateDevice(GUID_Joystick, &controllerInterface, NULL) != DI_OK) {
		ptrDirectInput->Release();
		std::cout << "Failed to create Device\n";
		return -1;
	}
	if (controllerInterface->SetDataFormat(&c_dfDIJoystick2) != DI_OK) {
		ptrDirectInput->Release();
		std::cout << "Failed to set Data Format\n";
		return -1;
	}
	if (controllerInterface->Acquire() != DI_OK) {
		ptrDirectInput->Release();
		std::cout << "Failed to acquire Device\n";
		return -1;
	}
	return 0;
}