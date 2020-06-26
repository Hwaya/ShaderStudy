#include "ShaderFramework.h"

// Statics
#define PI 3.14159265f
#define FOV (PI/4.f)									//Field of View
#define ASPECT_RATIO (WIN_WIDTH/(float)WIN_HEIGHT)		//Aspect
#define NEAR_PLANE 1									//near
#define FAR_PLANE 10000									//far

//D3D
LPDIRECT3D9			gpD3D = NULL;			//d3d
LPDIRECT3DDEVICE9	gpD3DDevice = NULL;		//d3d device

//Fonts
ID3DXFont*			gpFont = NULL;

//Models
LPD3DXMESH			gpSphere = NULL;

//Shaders
LPD3DXEFFECT		gpColorShader = NULL;

//Textures

//Program name
const char* gAppName = "Simple Shader demo Framework";

//Entrance

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	//Regist Window Class
	WNDCLASSEX wc = 	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC, 
		MsgProc, 
		0L, 
		0L, 
		GetModuleHandle(NULL), 
		NULL, 
		NULL, 
		NULL,
		NULL,
		gAppName, 
		NULL
	};
	RegisterClassEx(&wc);

	//Create Window
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(
		gAppName,
		gAppName,
		style,
		CW_USEDEFAULT,
		0,
		WIN_WIDTH,
		WIN_HEIGHT,
		GetDesktopWindow(),
		NULL,
		wc.hInstance,
		NULL
		);

	//Window Resize
	POINT ptDiff;
	RECT rcClient, rcWindow;
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd, rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	//Initialize Everything
	if (!InitEverything(hWnd))
	{
		PostQuitMessage(1);
	}

	//Loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//if message is none Update the game and render scene
			PlayDemo();
		}
	}

	//Unregist
	UnregisterClass(gAppName, wc.hInstance);
	return 0;
}

//Message Processor
LRESULT __stdcall MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//Keyboard Input
		case WM_KEYDOWN:
			ProcessInput(hWnd, wParam);
			break;
		//When Window is close Clean D3D resources and quit program
		case WM_DESTROY:
			Cleanup();
			PostQuitMessage(0);
			return 0;
	}
	//if Window messages not processed in here let them proccess by default window proccessor
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//Keyboard input process
void ProcessInput(HWND hWnd, WPARAM keyPress)
{
	switch (keyPress)
	{
		//Quit Program when escape button pressed
		case VK_ESCAPE:
			PostMessage(hWnd, WM_DESTROY, 0L, 0L);
			break;
	}
}

/****************************************************************************************/

bool InitEverything(HWND hWnd)
{
	//D3D initialize
	if (!InitD3D(hWnd))
	{
		return false;
	}

	//Load Model, Shader, Texture
	if (!LoadAssets())
	{
		return false;
	}

	//Load Font
	if (FAILED(D3DXCreateFont(
		gpD3DDevice,							//D3DDevice
		20,									//Font Height
		10,									//Font Width
		FW_BOLD,							//Font Style
		1,									//Mipmap level	
		FALSE,								//shoud be use italic?
		DEFAULT_CHARSET,					//Char set type
		OUT_DEFAULT_PRECIS,					//how much same quality with option defined in here
		DEFAULT_QUALITY,					//how much same quality with real font
		(DEFAULT_PITCH | FF_DONTCARE),		//use default pitch, and no matter font group
		"Arial",							//Font Name
		&gpFont)))							//Font Pointer
	{
		return false;
	}
	return true;
}

//Initialize D3D Object and Device
bool InitD3D(HWND hWnd)
{
	//D3D Device
	gpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!gpD3D)
	{
		return false;
	}
	
	//Fill the structure for create the d3d device
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	//D3D Present Parameter
	d3dpp.BackBufferWidth		= WIN_WIDTH;
	d3dpp.BackBufferHeight		= WIN_HEIGHT;
	d3dpp.BackBufferFormat		= D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount		= 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	//Create D3D Device
	if (FAILED(gpD3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		&gpD3DDevice)))
	{
		return false;
	}
	return true;
}

bool LoadAssets()
{
	// Load Texture

	// Load Shader
	gpColorShader = LoadShader("ColorShader.fx");
	if (!gpColorShader)
	{
		return false;
	}
	// Load Model
	gpSphere = LoadModel("Sphere.x");
	if (!gpSphere)
	{
		return false;
	}
	return true;
}

LPD3DXEFFECT LoadShader(const char * filename)
{
	LPD3DXEFFECT ret = NULL;
	LPD3DXBUFFER pError = NULL;
	DWORD dwShaderFlags = 0;

#if _DEBUG
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif
	D3DXCreateEffectFromFile(
		gpD3DDevice,				//D3D Device
		filename,					//Shader File name
		NULL,						//Define Shader Compile's additional option (?)
		NULL,						//Interface Pointer							(?)
		dwShaderFlags,				//The flag for use with shader compile
		NULL,						//The event pool for use with sharing variable
		&ret,						//loading effect pointer
		&pError);					//compiler error message pointer
	
	//When Shader Load failed print shader compile error to output window
	if (!ret && pError)
	{
		int size = pError->GetBufferSize();
		void *ack = pError->GetBufferPointer();

		if (ack)
		{
			char* str = new char[size];
			//sprintf(str, (const char*)ack, size);
			sprintf_s(str, size, (const char*)ack);
			OutputDebugString(str);
			delete[] str;
		}
	}

	return ret;
}

LPDIRECT3DTEXTURE9 LoadTexture(const char * filename)
{
	LPDIRECT3DTEXTURE9 ret = NULL;
	if (FAILED(D3DXCreateTextureFromFile(
		gpD3DDevice,
		filename,
		&ret)))
	{
		OutputDebugString("Texture Load Failed : ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}
	return ret;
}

LPD3DXMESH LoadModel(const char * filename)
{
	LPD3DXMESH ret = NULL;
	if (FAILED(D3DXLoadMeshFromX(
		filename,					//mesh filename
		D3DXMESH_SYSTEMMEM,			//Load mesh to system memory
		gpD3DDevice,				//D3D Device
		NULL,						//do not try to get contigous poly data
		NULL,						//do not try to get material info
		NULL,						//do not try to get effect instance
		NULL,						//do not try to get number of material
		&ret						//loaded mesh will be save to this pointer
	)))
	{
		OutputDebugString("Model Load Failed : ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	};
	return ret;
}

/****************************************************************************************/
// Loop
/****************************************************************************************/
void PlayDemo()
{
	Update();
	RenderFrame();
}

void Update()
{
}

/****************************************************************************************/
// Rendering
/****************************************************************************************/
void RenderFrame()
{
	// make screen blue
	D3DCOLOR bgColour = 0xFF0000FF;
	gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

	// render scene and debug info
	gpD3DDevice->BeginScene();
	{
		RenderScene();
		RenderInfo();
	}
	gpD3DDevice->EndScene();

	//if all render phase are done render backbuffer data to screen 
	gpD3DDevice->Present(NULL, NULL, NULL, NULL);
}

void RenderScene()
{
	D3DXMATRIXA16 matView;
	D3DXVECTOR3 vEyePt(0.f, 0.f, -200.f);
	D3DXVECTOR3 vLookatPt(0.f, 0.f, 0.f);
	D3DXVECTOR3 vUpVec(0.f, 1.f, 0.f);
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

	D3DXMATRIXA16 matProjection;
	D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);

	gpColorShader->SetMatrix("gWorldMatrix", &matWorld);
	gpColorShader->SetMatrix("gViewMatrix", &matView);
	gpColorShader->SetMatrix("gProjectionMatrix", &matProjection);

	// start Shading
	UINT numPasses = 0;
	gpColorShader->Begin(&numPasses, NULL);
	{
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpColorShader->BeginPass(i);
			{
				gpSphere->DrawSubset(0);
			}
			gpColorShader->EndPass();
		}
	}
	gpColorShader->End();
}

void RenderInfo()
{
	//Text Color
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	//Text Print Location
	RECT rct;
	rct.left = 5;
	rct.right = WIN_WIDTH / 3;
	rct.top = 5;
	rct.bottom = WIN_HEIGHT / 3;

	//Print input info by key input
	gpFont->DrawTextA(NULL, "aaaaaaaaaaaaaaa", -1, &rct, 0, fontColor);
}

void Cleanup()
{
	// Release the font
	if (gpFont)
	{
		gpFont->Release();
		gpFont = NULL;
	}

	//Release the Model

	//Release the Shader

	//Release the Texture

	//Release the D3D
	if (gpD3DDevice)
	{
		gpD3DDevice->Release();
		gpD3DDevice = NULL;
	}

	if (gpD3D)
	{
		gpD3D->Release();
		gpD3D = NULL;
	}

	if (gpSphere)
	{
		gpSphere->Release();
		gpSphere = NULL;
	}
	if (gpColorShader)
	{
		gpColorShader->Release();
		gpColorShader = NULL;
	}
}

/****************************************************************************************/