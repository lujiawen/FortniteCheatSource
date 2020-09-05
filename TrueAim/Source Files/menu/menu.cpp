//
// love fn sucks
// paulgamer stop pasteing plz
// hehe cracked
//
// YTMcGamer#0131
//

#include "../../Globals.h"
#include "../../Header Files/menu/menu.h"
#include "../../Header Files/includes.h"
#include "../../Header Files/Config/config.h"
#include "../../DiscordHook/Discord.h"
#include "../../Helper/Helper.h"
#include "../../Header Files/offsets/offsets.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <stdio.h>
#include <Windows.h>
#include <psapi.h>
#include <intrin.h>
#include <string>
#include <vector>
namespace ImGui
{
	IMGUI_API bool Tab(unsigned int index, const char* label, int* selected, float width = 0)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4 color = style.Colors[ImGuiCol_Button];
		ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
		ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];

		if (index > 0)
			ImGui::SameLine();

		if (index == *selected)
		{
			style.Colors[ImGuiCol_Button] = colorActive;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorActive;
		}
		else
		{
			style.Colors[ImGuiCol_Button] = color;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		}

		if (ImGui::Button(label, ImVec2(width, 30)))
			*selected = index;

		style.Colors[ImGuiCol_Button] = color;
		style.Colors[ImGuiCol_ButtonActive] = colorActive;
		style.Colors[ImGuiCol_ButtonHovered] = colorHover;

		return *selected == index;
	}
}
template<typename T>
T WriteMem(DWORD_PTR address, T value)
{
	return *(T*)address = value;
}
ID3D11Device* device = nullptr;
ID3D11DeviceContext* immediateContext = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
HRESULT(*ResizeOriginal)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
WNDPROC oWndProc;
ImFont* m_pFont;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool ShowMenu = true;

void ToggleButton(const char* str_id, bool* v)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = 16.0f;
	float width = height * 1.60f;
	float radius = height * 0.50f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	if (ImGui::IsItemClicked())
		*v = !*v;

	float t = *v ? 1.0f : 0.0f;

	ImGuiContext& g = *GImGui;
	float ANIM_SPEED = 0.20f;
	if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
	{
		float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
		t = *v ? (t_anim) : (1.0f - t_anim);
	}

	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.0f, 0.0f, 0.80f, 1.0f), ImVec4(0.0f, 0.0f, 0.80f, 1.0f), t));
	else
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.0f, 0.0f, 1.0f), ImVec4(0.0f, 0.85f, 0.0f, 1.0f), t));

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius + 0.80f, IM_COL32(255, 255, 255, 255));
}

VOID AddMarker(ImGuiWindow& window, float width, float height, float* start, PVOID pawn, LPCSTR text, ImU32 color) {
	auto root = Util::GetPawnRootLocation(pawn);
	if (root) {
		auto pos = *root;
		float dx = start[0] - pos.X;
		float dy = start[1] - pos.Y;
		float dz = start[2] - pos.Z;

		if (Util::WorldToScreen(width, height, &pos.X)) {
			float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

			if (dist < 250)
			{
				CHAR modified[0xFF] = { 0 };
				snprintf(modified, sizeof(modified), ("%s [%dm]"), text, static_cast<INT>(dist));

				auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, modified);
				window.DrawList->AddText(ImVec2(pos.X - size.x / 2.0f, pos.Y - size.y / 2.0f), ImGui::GetColorU32(color), modified);
			}
		}
	}
}

FLOAT GetDistance(ImGuiWindow& window, float width, float height, float* start, PVOID pawn) {
	auto root = Util::GetPawnRootLocation(pawn);
	float dist;
	if (root) {
		auto pos = *root;
		float dx = start[0] - pos.X;
		float dy = start[1] - pos.Y;
		float dz = start[2] - pos.Z;

		if (Util::WorldToScreen(width, height, &pos.X)) {
			dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 1000.0f;
			return dist;
		}
	}
}

__declspec(dllexport) LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_KEYUP && (wParam == VK_F8 || (ShowMenu && wParam == VK_ESCAPE))) {
		ShowMenu = !ShowMenu;
		ImGui::GetIO().MouseDrawCursor = ShowMenu;
	}
	else if (msg == WM_QUIT && ShowMenu) {
		ExitProcess(0);
	}

	if (ShowMenu) {
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return TRUE;
	}

	return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

extern uint64_t base_address = 0;
DWORD processID;
const ImVec4 color = { 255.0,255.0,255.0,1 };
const ImVec4 red = { 0.65,0,0,1 };
const ImVec4 white = { 255.0,255.0,255.0,1 };
const ImVec4 green = { 0.03,0.81,0.14,1 };
const ImVec4 blue = { 0.21960784313,0.56470588235,0.90980392156,1.0 };

auto BoxColor = ImGui::GetColorU32({ config_system.item.BoxNotVisibleColor[0], config_system.item.BoxNotVisibleColor[1], config_system.item.BoxNotVisibleColor[2], config_system.item.BoxNotVisibleColor[3] }); //box color when not visible
void DrawCorneredBox(int X, int Y, int W, int H, const ImU32& color, int thickness) {
	float lineW = (W / 3);
	float lineH = (H / 3);

	//black outlines
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);

	//corners
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}

ImGuiWindow& BeginScene() {
	ImGui_ImplDX11_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin(("##scene"), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	return *ImGui::GetCurrentWindow();
}

char streamsnipena[256] = "Username";

VOID EndScene(ImGuiWindow& window) {
	window.DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.17f, 0.18f, 0.2f, 1.0f));

	const ImVec4 Purple = { 0.80f, 0.00f, 0.80f, 1.00f };
	const ImVec4 LY = { 1.00f, 1.00f, 0.80f, 1.00f };
	const ImVec4 LB = { 0.80f, 1.00f, 1.00f, 1.00f };
	const ImVec4 cyan = { 0.00f, 0.93f, 1.00f, 1.00f };
	const ImVec4 clear = { 1.00f, 1.00f, 1.00f, 0.00f };
	const ImVec4 orange = { 0.51f, 0.36f, 0.15f, 1.00f };
	const ImVec4 pink = { 0.79f, 0.19f, 0.65f, 1.00f };
	const ImVec4 color = { 255.0,255.0,255.0,1 };
	const ImVec4 red = { 0.65,0,0,1 };
	const ImVec4 red1 = { 0.65,0,0,0.5 };
	const ImVec4 white = { 255.0,255.0,255.0,1 };
	const ImVec4 green = { 0.03,0.81,0.14,1 };
	const ImVec4 blue = { 0.21960784313,0.56470588235,0.90980392156,1.0 };
	static bool VarsMenuOpened = true;

	if (ShowMenu) {
		ImGuiStyle* Style = &ImGui::GetStyle();
		Style->ItemSpacing = ImVec2(4, 3);
		Style->WindowRounding = 1.0f;
		Style->FrameBorderSize = 1;
		Style->Colors[ImGuiCol_WindowBg] = ImColor(0.00f, 0.00f, 0.00f, 0.00f); // 0, 0, 0, 0
		Style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		Style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		Style->Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		Style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		Style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		Style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		Style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		Style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		Style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		Style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		Style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		Style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		Style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		Style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		Style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		Style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		Style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		Style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		Style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		Style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		Style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		Style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		Style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		Style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		Style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		Style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		Style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		Style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		Style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		Style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		Style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		Style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		Style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		Style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		Style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

		static int iTab;

		ImGui::Begin("Covid-69 [BETA-RELEASE]", 0, ImVec2(600, 350), 1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar); {
			ImGui::Text("Covid-69 [BETA-RELEASE]");
			ImGui::Text("Made by YTMcGamer#1337 and Kenny's Cheetos#6969");
			{
				ImGui::Columns(2, nullptr, false);
				Style->ItemSpacing = ImVec2(0.f, 0.f);
				ImGui::SetColumnOffset(1, 230);
				ImGui::BeginChild("##tabs", ImVec2(600, 250), false);
				{
					if (ImGui::Button("Aimbot", ImVec2(200, 45))) iTab = 0;
					if (ImGui::Button("Visuals", ImVec2(200, 45))) iTab = 1;
					if (ImGui::Button("Exploits", ImVec2(200, 45))) iTab = 2;
					ImGui::Text("");
					ImGui::Text("");
					ImGui::Text("");
					ImGui::Text("");
					ImGui::Text("");
					ImGui::Text("");
					ImGui::Text("Press F8 to open the Menu :)");
				}

				ImGui::EndChild();
				ImGui::NextColumn();
				if (iTab == 0) {
					ImGui::TextColored(ImColor(pink), "Aimbot");
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "memory Aimbot"); ImGui::SameLine(); ImGui::Checkbox(("memory aimbot##checkbox"), &config_system.item.Aimbot);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "silent Aimbot"); ImGui::SameLine(); ImGui::Checkbox(("silent aimbot##checkbox"), &config_system.item.SilentAimbot);
					ImGui::TextColored(ImColor(pink), "Sliders");
					ImGui::Text(" ");
					ImGui::SliderFloat(("FOV Circle"), &config_system.item.AimbotFOV, 5.0f, 1000.0f, ("%.2f"));
					ImGui::Text(" ");
					ImGui::SliderFloat(("FOV Slider"), &config_system.item.FOV, 5.0f, 180.0f, ("%.2f"));
					ImGui::Text(" ");
					ImGui::SliderFloat(("Aim smooth"), &config_system.item.AimbotSlow, 5.0f, 30.0f, ("%.2f"));
				}
				if (iTab == 1) {
					ImGui::TextColored(ImColor(pink), "ESP");
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "Player names TEST");  ImGui::SameLine();  ImGui::Checkbox(("player name##checkbox"), &config_system.item.PlayerNames);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "Player box");  ImGui::SameLine();  ImGui::Checkbox(("player box##checkbox"), &config_system.item.PlayerBox);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "Vehicle ESP");  ImGui::SameLine();  ImGui::Checkbox(("Vehicle ESP##checkbox"), &config_system.item.Vehicle);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "Player Cornor TEST");  ImGui::SameLine();  ImGui::Checkbox(("player corner##checkbox"), &config_system.item.PlayersCorner);
					ImGui::Text(" ");
				}
				if (iTab == 2) {
					ImGui::TextColored(ImColor(pink), "Exploits");
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "spinbot");  ImGui::SameLine();  ImGui::Checkbox(("spinbot##checkbox"), &config_system.item.SpinBot);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "debug2");  ImGui::SameLine();  ImGui::Checkbox(("debug2##checkbox"), &config_system.item.debug2);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "Info TEST");  ImGui::SameLine();  ImGui::Checkbox(("debug2##checkbox"), &config_system.item.Info);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(cyan), "projectile teleport");  ImGui::SameLine();  ImGui::Checkbox(("projectile teleport##checkbox"), &config_system.item.BulletTP);
					ImGui::Text(" ");
					ImGui::TextColored(ImColor(red), "WARNING ITEMS HERE CAN GET YOU BANNED RATHER QUICK!");
				}
				ImGui::End();
			}
		}
	}

	ImGui::PopStyleColor();

	ImGui::Render();
}
VOID AddLine(ImGuiWindow& window, float width, float height, float a[3], float b[3], ImU32 color, float& minX, float& maxX, float& minY, float& maxY) {
	float ac[3] = { a[0], a[1], a[2] };
	float bc[3] = { b[0], b[1], b[2] };
	if (Util::WorldToScreen(width, height, ac) && Util::WorldToScreen(width, height, bc)) {
		window.DrawList->AddLine(ImVec2(ac[0], ac[1]), ImVec2(bc[0], bc[1]), color, 2.0f);

		minX = min(ac[0], minX);
		minX = min(bc[0], minX);

		maxX = max(ac[0], maxX);
		maxX = max(bc[0], maxX);

		minY = min(ac[1], minY);
		minY = min(bc[1], minY);

		maxY = max(ac[1], maxY);
		maxY = max(bc[1], maxY);
	}
}

__declspec(dllexport) HRESULT PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
	static float width = 0;
	static float height = 0;
	static HWND hWnd = 0;
	using f_present = HRESULT(__stdcall*)(IDXGISwapChain* pthis, UINT sync_interval, UINT flags);
	f_present o_present = nullptr;
	if (!device) {
		swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device));
		device->GetImmediateContext(&immediateContext);

		ID3D11Texture2D* renderTarget = nullptr;
		swapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget));
		device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
		renderTarget->Release();

		ID3D11Texture2D* backBuffer = 0;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);
		D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
		backBuffer->GetDesc(&backBufferDesc);

		hWnd = FindWindow((L"UnrealWindow"), (L"Fortnite  "));
		if (!width) {
			oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));
		}

		width = (float)backBufferDesc.Width;
		height = (float)backBufferDesc.Height;
		backBuffer->Release();

		ImGui::GetIO().Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\trebucbd.ttf"), 13.0f);

		ImGui_ImplDX11_Init(hWnd, device, immediateContext);
		ImGui_ImplDX11_CreateDeviceObjects();
	}
	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
	////// reading
	auto& window = BeginScene();
	////// readin
	auto success = FALSE;
	do {
		float closestDistance = FLT_MAX;
		PVOID closestPawn = NULL;

		auto world = *Offsets::uWorld;
		if (!world) break;

		auto gameInstance = ReadPointer(world, Offsets::Engine::World::OwningGameInstance);
		if (!gameInstance) break;

		auto localPlayers = ReadPointer(gameInstance, Offsets::Engine::GameInstance::LocalPlayers);
		if (!localPlayers) break;

		auto localPlayer = ReadPointer(localPlayers, 0);
		if (!localPlayer) break;

		auto localPlayerController = ReadPointer(localPlayer, Offsets::Engine::Player::PlayerController);
		if (!localPlayerController) break;

		auto localPlayerPawn = reinterpret_cast<UObject*>(ReadPointer(localPlayerController, Offsets::Engine::PlayerController::AcknowledgedPawn));
		if (!localPlayerPawn) break;

		auto localPlayerWeapon = ReadPointer(localPlayerPawn, Offsets::FortniteGame::FortPawn::CurrentWeapon);
		if (!localPlayerWeapon) break;

		auto localPlayerRoot = ReadPointer(localPlayerPawn, Offsets::Engine::Actor::RootComponent);
		if (!localPlayerRoot) break;

		auto localPlayerState = ReadPointer(localPlayerPawn, Offsets::Engine::Pawn::PlayerState);
		if (!localPlayerState) break;

		auto localPlayerLocation = reinterpret_cast<float*>(reinterpret_cast<PBYTE>(localPlayerRoot) + Offsets::Engine::SceneComponent::RelativeLocation);
		auto localPlayerTeamIndex = ReadDWORD(localPlayerState, Offsets::FortniteGame::FortPlayerStateAthena::TeamIndex);

		auto weaponName = Util::GetObjectFirstName((UObject*)localPlayerWeapon);
		auto isProjectileWeapon = wcsstr(weaponName.c_str(), L"");

		Core::LocalPlayerPawn = localPlayerPawn;
		Core::LocalPlayerController = localPlayerController;

		std::vector<PVOID> playerPawns;
		for (auto li = 0UL; li < ReadDWORD(world, Offsets::Engine::World::Levels + sizeof(PVOID)); ++li) {
			auto levels = ReadPointer(world, Offsets::Engine::World::Levels);
			if (!levels) break;

			auto level = ReadPointer(levels, li * sizeof(PVOID));
			if (!level) continue;

			for (auto ai = 0UL; ai < ReadDWORD(level, Offsets::Engine::Level::AActors + sizeof(PVOID)); ++ai) {
				auto actors = ReadPointer(level, Offsets::Engine::Level::AActors);
				if (!actors) break;

				auto pawn = reinterpret_cast<UObject*>(ReadPointer(actors, ai * sizeof(PVOID)));
				if (!pawn || pawn == localPlayerPawn) continue;

				auto name = Util::GetObjectFirstName(pawn);
				if (wcsstr(name.c_str(), L"PlayerPawn_Athena_C") || wcsstr(name.c_str(), L"PlayerPawn_Athena_Phoebe_C") || wcsstr(name.c_str(), L"BP_MangPlayerPawn") || wcsstr(name.c_str(), L"HoagieVehicle_C")) {
					playerPawns.push_back(pawn);
				}

				else if (config_system.item.Vehicle)
				{
					if (wcsstr(name.c_str(), (L"MeatballVehicle_L")) || wcsstr(name.c_str(), (L"JackalVehicle_Athena_C")) || wcsstr(name.c_str(), (L"FerretVehicle_C")) || wcsstr(name.c_str(), (L"AntelopeVehicle_C")) || wcsstr(name.c_str(), (L"GolfCartVehicleSK_C")) || wcsstr(name.c_str(), (L"TestMechVehicle_C")) || wcsstr(name.c_str(), (L"OctopusVehicle_C")) || wcsstr(name.c_str(), (L"ShoppingCartVehicleSK_C")) || wcsstr(name.c_str(), (L"HoagieVehicle_C")))
					{
						auto VehicleRoot = Util::GetPawnRootLocation(pawn);
						if (VehicleRoot) {
							auto VehiclePos = *VehicleRoot;
							float dx = localPlayerLocation[0] - VehiclePos.X;
							float dy = localPlayerLocation[1] - VehiclePos.Y;
							float dz = localPlayerLocation[2] - VehiclePos.Z;

							if (Util::WorldToScreen(width, height, &VehiclePos.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 1500)
								{
									AddMarker(window, width, height, localPlayerLocation, pawn, "Vehicle", ImGui::GetColorU32({ 0, 65, 200, 255 }));
								}
							}
						}
					}
				}
				else if (config_system.item.Llama && wcsstr(name.c_str(), L"AthenaSupplyDrop_Llama")) {
					AddMarker(window, width, height, localPlayerLocation, pawn, "Llama", ImGui::GetColorU32({ 0.03f, 0.78f, 0.91f, 1.0f }));
				}
				if (config_system.item.Chest && wcsstr(name.c_str(), L"Tiered_Chest") && !((ReadBYTE(pawn, Offsets::FortniteGame::BuildingContainer::bAlreadySearched) >> 7) & 1)) {
					AddMarker(window, width, height, localPlayerLocation, pawn, "Chest", ImGui::GetColorU32({ 255,255,0,255 }));
				}
				else if (config_system.item.Ammo && wcsstr(name.c_str(), L"Tiered_Ammo") && !((ReadBYTE(pawn, Offsets::FortniteGame::BuildingContainer::bAlreadySearched) >> 7) & 1)) {
					AddMarker(window, width, height, localPlayerLocation, pawn, "Ammo Box", ImGui::GetColorU32({ 0.75f, 0.75f, 0.75f, 1.0f }));
				}
				else if (config_system.item.chopper && wcsstr(name.c_str(), L"HoagieVehicle_C")) {
					AddMarker(window, width, height, localPlayerLocation, pawn, "Chopper", ImGui::GetColorU32({ 1.0f, 0.0f, 0.0f, 1.0f }));
				}
				else if (config_system.item.boat && wcsstr(name.c_str(), L"MeatballVehicle_L")) {
					AddMarker(window, width, height, localPlayerLocation, pawn, "Boat", ImGui::GetColorU32({ 1.0f, 0.0f, 0.0f, 1.0f }));
				}
			}
		}

		float CurrentAimPointer[3] = { 0 };
		float AimPointer;
		if (config_system.item.AimPoint == 0) {
			AimPointer = BONE_HEAD_ID;
		}
		else if (config_system.item.AimPoint == 1) {
			AimPointer = BONE_NECK_ID;
		}
		else if (config_system.item.AimPoint == 2) {
			AimPointer = BONE_CHEST_ID;
		}
		else if (config_system.item.AimPoint == 3) {
			AimPointer = BONE_PELVIS_ID;
		}
		else if (config_system.item.AimPoint == 4) {
			AimPointer = BONE_RIGHTELBOW_ID;
		}
		else if (config_system.item.AimPoint == 5) {
			AimPointer = BONE_LEFTELBOW_ID;
		}
		else if (config_system.item.AimPoint == 6) {
			AimPointer = BONE_RIGHTTHIGH_ID;
		}
		else if (config_system.item.AimPoint == 7) {
			AimPointer = BONE_LEFTTHIGH_ID;
		}
		else if (config_system.item.AimPoint == 8) { // automatic
		}

		for (auto pawn : playerPawns)
		{
			auto state = ReadPointer(pawn, Offsets::Engine::Pawn::PlayerState);
			if (!state) continue;

			auto mesh = ReadPointer(pawn, Offsets::Engine::Character::Mesh);
			if (!mesh) continue;

			auto bones = ReadPointer(mesh, Offsets::Engine::StaticMeshComponent::StaticMesh);
			if (!bones) bones = ReadPointer(mesh, Offsets::Engine::StaticMeshComponent::StaticMesh + 0x10);
			if (!bones) continue;
			float compMatrix[4][4] = { 0 };
			Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + 0x1C0), compMatrix);

			// root
			float root[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 0, root);

			// Top
			float head[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, BONE_HEAD_ID, head);

			float head2[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, BONE_HEAD_ID, head2);

			float body[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 5, body);

			float neck[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 65, neck);

			float chest[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 36, chest);

			float pelvis[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 2, pelvis);

			// Arms
			float leftShoulder[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 9, leftShoulder);

			float rightShoulder[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 62, rightShoulder);

			float leftElbow[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 10, leftElbow);

			float rightElbow[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 38, rightElbow);

			float leftHand[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 11, leftHand);

			float rightHand[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 39, rightHand);

			// Legs
			float leftLeg[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 67, leftLeg);

			float rightLeg[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 74, rightLeg);

			float leftThigh[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 73, leftThigh);

			float rightThigh[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 80, rightThigh);

			float leftFoot[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 68, leftFoot);

			float rightFoot[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 75, rightFoot);

			float leftFeet[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 71, leftFeet);

			float rightFeet[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 78, rightFeet);

			float leftFeetFinger[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 72, leftFeetFinger);

			float rightFeetFinger[3] = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 79, rightFeetFinger);

			auto color = ImGui::GetColorU32({ config_system.item.PlayerNotVisibleColor[0], config_system.item.PlayerNotVisibleColor[1], config_system.item.PlayerNotVisibleColor[2], 1.0f });
			FVector viewPoint = { 0 };

			if (ReadDWORD(state, 0xE88) == localPlayerTeamIndex) {
				color = ImGui::GetColorU32({ 0.0f, 1.0f, 0.0f, 1.0f });
			}
			else if (!config_system.item.CheckVisible) {
				auto w2s = *reinterpret_cast<FVector*>(CurrentAimPointer);
				if (Util::WorldToScreen(width, height, &w2s.X)) {
					auto dx = w2s.X - (width / 2);
					auto dy = w2s.Y - (height / 2);
					auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
					if (dist < config_system.item.AimbotFOV && dist < closestDistance) {
						closestDistance = dist;
						closestPawn = pawn;
					}
				}
			}
			else if ((ReadBYTE(pawn, Offsets::FortniteGame::FortPawn::bIsDBNO) & 1) && (isProjectileWeapon || Util::LineOfSightTo(localPlayerController, pawn, &viewPoint))) {
				color = ImGui::GetColorU32({ config_system.item.PlayerVisibleColor[0], config_system.item.PlayerVisibleColor[1], config_system.item.PlayerVisibleColor[2], 1.0f });
				if (config_system.item.AutoAimbot) {
					if (config_system.item.AimPoint = 8) {
						Util::GetBoneLocation(compMatrix, bones, BONE_HEAD_ID, CurrentAimPointer);
						auto dx = CurrentAimPointer[0] - localPlayerLocation[0];
						auto dy = CurrentAimPointer[1] - localPlayerLocation[1];
						auto dz = CurrentAimPointer[2] - localPlayerLocation[2];
						auto dist = dx * dx + dy * dy + dz * dz;
						if (dist < closestDistance) {
							closestDistance = dist;
							closestPawn = pawn;
						}
						else {
							Util::GetBoneLocation(compMatrix, bones, BONE_NECK_ID, CurrentAimPointer);
							auto dx = CurrentAimPointer[0] - localPlayerLocation[0];
							auto dy = CurrentAimPointer[1] - localPlayerLocation[1];
							auto dz = CurrentAimPointer[2] - localPlayerLocation[2];
							auto dist = dx * dx + dy * dy + dz * dz;
							if (dist < closestDistance) {
								closestDistance = dist;
								closestPawn = pawn;
							}
							else {
								Util::GetBoneLocation(compMatrix, bones, BONE_CHEST_ID, CurrentAimPointer);
								auto dx = CurrentAimPointer[0] - localPlayerLocation[0];
								auto dy = CurrentAimPointer[1] - localPlayerLocation[1];
								auto dz = CurrentAimPointer[2] - localPlayerLocation[2];
								auto dist = dx * dx + dy * dy + dz * dz;
								if (dist < closestDistance) {
									closestDistance = dist;
									closestPawn = pawn;
								}
								else {
									Util::GetBoneLocation(compMatrix, bones, BONE_PELVIS_ID, CurrentAimPointer);
									auto dx = CurrentAimPointer[0] - localPlayerLocation[0];
									auto dy = CurrentAimPointer[1] - localPlayerLocation[1];
									auto dz = CurrentAimPointer[2] - localPlayerLocation[2];
									auto dist = dx * dx + dy * dy + dz * dz;
									if (dist < closestDistance) {
										closestDistance = dist;
										closestPawn = pawn;
									}
								}
							}
						}
					}
					else {
						auto dx = CurrentAimPointer[0] - localPlayerLocation[0];
						auto dy = CurrentAimPointer[1] - localPlayerLocation[1];
						auto dz = CurrentAimPointer[2] - localPlayerLocation[2];
						auto dist = dx * dx + dy * dy + dz * dz;
						if (dist < closestDistance) {
							closestDistance = dist;
							closestPawn = pawn;
						}
					}
				}
				else
				{
					auto w2s = *reinterpret_cast<FVector*>(CurrentAimPointer);
					if (Util::WorldToScreen(width, height, &w2s.X)) {
						auto dx = w2s.X - (width / 2);
						auto dy = w2s.Y - (height / 2);
						auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
						if (dist < config_system.item.AimbotFOV && dist < closestDistance) {
							closestDistance = dist;
							closestPawn = pawn;
						}
					}
				}
			}

			//if (!config_system.item.Players) continue;

			if (config_system.item.CrosshairSize) {
				window.DrawList->AddLine(ImVec2(width / 2 - 15, height / 2), ImVec2(width / 2 + 15, height / 2), ImGui::GetColorU32(white), 2);
				window.DrawList->AddLine(ImVec2(width / 2, height / 2 - 15), ImVec2(width / 2, height / 2 + 15), ImGui::GetColorU32(white), 2);
			}

			if (config_system.item.PlayerLines) {
				auto end = *reinterpret_cast<FVector*>(CurrentAimPointer);
				if (Util::WorldToScreen(width, height, &end.X)) {
					if (ReadDWORD(state, 0xE88) != localPlayerTeamIndex) {
						if (config_system.item.LineESP) {
							window.DrawList->AddLine(ImVec2(width / 2, height / 2), ImVec2(end.X, end.Y), ImGui::GetColorU32({ config_system.item.LineESP[0], config_system.item.LineESP[1], config_system.item.LineESP[2], 1.0f }));
						}
					}
				}
			}

			float minX = FLT_MAX;
			float maxX = -FLT_MAX;
			float minY = FLT_MAX;
			float maxY = -FLT_MAX;

			if (config_system.item.PlayerBox) {
				auto Spikey1 = ImVec2(maxX + 4.0f, maxY + 4.0f);
				auto Spikey2 = ImVec2(minX - 4.0f, minY - 4.0f);;

				window.DrawList->AddRect(Spikey1, Spikey2, ImGui::GetColorU32({ white }), 0.5, 15, 1.5f);
			}

			if (config_system.item.Players) {
				AddLine(window, width, height, head, neck, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, neck, pelvis, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, chest, leftShoulder, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, chest, rightShoulder, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, leftShoulder, leftElbow, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, rightShoulder, rightElbow, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, leftElbow, leftHand, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, rightElbow, rightHand, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, pelvis, leftLeg, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, pelvis, rightLeg, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, leftLeg, leftThigh, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, rightLeg, rightThigh, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, leftThigh, leftFoot, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, rightThigh, rightFoot, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, leftFoot, leftFeet, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, rightFoot, rightFeet, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, leftFeet, leftFeetFinger, color, minX, maxX, minY, maxY);
				AddLine(window, width, height, rightFeet, rightFeetFinger, color, minX, maxX, minY, maxY);
			}

			if (minX < width && maxX > 0 && minY < height && maxY > 0) {
				//ALL CORRECT
				auto topLeft = ImVec2(minX - 3.0f, minY - 3.0f);
				auto bottomRight = ImVec2(maxX + 3.0f, maxY);
				auto topRight = ImVec2(maxX + 3.0f, minY - 3.0f);
				auto bottomLeft = ImVec2(minX - 3.0f, maxY);

				/*float dist;
				if (dist >= 100)
				dist = 75;*/

				auto root = Util::GetPawnRootLocation(pawn);
				float dx;
				float dy;
				float dz;
				float dist;
				if (root) {
					auto pos = *root;
					dx = localPlayerLocation[0] - pos.X;
					dy = localPlayerLocation[1] - pos.Y;
					dz = localPlayerLocation[2] - pos.Z;

					if (Util::WorldToScreen(width, height, &pos.X)) {
						dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 1500.0f;
					}
				}

				if (dist >= 100)
					dist = 75;
				else if (config_system.item.PlayersCorner) {
					auto topLeft = ImVec2(minX - 3.0f, minY - 3.0f);
					auto bottomRight = ImVec2(maxX + 3.0f, maxY + 3.0f);
					float lineW = (width / 5);
					float lineH = (height / 6);
					float lineT = 1;

					auto w2sa = *reinterpret_cast<FVector*>(head);
					Util::WorldToScreen(width, height, &w2sa.X);
					Util::WorldToScreen(width, height, &w2sa.Y);
					auto X = w2sa.X;
					auto Y = w2sa.Y;

					auto bottomRightLEFT = ImVec2(maxX - config_system.item.CornerSize + dist, maxY + 2.5f);
					auto bottomRightUP = ImVec2(maxX + 3.0f, maxY - config_system.item.CornerSize + dist);
					auto topRight = ImVec2(maxX + 3.0f, minY - 3.0f);
					auto topRightLEFT = ImVec2(maxX - config_system.item.CornerSize + dist, minY - 3.0f);
					auto topRightDOWN = ImVec2(maxX + 3.0f, minY + config_system.item.CornerSize - dist);

					auto bottomLeft = ImVec2(minX - 3.0f, maxY + 3.f);
					auto bottomLeftRIGHT = ImVec2(minX + config_system.item.CornerSize - dist, maxY + 3.f);
					auto bottomLeftUP = ImVec2(minX - 3.0f, maxY - config_system.item.CornerSize + dist);
					auto topLeftRIGHT = ImVec2(minX + config_system.item.CornerSize - dist, minY - 3.0f);
					auto topLeftDOWN = ImVec2(minX - 3.0f, minY + config_system.item.CornerSize - dist);

					ImU32 kek = ImGui::GetColorU32({ ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.0f }) });
					window.DrawList->AddLine(topLeftRIGHT, topLeft, ImGui::GetColorU32({ white }), 1.00f);
					window.DrawList->AddLine(topLeftDOWN, topLeft, ImGui::GetColorU32({ white }), 1.00f);

					window.DrawList->AddLine(bottomRightLEFT, bottomRight, ImGui::GetColorU32({ white }), 1.5f);
					window.DrawList->AddLine(bottomRightUP, bottomRight, ImGui::GetColorU32({ white }), 1.5f);

					window.DrawList->AddLine(topRightLEFT, topRight, ImGui::GetColorU32({ white }), 1.5f);
					window.DrawList->AddLine(topRightDOWN, topRight, ImGui::GetColorU32({ white }), 1.5f);

					window.DrawList->AddLine(bottomLeftRIGHT, bottomLeft, ImGui::GetColorU32({ white }), 1.5f);
					window.DrawList->AddLine(bottomLeftUP, bottomLeft, ImGui::GetColorU32({ white }), 1.5f);
				}

				if (!config_system.item.AutoAimbot && config_system.item.AimbotFOV) {
					window.DrawList->AddCircle(ImVec2(width / 2, height / 2), config_system.item.AimbotFOV, ImGui::GetColorU32({ white }), 128);
				}

				/*float dist;
				if (dist >= 100)
					dist = 75;*/

				if (config_system.item.PlayerNames) {
					FString playerName;
					Core::ProcessEvent(state, Offsets::Engine::PlayerState::GetPlayerName, &playerName, 0);
					if (playerName.c_str()) {
						CHAR copy[0xFF] = { 0 };
						auto w2s = *reinterpret_cast<FVector*>(head);
						float dist;
						if (Util::WorldToScreen(width, height, &w2s.X)) {
							auto dx = w2s.X;
							auto dy = w2s.Y;
							auto dz = w2s.Z;
							dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;
						}
						CHAR lel[0xFF] = { 0 };
						wcstombs(lel, playerName.c_str(), sizeof(lel));
						Util::FreeInternal(playerName.c_str());
						snprintf(copy, sizeof(copy), ("%s [%dm]"), lel, static_cast<INT>(dist));
						auto centerTop = ImVec2((topLeft.x + bottomRight.x) / 2.0f, topLeft.y);
						auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, copy);
						//	window.DrawList->AddRectFilled(ImVec2(centerTop.x - size.x / 2.0f, centerTop.y - size.y + 3.0f), ImVec2(centerTop.x + size.x / 2.0f, centerTop.y), ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 0.4f }));
						ImVec2 kek = ImVec2(centerTop.x - size.x / 2.0f + 10, centerTop.y - size.y);
						//	window.DrawList->AddRectFilled(kek, ImVec2(centerTop.y - size.y), ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 0.20f }));
						std::string jsj = copy;
						if (jsj.find(streamsnipena) != std::string::npos) {
							window.DrawList->AddText(ImVec2(centerTop.x - size.x / 2.0f + 10, centerTop.y - size.y), ImGui::GetColorU32({ 1.0f, 0.0f, 1.0f, 1.0f }), copy);
						}
						else
						{
							window.DrawList->AddText(ImVec2(centerTop.x - size.x / 2.0f + 10, centerTop.y - size.y), color, copy);
						}
					}
				}
			}
		}

		if (config_system.item.Aimbot && closestPawn && Util::SpoofCall(GetAsyncKeyState, config_system.keybind.AimbotLock) < 0 && Util::SpoofCall(GetForegroundWindow) == hWnd) {
			Core::TargetPawn = closestPawn;
			Core::NoSpread = TRUE;
			if (config_system.item.Aimbot && config_system.item.AntiAim && Util::SpoofCall(GetAsyncKeyState, config_system.keybind.AntiAim)) {
				int rnd = rand();
				FRotator args = { 0 };
				args.Yaw = rnd;
				Core::ProcessEvent(Core::LocalPlayerController, Offsets::Engine::Controller::ClientSetRotation, &args, 0);
				//mouse_event(000001, rnd, NULL, NULL, NULL); old anti aim
			}
		}
		else {
			Core::TargetPawn = nullptr;
			Core::NoSpread = FALSE;
		}

		if (config_system.item.SpinBot && Util::SpoofCall(GetAsyncKeyState, config_system.keybind.Spinbot) && Util::SpoofCall(GetForegroundWindow) == hWnd) {
			int rnd = rand();
			FRotator args = { 0 };
			args.Yaw = rnd;
			if (closestPawn) {
				Core::TargetPawn = closestPawn;
				Core::NoSpread = TRUE;
			}
			else {
				Core::ProcessEvent(Core::LocalPlayerController, Offsets::Engine::Controller::ClientSetRotation, &args, 0);
			}
			config_system.item.AutoAimbot = true;
			config_system.item.SilentAimbot = true;
		}
		else {
		}

		if (config_system.item.FlickAimbot && closestPawn && Util::SpoofCall(GetAsyncKeyState, config_system.keybind.AimbotShoot) < 0 && Util::SpoofCall(GetForegroundWindow) == hWnd) {
			Core::TargetPawn = closestPawn;
			Core::NoSpread = TRUE;
		}

		if (config_system.item.AutoAim && closestPawn && Util::SpoofCall(GetForegroundWindow) == hWnd) {
			//mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			Core::TargetPawn = closestPawn;
			Core::NoSpread = TRUE;
		}

		if (config_system.item.SpamAutoAim && closestPawn && Util::SpoofCall(GetForegroundWindow) == hWnd) {
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			Core::TargetPawn = closestPawn;
			Core::NoSpread = TRUE;
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			Core::TargetPawn = nullptr;
			Core::NoSpread = FALSE;
		}

		success = TRUE;
	} while (FALSE);

	if (!success) {
		Core::LocalPlayerController = Core::LocalPlayerPawn = Core::TargetPawn = nullptr;
	}
	EndScene(window);
	return PresentOriginal(swapChain, syncInterval, flags);
}

__declspec(dllexport) HRESULT ResizeHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
	ImGui_ImplDX11_Shutdown();
	renderTargetView->Release();
	immediateContext->Release();
	device->Release();
	device = nullptr;

	return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

bool Render::Initialize() {
	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	auto featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = FindWindow((L"UnrealWindow"), (L"Fortnite  "));
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &featureLevel, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context))) {
		MessageBox(0, L"Critical error have happened\nPlease contact an admin with the error code:\n0x0001b", L"Error", MB_ICONERROR);
		return FALSE;
	}

	auto table = *reinterpret_cast<PVOID**>(swapChain);
	auto present = table[8];
	auto resize = table[13];

	context->Release();
	device->Release();
	swapChain->Release();

	const auto pcall_present_discord = Helper::PatternScan(Discord::GetDiscordModuleBase(), "FF 15 ? ? ? ? 8B D8 E8 ? ? ? ? E8 ? ? ? ? EB 10");
	auto presentSceneAdress = Helper::PatternScan(Discord::GetDiscordModuleBase(),
		"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B D9 41 8B F8");

	DISCORD.HookFunction(presentSceneAdress, (uintptr_t)PresentHook, (uintptr_t)&PresentOriginal);

	DISCORD.HookFunction(presentSceneAdress, (uintptr_t)ResizeHook, (uintptr_t)&PresentOriginal);
	return TRUE;
}