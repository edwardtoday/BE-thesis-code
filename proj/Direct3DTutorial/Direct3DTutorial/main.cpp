// include the basic windows header files and the Direct 3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <atlconv.h>

#include "mytimer.h"
#include "utils.h"
#include "config.h"

// include the Direct3D Libary file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

using namespace std;

// global declarations
LPDIRECT3D9 d3d;//pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;// the pointer to the device class
IDirect3DSurface9*	gImageSrc = NULL; //Source stereo image
IDirect3DSurface9* gBackBuf = NULL;
RECT destRect; // rectangle to change the destination of the 2 image we are loading

//function prototypes
void initD3D(HWND hWnd);// set up and initializes Direct3D
void render_frame(void);// renders a single frame
void cleanD3D(void);// closes Direct3D and releases memory
void getFilename(int); // get filename of the next img src
void loadFrame(int); // load dual-view frames
void render3D(void); // stretches our surface to the backbuffer


// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message,WPARAM wParam,LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
	// the handle for the window, filled by a function
	HWND hWnd;
	// this struct holds information for the window class
	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	// wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	string lpszClassNameStr = "WindowClass1";
	string windowNameStr = "3DVision Test";
	wc.lpszClassName = lpszClassNameStr.c_str();

	// register the window class
	RegisterClassEx(&wc);

	// create the window and use the result as the handle
	hWnd = CreateWindowEx(NULL,
		lpszClassNameStr.c_str(),// name of the window class
		windowNameStr.c_str(), // title of the window
		WS_EX_TOPMOST | WS_POPUP,// fullscreen values
		0,// x-position of the window
		0,// y-position of the window
		gImageWidth,// width of the window
		gImageHeight,// height of the window
		NULL,// we have no parent window, NULL
		NULL,// we aren't using menus, NULL
		hInstance,// application handle
		NULL);// used with multiple windows, NULL

	// display the window on the screen
	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	initD3D(hWnd);

	//3D VISION
	// create the surface
	d3ddev->CreateOffscreenPlainSurface(gImageWidth *2, gImageHeight + 1, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrc, NULL);


	fout.open("result.txt");
	MyTimer timerIns;
	timerIns.Start();
	LONGLONG _start = timerIns.getElapsedTime();
	LONGLONG _current;
	int frameToRender;
	int lastRenderedFrame = -1;
	bool buffered = false;
	int counter = 0;	// rendered frames counter
	int bufcounter = 0;	// loaded frames counter

	// Lock the stereo image
	D3DLOCKED_RECT lr;
	gImageSrc->LockRect(&lr,NULL,0);
	// write stereo signature in the last raw of the stereo image
	LPNVSTEREOIMAGEHEADER pSIH = (LPNVSTEREOIMAGEHEADER)(((unsigned char *) lr.pBits) + (lr.Pitch * (gImageHeight)));

	// Update the signature header values
	pSIH->dwSignature = NVSTEREO_IMAGE_SIGNATURE;
	pSIH->dwBPP = 32;
	//pSIH->dwFlags = SIH_SWAP_EYES; // Src image has left on left and right on right, thats why this flag is not needed.
	pSIH->dwWidth = gImageWidth*2;
	pSIH->dwHeight = gImageHeight;

	// Unlock surface
	gImageSrc->UnlockRect();

	// enter the main loop:

	// this struct holds Windows event messages
	MSG msg;

	while(TRUE) {
		// wait for the next message in the queue, store the result in 'msg'
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// translate keystroke messages into the right format
			TranslateMessage(&msg);

			// send the message to the WindowProc function
			DispatchMessage(&msg);
		}

		// If the message is WM_QUIT, exit the while loop
		if(msg.message == WM_QUIT)
			break;

		_current = timerIns.getElapsedTime() - _start;
		frameToRender = (int)(_current * fps / (double)1000000);
		//frameToRender = frameToRender % 3 + 400;
		//if (counter>250)
		//{
		//	break;
		//}
		if(frameToRender + 1 > totalFrames) {
			//switch to another demo
			if(demo == 2){
				prefixL = prefixL3;
				prefixR = prefixR3;
				totalFrames = totalFrames3;
				demo = 3;
			}
			else if(demo == 3){
				prefixL = prefixL1;
				prefixR = prefixR1;
				totalFrames = totalFrames1;
				demo = 1;
			}
			else if(demo == 1){
				prefixL = prefixL2;
				prefixR = prefixR2;
				totalFrames = totalFrames2;
				demo = 2;
			}
			_start = timerIns.getElapsedTime(); // start over
			continue;
			break;
		}
		if(buffered==false) {
			getFilename(lastRenderedFrame+1);
			loadFrame(lastRenderedFrame+1);
			//configureSurface();
			bufcounter++;
			buffered=true;
		}
		if(frameToRender!=lastRenderedFrame) {
			//fout<<"rendering frame "<<frameToRender<<endl;
			render_frame();
			counter++;
			lastRenderedFrame = frameToRender;
			buffered = false;
		}
	}

	// clean up DirectX and COM

	fout<< "total frames buffered = "<<bufcounter<<endl;
	fout<< "total frames rendered = "<<counter<<endl;
	double perf = (double)counter * 1000000/ timerIns.getElapsedTime();
	fout << "actual fps reached = "<< perf << endl;
	fout.close();
	// return this part of the WM_QUIT message to Windows
	cleanD3D();
	return msg.wParam;
}

///////////////////////////////////////////////////////////////////////////////
// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// sort through and find what code to run for the message given
	switch(message) {
		// this message is read when the window is closed
case WM_DESTROY: {
	// close the application entirely
	PostQuitMessage(0);
	return 0;
				 }
				 break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc (hWnd, message, wParam, lParam);
}

// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd) {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);// create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;// create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));// clear out the struct for use
	d3dpp.Windowed = FALSE;// program fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;// discard old frames
	d3dpp.hDeviceWindow = hWnd;// set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;// set the back buffer format to 32 bit
	d3dpp.BackBufferWidth = gImageWidth;
	d3dpp.BackBufferHeight = gImageHeight;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.BackBufferCount = 1;

	// create a device class using this information and information from the d3dpp struct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&d3ddev);
}

// this is the function used to render a single frame
void render_frame(void) {
	// clear the window to a deep blue
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	d3ddev->BeginScene();// begins the 3D scene

	// do 3D rendering on the back buffer here
	render3D();


	d3ddev->EndScene();// ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);// displays the created frame
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void) {
	d3ddev->Release();// close and release the 3D device
	d3d->Release();// close and release Direct3D
}


// this is the function that combines two images.
void render3D(void) {
	// set destRect
	destRect.left = 0;
	destRect.top = 0;
	destRect.bottom = gImageHeight;
	destRect.right = gImageWidth;

	// Get the Backbuffer then Stretch the Surface on it.
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &gBackBuf);
	d3ddev->StretchRect(gImageSrc, NULL, gBackBuf, &destRect, D3DTEXF_NONE);
}

wstring s2ws(const string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	wstring r(buf);
	delete[] buf;
	return r;
}

void getFilename(int _frame) {
	stringstream filenamer;

	filenamer<<prefixL<<setw(4)<<setfill('0')<<_frame<<suffix;
	//filenamer<<prefixL<<_frame<<suffix;
	filenamer>>filenameStrL;
	filenameWstrL = s2ws(filenameStrL);

	filenamer.clear();
	filenamer<<prefixR<<setw(4)<<setfill('0')<<_frame<<suffix;
	//filenamer<<prefixR<<_frame<<suffix;
	filenamer>>filenameStrR;
	filenamer.clear();
	filenameWstrR = s2ws(filenameStrR);

	filenameL = filenameWstrL.c_str();
	filenameR = filenameWstrR.c_str();
	filenameLL = filenameStrL.c_str();
	filenameRR = filenameStrR.c_str();
}

void loadFrame(int _frame) {
	static bool loaded = false;
	static char *data1 = 0, *data2 = 0;
	int dataSize1 = 0, dataSize2 = 0;
	/*
	// set up destRect
	destRect.left = 0;
	destRect.top = 0;
	destRect.bottom = gImageHeight;
	destRect.right = gImageWidth;
	//if (!loaded)
	{
	loaded = true;
	fout << "load file\n";
	FILE *f = fopen(filenameStrL.c_str(), "rb");
	//fseek(f, 0, SEEK_END);
	//int fsize = ftell(f);
	//fseek(f, 0, SEEK_SET);
	fout << "file loaded \n";
	int fsize = 10 * 1024 * 1024;
	data1 = new char[10*1024*1024];
	dataSize1 = fread(data1, 1, fsize, f);
	fclose(f);

	f = fopen(filenameStrR.c_str(), "rb");
	//fseek(f, 0, SEEK_END);
	//fsize = ftell(f);
	//fseek(f, 0, SEEK_SET);
	data2 = new char[fsize];
	dataSize2 = fread(data2, 1, fsize, f);
	fclose(f);

	fout << "Size = " << dataSize1 << " " << dataSize2 << endl;
	}

	D3DXLoadSurfaceFromMemory(gImageSrc, NULL, &destRect, data1, D3DFMT_A8R8G8B8, 0, NULL, NULL,D3DX_DEFAULT, 0);
	destRect.left = gImageWidth;
	destRect.top = 0;
	destRect.bottom = gImageHeight;
	destRect.right = gImageWidth*2;
	D3DXLoadSurfaceFromMemory(gImageSrc, NULL, &destRect, data2, D3DFMT_A8R8G8B8, 0, NULL, NULL, D3DX_DEFAULT, 0);
	*/

	///*
	// set up destRect
	destRect.left = 0;
	destRect.top = 0;
	destRect.bottom = gImageHeight;
	destRect.right = gImageWidth;

	// load left image
	D3DXLoadSurfaceFromFile(gImageSrc, NULL, &destRect, filenameLL, NULL, D3DX_DEFAULT, 0, NULL);

	// shift destRect to the right
	destRect.left = gImageWidth;
	destRect.top = 0;
	destRect.bottom = gImageHeight;
	destRect.right = gImageWidth*2;

	// load right image
	D3DXLoadSurfaceFromFile(gImageSrc, NULL, &destRect, filenameRR, NULL, D3DX_DEFAULT, 0, NULL);
	//*/
}