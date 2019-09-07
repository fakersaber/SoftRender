﻿#ifndef _YRENDER_RENDERDEVICE_H
#define _YRENDER_RENDERDEVICE_H

#include <windows.h>
#include <iostream>



namespace YRender {
	class RenderDevice {
	public:
		RenderDevice& operator=(const RenderDevice& rhs) = delete;
		RenderDevice(const RenderDevice& rhs) = delete;
		RenderDevice();
		~RenderDevice();
		//static RenderDevice* GetDevice();

	public:
		bool Initial(HWND hwnd, const int width, const int height);
		void DrawPixel(const int x,const int y);
		void DrawFrameBuffer();

	private:
		uint32_t* FrameBuffer;
		int width;
		int height;
		HDC BufferHdc;
		HDC WindowHdc;
		HBITMAP hBitmap;
		HBITMAP hOldBitmap;
		HWND hwnd;
	};
}



#endif 