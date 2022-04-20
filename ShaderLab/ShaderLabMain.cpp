#include "pch.h"
#include "ShaderLabMain.h"
#include "Common\DirectXHelper.h"
#include <imgui/backends/imgui_impl_dx11.h>

using namespace ShaderLab;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
ShaderLabMain::ShaderLabMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<SceneRenderer>(new SceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	//io.ConfigViewportsNoDefaultParent = true;
	//io.ConfigDockingAlwaysTabBar = true;
	//io.ConfigDockingTransparentPayload = true;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplDX11_Init(deviceResources->GetD3DDevice(),deviceResources->GetD3DDeviceContext());

}

ShaderLabMain::~ShaderLabMain()
{
	// Deregister device notification
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void ShaderLabMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void ShaderLabMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_sceneRenderer->Update(m_timer);
		ImGui::NewFrame();
		if (m_show_demo_window)
			ImGui::ShowDemoWindow(&m_show_demo_window);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool ShaderLabMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::Black);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	m_sceneRenderer->Render();
	ImGui::Render();
	m_fpsTextRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void ShaderLabMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void ShaderLabMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
