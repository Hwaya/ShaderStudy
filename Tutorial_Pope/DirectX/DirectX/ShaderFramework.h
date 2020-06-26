#pragma once

#include <iostream>
#include <d3d9.h>
#include <d3dx9.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// --- Func Define

// Msg Proc
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ProcessInput(HWND hWnd, WPARAM keyPress);

// Initializer
bool InitEverything(HWND hWnd);
bool InitD3D(HWND hWnd);
bool LoadAssets();
LPD3DXEFFECT LoadShader(const char* filename);
LPDIRECT3DTEXTURE9 LoadTexture(const char* filename);
LPD3DXMESH LoadModel(const char* filename);

// Loop
void PlayDemo();
void Update();

//Rendering
void RenderFrame();
void RenderScene();
void RenderInfo();

//Cleanup
void Cleanup();