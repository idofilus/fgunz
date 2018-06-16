#ifndef __D3D9RENDERER_H__
#define __D3D9RENDERER_H__

#include "hackbase.h"

class D3D9Renderer : public Renderer
{
private:
	IDirect3DDevice9 *pDevice;
	LPD3DXFONT pFont;

public:
	D3D9Renderer(IDirect3DDevice9 *Device);
	~D3D9Renderer();
	IDirect3DDevice9* GetDevice() { return pDevice; };
	void BeginScene() {}
	void EndScene() {}
	void RefreshData(IDirect3DDevice9 *Device);
	void DrawRect(int x, int y, int w, int h, Color color);
	void DrawBorder(int x, int y, int w, int h, int d, Color color);
	void DrawLine(int x1, int y1, int x2, int y2, float size, Color color) {};
	void DrawText(int x, int y, char *text, ...);
	void DrawText(int x, int y, D3DXCOLOR color, char *text);
	void DrawText(int x, int y, D3DXCOLOR color, bool bold, char *text);
	int GetWidth();
	int GetHeight();
	void loglnDebugInfo();
};

#endif