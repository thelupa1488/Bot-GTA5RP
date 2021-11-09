#include "Functions.h"
#include "Overlay.h"

LPCSTR TargetProcess = "GTA5.exe";

bool ShowMenu = false;
bool Work = false;
bool CreateConsole = false;

namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	int WindowLeft;
	int WindowRight;
	int WindowTop;
	int WindowBottom;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}

namespace OverlayWindow {
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

namespace DirectX9Interface {
	IDirect3D9Ex* Direct3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParams = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}

const char* bots[]{ "None", u8"Рыбалка", u8"Копать червей", u8"Фитнес", u8"Схемы" };
namespace Botfunction {
	bool debug = false;
	int menu_hotkey = VK_INSERT;
	int start_hotkey = VK_HOME;
	int type_bot = 0;
	bool auto_hotkey = false;
	int bait = 0;
	int worms = 0;
	int fast_hotkey = 0x4F;
	int delay_press = 5;

	int Text_x = 25;
	int Text_y = 700;
}

void Draw() {
	int x_pos = Botfunction::Text_x, y_pos = Botfunction::Text_y;
	RGBA White = { 255, 255, 255, 255 };

	if (Botfunction::type_bot > 0) {
		char WorkInfo[64];

		sprintf(WorkInfo, u8"Бот: %s | %s", bots[Botfunction::type_bot], (Work ? u8"Включен" : u8"Выключен"));
		DrawStrokeText(x_pos, y_pos, &White, WorkInfo);
	}

	if (Botfunction::type_bot == 1) {
		char FishInfo[64];

		sprintf(FishInfo, u8"Кол-во наживки: %d\nКол-во червячеков: %d", Botfunction::bait, Botfunction::worms);
		DrawStrokeText(x_pos, y_pos + 15, &White, FishInfo);
	}

	if (Botfunction::debug) {
		char MouseInfo[64];
		POINT cur_pos;
		GetCursorPos(&cur_pos);
		ScreenToClient(Process::Hwnd, &cur_pos);
		HDC hDC = GetDC(Process::Hwnd);
		COLORREF rgb = GetPixel(hDC, cur_pos.x, cur_pos.y - 1);

		sprintf(MouseInfo, u8"X: %d | Y: %d\nR: %d | G: %d | B: %d\nCOLORREF: %d", cur_pos.x, cur_pos.y - 1, (int)GetRValue(rgb), (int)GetGValue(rgb), (int)GetBValue(rgb), rgb);
		DrawFilledRect(cur_pos.x, cur_pos.y - 1, 3, 3, &White);
		DrawStrokeText(cur_pos.x - 1, cur_pos.y, &White, MouseInfo);
		ReleaseDC(Process::Hwnd, hDC);
	}
}

void Menu() {
	ImVec2 pos;
	ImGui::SetNextWindowPos(ImVec2(Process::WindowWidth / 2, Process::WindowHeight / 2), ImGuiCond_FirstUseEver, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin(u8"Молчать шлюха", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
	{
		pos = ImGui::GetWindowPos();

		ImGui::Hotkey(u8"Меню", &Botfunction::menu_hotkey, ImVec2(100, 25));
		ImGui::SameLine();
		ImGui::Hotkey(u8"Вкл/Выкл", &Botfunction::start_hotkey, ImVec2(125, 25));
		ImGui::Separator();
		ImGui::PushItemWidth(125.f);
		ImGui::Combo(u8"Бот", &Botfunction::type_bot, bots, IM_ARRAYSIZE(bots));
		ImGui::SameLine();
		ImGui::Checkbox(u8"Дебаг", &Botfunction::debug);
		ImGui::Separator();
		ImGui::PushItemWidth(100.f);
		ImGui::SliderInt(u8"Количество наживки", &Botfunction::bait, 0, 99);
		ImGui::SliderInt(u8"Количество червей", &Botfunction::worms, 0, 99);
		ImGui::PushItemWidth(135.f);
		ImGui::SliderInt(u8"Задержка ЛКМ", &Botfunction::delay_press, 1, 50);
		ImGui::Separator();
		ImGui::PushItemWidth(100.f);
		ImGui::Checkbox(u8"Быстрый слот", &Botfunction::auto_hotkey);
		ImGui::SameLine();
		ImGui::Hotkey(u8"", &Botfunction::fast_hotkey, ImVec2(114, 25));
		ImGui::Separator();
		ImGui::PushItemWidth(160.f);
		ImGui::SliderInt(u8"X позиция", &Botfunction::Text_x, 25, Process::WindowWidth - 25);
		ImGui::SliderInt(u8"Y позиция", &Botfunction::Text_y, 25, Process::WindowHeight - 25);
		ImGui::PopItemWidth();
		ImGui::Separator();

		if (ImGui::Button(u8"Выключение", ImVec2(235, 30)))
			exit(0);
	}
	ImGui::End();

	if (Botfunction::debug)
		ImGui::ShowStyleEditor();
}

void Render() {
	if (GetAsyncKeyState(Botfunction::menu_hotkey) & 1)
		ShowMenu = !ShowMenu;

	if (GetAsyncKeyState(Botfunction::start_hotkey) & 1)
		Work = !Work;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	Draw();
	ImGui::GetIO().MouseDrawCursor = ShowMenu;
	if (ShowMenu) {
		Menu();
		SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		UpdateWindow(OverlayWindow::Hwnd);
		SetFocus(OverlayWindow::Hwnd);
	}
	else {
		SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
		UpdateWindow(OverlayWindow::Hwnd);
	}
	ImGui::EndFrame();

	DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9Interface::pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9Interface::pDevice->EndScene();
	}

	HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));

	while (DirectX9Interface::Message.message != WM_QUIT) {
		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9Interface::Message);
			DispatchMessage(&DirectX9Interface::Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == Process::Hwnd) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(Process::Hwnd, &TempRect);
		ClientToScreen(Process::Hwnd, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = Process::Hwnd;

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			Process::WindowWidth = TempRect.right;
			Process::WindowHeight = TempRect.bottom;
			DirectX9Interface::pParams.BackBufferWidth = Process::WindowWidth;
			DirectX9Interface::pParams.BackBufferHeight = Process::WindowHeight;
			SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, Process::WindowWidth, Process::WindowHeight, SWP_NOREDRAW);
			DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9Interface::pDevice != NULL) {
		DirectX9Interface::pDevice->EndScene();
		DirectX9Interface::pDevice->Release();
	}
	if (DirectX9Interface::Direct3D9 != NULL) {
		DirectX9Interface::Direct3D9->Release();
	}
	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

bool DirectXInit() {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) {
		return false;
	}

	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = OverlayWindow::Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = Process::WindowWidth;
	Params.BackBufferHeight = Process::WindowHeight;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D16;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice))) {
		DirectX9Interface::Direct3D9->Release();
		return false;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
	
	ImGui::StyleColorsMy();
	ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
	ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
	DirectX9Interface::Direct3D9->Release();
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9Interface::pDevice != NULL) {
			DirectX9Interface::pDevice->EndScene();
			DirectX9Interface::pDevice->Release();
		}
		if (DirectX9Interface::Direct3D9 != NULL) {
			DirectX9Interface::Direct3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
			DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	OverlayWindow::WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
	};

	RegisterClassEx(&OverlayWindow::WindowClass);
	if (Process::Hwnd) {
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(Process::Hwnd, &TempRect);
		ClientToScreen(Process::Hwnd, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		Process::WindowWidth = TempRect.right;
		Process::WindowHeight = TempRect.bottom;
	}

	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, Process::WindowLeft, Process::WindowTop, Process::WindowWidth, Process::WindowHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
}

#define X 0
#define Y 1

DWORD WINAPI Function(LPVOID lpParameter) {
	static bool fish = false;
	static bool schemic = false;
	static int type_bait = 0;
	static int arrCoord[2][64];

	if (Process::WindowWidth == 1366 && Process::WindowHeight == 768) {
		int CoordX[] = { 945, 775, 379, Process::WindowWidth / 2, Process::WindowWidth / 2 - 15, 255 };
		int CoordY[] = { 105, 180, 220, 675, 650, 705, 330, 450, 345, 660 };

		for (int i = 0; i < (sizeof(CoordX) / sizeof(CoordX[0])); i++)
			arrCoord[X][i] = CoordX[i];
		for (int i = 0; i < (sizeof(CoordY) / sizeof(CoordY[0])); i++)
			arrCoord[Y][i] = CoordY[i];
	}
	else { // full hd
		int CoordX[] = { 1495, 1052, 656, Process::WindowWidth / 2, Process::WindowWidth / 2 - 15, 340 };
		int CoordY[] = { 260, 335, 380, 923, 898, 1017, 630, 750, 495, 970 };

		for (int i = 0; i < (sizeof(CoordX) / sizeof(CoordX[0])); i++)
			arrCoord[X][i] = CoordX[i];
		for (int i = 0; i < (sizeof(CoordY) / sizeof(CoordY[0])); i++)
			arrCoord[Y][i] = CoordY[i];
	}

	while (true) {
		if (!Work) {
			fish = false;
			type_bait = 0;
		}

		if (Work && GetForegroundWindow() == Process::Hwnd) {
			if (Botfunction::type_bot == 1) { //Рыбалка
				if (!fish && Botfunction::bait <= 0 && Botfunction::worms <= 0) {
					Work = false;
					fish = false;
					type_bait = 0;
				}
				HDC hDC = GetDC(Process::Hwnd);
				if (!fish) {
					if (Botfunction::auto_hotkey && Botfunction::bait > 0) {
						PostMessage(Process::Hwnd, WM_KEYDOWN, Botfunction::fast_hotkey, NULL);
						PostMessage(Process::Hwnd, WM_KEYUP, Botfunction::fast_hotkey, NULL);
						type_bait = 1;
					}
					else {
						if (Botfunction::bait > 0 || Botfunction::worms > 0) {
							PostMessage(Process::Hwnd, WM_KEYDOWN, 0x49, NULL);
							PostMessage(Process::Hwnd, WM_KEYUP, 0x49, NULL);
							Sleep(150);
						}
						if (Botfunction::bait > 0) {
							SendMessage(Process::Hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][0]));
							SendMessage(Process::Hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][0]));
							Sleep(150);
							SendMessage(Process::Hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][1]));
							SendMessage(Process::Hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][1]));
							type_bait = 1;
						}
						else if (Botfunction::worms > 0 && Botfunction::bait <= 0) {
							SendMessage(Process::Hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][0]));
							SendMessage(Process::Hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][0]));
							Sleep(150);
							SendMessage(Process::Hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][2]));
							SendMessage(Process::Hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][2]));
							type_bait = 2;
						}
						Sleep(250);
					}
					if (GetPixel(hDC, arrCoord[X][5], arrCoord[Y][9]) == 55551) {
						if (type_bait == 1)
							Botfunction::bait--;
						else if (type_bait == 2) 
							Botfunction::worms--;
					}
					fish = true;
				}
				while (GetPixel(hDC, arrCoord[X][1], arrCoord[Y][3]) == 1623295) {
					if (fish) {
						type_bait = 0;
						fish = false;
					}
					if (GetPixel(hDC, arrCoord[X][1], arrCoord[Y][4]) == 255) {
						SendMessage(Process::Hwnd, WM_LBUTTONDOWN, MK_LBUTTON, NULL);
						SendMessage(Process::Hwnd, WM_LBUTTONUP, MK_LBUTTON, NULL);
						Sleep(Botfunction::delay_press);
					}
				}
				ReleaseDC(Process::Hwnd, hDC);
			}
			else if (Botfunction::type_bot == 2) { //Копать червей
				if (Botfunction::auto_hotkey && Botfunction::fast_hotkey != 0) {
					PostMessage(Process::Hwnd, WM_KEYDOWN, Botfunction::fast_hotkey, NULL);
					PostMessage(Process::Hwnd, WM_KEYUP, Botfunction::fast_hotkey, NULL);
				}
				else {
					PostMessage(Process::Hwnd, WM_KEYDOWN, 0x49, NULL);
					PostMessage(Process::Hwnd, WM_KEYUP, 0x49, NULL);
					Sleep(250);
					SendMessage(Process::Hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][0]));
					SendMessage(Process::Hwnd, WM_RBUTTONUP, MK_RBUTTON, MAKELONG(arrCoord[X][0], arrCoord[Y][0]));
					Sleep(250);
					PostMessage(Process::Hwnd, WM_KEYDOWN, 0x49, NULL);
					PostMessage(Process::Hwnd, WM_KEYUP, 0x49, NULL);
				}
				Sleep(10000);
			}
			else if (Botfunction::type_bot == 3) { //Фитнес
				HDC hDC = GetDC(Process::Hwnd);
				COLORREF pixel[120];
				for (int j = 0, i = arrCoord[Y][6]; i <= arrCoord[Y][7]; i++, j++) {
					pixel[j] = GetPixel(hDC, arrCoord[X][3], i);
					if (pixel[j] == 10944376) {
						PostMessage(Process::Hwnd, WM_KEYDOWN, VK_SPACE, NULL);
						PostMessage(Process::Hwnd, WM_KEYUP, VK_SPACE, NULL);
					}
					else if (pixel[j] == 13762496) {
						PostMessage(Process::Hwnd, WM_KEYDOWN, VK_SPACE, NULL);
						PostMessage(Process::Hwnd, WM_KEYUP, VK_SPACE, NULL);
					}
				}
				PostMessage(Process::Hwnd, WM_KEYDOWN, 0x45, NULL);
				PostMessage(Process::Hwnd, WM_KEYUP, 0x45, NULL);
				ReleaseDC(Process::Hwnd, hDC);
			}
			else if (Botfunction::type_bot == 4) { //Схемы
				HDC hDC = GetDC(Process::Hwnd);
				if (GetPixel(hDC, arrCoord[X][4], arrCoord[Y][8]) >= 328965) {
					PostMessage(Process::Hwnd, WM_KEYDOWN, 0x45, NULL);
					PostMessage(Process::Hwnd, WM_KEYUP, 0x45, NULL);
				}
				ReleaseDC(Process::Hwnd, hDC);
			}
		}
		Sleep(5);
	}
}

DWORD WINAPI ProcessCheck(LPVOID lpParameter) {
	while (true) {
		if (Process::Hwnd != NULL) {
			if (GetProcessId(TargetProcess) == 0) {
				exit(0);
			}
		}
	}
}

int main() {
	ShowWindow(GetConsoleWindow(), CreateConsole ? SW_SHOW : SW_HIDE);

	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetProcessId(TargetProcess) == ForegroundWindowProcessID) {
			Process::ID = GetCurrentProcessId();
			Process::Handle = GetCurrentProcess();
			Process::Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;
			Process::WindowLeft = TempRect.left;
			Process::WindowRight = TempRect.right;
			Process::WindowTop = TempRect.top;
			Process::WindowBottom = TempRect.bottom;

			char TempTitle[MAX_PATH];
			GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
			Process::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
		}
	}

	OverlayWindow::Name = RandomString(10).c_str();
	SetupWindow();
	DirectXInit();
	CreateThread(NULL, 0, ProcessCheck, 0, 0, NULL);
	CreateThread(NULL, 0, Function, 0, 0, NULL);
	while (TRUE) {
		MainLoop();
	}
}