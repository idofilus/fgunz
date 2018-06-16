//=====================================================================================

// Roverturbo | www.unknowncheats.me

#include <windows.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

//=====================================================================================

typedef HRESULT(WINAPI* CreateDevice_Prototype)        (LPDIRECT3D9, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, LPDIRECT3DDEVICE9*);
typedef HRESULT(WINAPI* Reset_Prototype)               (LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
typedef HRESULT(WINAPI* EndScene_Prototype)            (LPDIRECT3DDEVICE9);
typedef HRESULT(WINAPI* DrawIndexedPrimitive_Prototype)(LPDIRECT3DDEVICE9, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);

CreateDevice_Prototype         CreateDevice_Pointer = NULL;
Reset_Prototype                Reset_Pointer = NULL;
EndScene_Prototype             EndScene_Pointer = NULL;
DrawIndexedPrimitive_Prototype DrawIndexedPrimitive_Pointer = NULL;

HRESULT WINAPI Direct3DCreate9_VMTable(VOID);
HRESULT WINAPI CreateDevice_Detour(LPDIRECT3D9, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, LPDIRECT3DDEVICE9*);
HRESULT WINAPI Reset_Detour(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
HRESULT WINAPI EndScene_Detour(LPDIRECT3DDEVICE9);
HRESULT WINAPI DrawIndexedPrimitive_Detour(LPDIRECT3DDEVICE9, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);

DWORD WINAPI VirtualMethodTableRepatchingLoopToCounterExtensionRepatching(LPVOID);
PDWORD Direct3D_VMTable = NULL;

//=====================================================================================
/*
BOOL WINAPI DllMain(HINSTANCE hinstModule, DWORD dwReason, LPVOID lpvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstModule);

		if (Direct3DCreate9_VMTable() == D3D_OK)
			return TRUE;
	}

	return FALSE;
}*/

//=====================================================================================

HRESULT WINAPI Direct3DCreate9_VMTable(VOID)
{
	LPDIRECT3D9 Direct3D_Object = Direct3DCreate9(D3D_SDK_VERSION);

	if (Direct3D_Object == NULL)
		return D3DERR_INVALIDCALL;

	Direct3D_VMTable = (PDWORD)*(PDWORD)Direct3D_Object;
	Direct3D_Object->Release();

	DWORD dwProtect;

	if (VirtualProtect(&Direct3D_VMTable[16], sizeof(DWORD), PAGE_READWRITE, &dwProtect) != 0)
	{
		*(PDWORD)&CreateDevice_Pointer = Direct3D_VMTable[16];
		*(PDWORD)&Direct3D_VMTable[16] = (DWORD)CreateDevice_Detour;

		if (VirtualProtect(&Direct3D_VMTable[16], sizeof(DWORD), dwProtect, &dwProtect) == 0)
			return D3DERR_INVALIDCALL;
	}
	else
		return D3DERR_INVALIDCALL;

	return D3D_OK;
}

//=====================================================================================

HRESULT WINAPI CreateDevice_Detour(LPDIRECT3D9 Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND FocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* PresentationParameters,
	LPDIRECT3DDEVICE9* Returned_Device_Interface)
{
	HRESULT Returned_Result = CreateDevice_Pointer(Direct3D_Object, Adapter, DeviceType, FocusWindow, BehaviorFlags,
		PresentationParameters, Returned_Device_Interface);

	DWORD dwProtect;

	if (VirtualProtect(&Direct3D_VMTable[16], sizeof(DWORD), PAGE_READWRITE, &dwProtect) != 0)
	{
		*(PDWORD)&Direct3D_VMTable[16] = *(PDWORD)&CreateDevice_Pointer;
		CreateDevice_Pointer = NULL;

		if (VirtualProtect(&Direct3D_VMTable[16], sizeof(DWORD), dwProtect, &dwProtect) == 0)
			return D3DERR_INVALIDCALL;
	}
	else
		return D3DERR_INVALIDCALL;

	if (Returned_Result == D3D_OK)
	{
		Direct3D_VMTable = (PDWORD)*(PDWORD)*Returned_Device_Interface;

		*(PDWORD)&Reset_Pointer = (DWORD)Direct3D_VMTable[16];
		*(PDWORD)&EndScene_Pointer = (DWORD)Direct3D_VMTable[42];
		*(PDWORD)&DrawIndexedPrimitive_Pointer = (DWORD)Direct3D_VMTable[82];

		if (CreateThread(NULL, 0, VirtualMethodTableRepatchingLoopToCounterExtensionRepatching, NULL, 0, NULL) == NULL)
			return D3DERR_INVALIDCALL;
	}

	return Returned_Result;
}

//=====================================================================================

HRESULT WINAPI Reset_Detour(LPDIRECT3DDEVICE9 Device_Interface, D3DPRESENT_PARAMETERS* PresentationParameters)
{
	return Reset_Pointer(Device_Interface, PresentationParameters);
}

//=====================================================================================

HRESULT WINAPI EndScene_Detour(LPDIRECT3DDEVICE9 Device_Interface)
{
	Console::print("irect3DCreate9_VMTable()");
	return EndScene_Pointer(Device_Interface);
}

//=====================================================================================

HRESULT WINAPI DrawIndexedPrimitive_Detour(LPDIRECT3DDEVICE9 Device_Interface, D3DPRIMITIVETYPE Type, INT BaseIndex,
	UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	LPDIRECT3DVERTEXBUFFER9 Stream_Data;
	UINT Offset = 0;
	UINT Stride = 0;

	if (Device_Interface->GetStreamSource(0, &Stream_Data, &Offset, &Stride) == D3D_OK)
		Stream_Data->Release();

	if (Stride == 0)
	{
	}

	return DrawIndexedPrimitive_Pointer(Device_Interface, Type, BaseIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
}

//=====================================================================================

DWORD WINAPI VirtualMethodTableRepatchingLoopToCounterExtensionRepatching(LPVOID Param)
{
	UNREFERENCED_PARAMETER(Param);

	while (1)
	{
		Sleep(100);

		*(PDWORD)&Direct3D_VMTable[16] = (DWORD)Reset_Detour;
		*(PDWORD)&Direct3D_VMTable[42] = (DWORD)EndScene_Detour;
		*(PDWORD)&Direct3D_VMTable[82] = (DWORD)DrawIndexedPrimitive_Detour;
	}

	return 1;
}

//=====================================================================================