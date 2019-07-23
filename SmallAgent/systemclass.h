#pragma once

////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////

enum DecoderType;

#include "d3dclass.h"
#include "FrameQueueClass.h"
#include "SourceClass.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;
////////////////////////////////////////////////////////////////////////////////
// Class name: SystemClass
////////////////////////////////////////////////////////////////////////////////
class SystemClass
{
public:

	SystemClass(FramequeueClass* src_queue, DecoderType Type) : selectdecode(Type),m_queue(src_queue), Pend(false), Render_FPS(0), timecheck(0), m_Show_Debug(true){
		m_keys[256] = 0;
		m_D3D = new D3DClass();

	}
	~SystemClass() {}

	HWND InitializeWindows();
	bool Initialize3D();
	void Run();

	bool ClearQueue();
	bool Close();
	const DecoderType selectdecode;

	HWND m_hwnd;
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	bool Pend;

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	bool m_keys[256];
	clock_t end;
	float timecheck;
	FramequeueClass* m_queue;
	double VideoFPS;
	int Render_FPS;
	ID3D11Texture2D* m_2DTex;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
	D3DClass* m_D3D = 0;

	void DeleteFrame(AVFrame* temp);
	void Render(AVFrame* VFtemp);
	void Sleep_time_control(float RenderCycle, float &RealFPStemp, float &sleeptime);
	int  userinput(string &filename);
	void Initializeinput();
	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	bool IsKeyDown(unsigned int);
	bool m_Show_Debug;
	bool timer_set();


	INT64 m_frequency;
	float m_ticksPerMs;
	INT64 m_startTime;
	float m_frameTime;

	//Debug_logClass *log;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;