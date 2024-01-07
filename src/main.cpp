#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string> 
#include <EGL/egl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <time.h>
#include <GLES2/gl2.h>
#include <android/native_window.h>

static double g_Time = 0.0;
static std::string g_IniFilename;

static void (*mcpelauncher_preinithook)(const char*sym, void*val, void **orig);

static decltype(&eglSwapBuffers) eglSwapBuffers_orig;
static decltype(&eglMakeCurrent) eglMakeCurrent_orig;

static EGLBoolean eglSwapBuffers_(EGLDisplay dpy, EGLSurface surface) {
    ImGuiIO& io = ImGui::GetIO();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplAndroid_NewFrame();

        // Setup display size (every frame to accommodate for window resizing)
    int32_t window_width = ANativeWindow_getWidth((ANativeWindow*)surface);
    int32_t window_height = ANativeWindow_getHeight((ANativeWindow*)surface);
    int display_width = window_width;
    int display_height = window_height;

    io.DisplaySize = ImVec2((float)window_width, (float)window_height);
    if (window_width > 0 && window_height > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_width / window_width, (float)display_height / window_height);

    // Setup time step
    struct timespec current_timespec;
    clock_gettime(CLOCK_MONOTONIC, &current_timespec);
    double current_time = (double)(current_timespec.tv_sec) + (current_timespec.tv_nsec / 1000000000.0);
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;

    ImGui::NewFrame();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static int location = 1;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (location >= 0)
        {
            const float PAD = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Example: Simple overlay", nullptr, window_flags))
        {
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        }
        ImGui::End();
    }

    // Rendering
    ImGui::Render();

    //void (*glViewport)(int x, int y, int width, int height) = (decltype(glViewport)) GameWindowManager::getManager()->getProcAddrFunc()("glViewport");

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // eglSwapBuffers(g_EglDisplay, g_EglSurface);
    return eglSwapBuffers_orig(dpy, surface);
}

static EGLBoolean eglMakeCurrent_(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    auto r = eglMakeCurrent_orig(dpy, draw, read, ctx);
    if(draw != nullptr && g_IniFilename.empty()) {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // Redirect loading/saving of .ini file to our location.
        // Make sure 'g_IniFilename' persists while we use Dear ImGui.
        g_IniFilename = std::string("/tmp") + "/imgui.ini";
        io.IniFilename = g_IniFilename.c_str();;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        // ImGui_ImplAndroid_Init(g_App->window);
        // ImGuiIO& io = ImGui::GetIO();
        io.BackendPlatformName = "imgui_impl_android";

        ImGui_ImplOpenGL3_Init("#version 100");

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // - Android: The TTF files have to be placed into the assets/ directory (android/app/src/main/assets), we use our GetAssetData() helper to retrieve them.

        // We load the default font with increased size to improve readability on many devices with "high" DPI.
        // FIXME: Put some effort into DPI awareness.
        // Important: when calling AddFontFromMemoryTTF(), ownership of font_data is transfered by Dear ImGui by default (deleted is handled by Dear ImGui), unless we set FontDataOwnedByAtlas=false in ImFontConfig
        ImFontConfig font_cfg;
        font_cfg.SizePixels = 12.0f;
        io.Fonts->AddFontDefault(&font_cfg);
        //void* font_data;
        //int font_data_size;
        //ImFont* font;
        //font_data_size = GetAssetData("segoeui.ttf", &font_data);
        //font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f);
        //IM_ASSERT(font != nullptr);
        //font_data_size = GetAssetData("DroidSans.ttf", &font_data);
        //font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f);
        //IM_ASSERT(font != nullptr);
        //font_data_size = GetAssetData("Roboto-Medium.ttf", &font_data);
        //font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f);
        //IM_ASSERT(font != nullptr);
        //font_data_size = GetAssetData("Cousine-Regular.ttf", &font_data);
        //font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 15.0f);
        //IM_ASSERT(font != nullptr);
        //font_data_size = GetAssetData("ArialUni.ttf", &font_data);
        //font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != nullptr);

        // Arbitrary scale-up
        // FIXME: Put some effort into DPI awareness
        ImGui::GetStyle().ScaleAllSizes(1.0f);
    }
    return r;
}



extern "C" void __attribute__ ((visibility ("default"))) mod_preinit() {
    auto h = dlopen("libmcpelauncher_mod.so", 0);
    if(!h) {
        return;
    }
    mcpelauncher_preinithook = (decltype(mcpelauncher_preinithook)) dlsym(h, "mcpelauncher_preinithook");
    mcpelauncher_preinithook("eglSwapBuffers", (void*)&eglSwapBuffers_, (void**)&eglSwapBuffers_orig);
    mcpelauncher_preinithook("eglMakeCurrent", (void*)&eglMakeCurrent_, (void**)&eglMakeCurrent_orig);
    dlclose(h);
}

extern "C" __attribute__ ((visibility ("default"))) void mod_init() {
    auto egl = dlopen("libEGL.so", 0);
    if(!egl) {
        printf("%s\n", "egl is null!!");
    }
    // enqueueButtonPressAndRelease = (decltype(enqueueButtonPressAndRelease))dlsym(mc, "_ZN15InputEventQueue28enqueueButtonPressAndReleaseEj11FocusImpacti");
    if(!eglSwapBuffers_orig) {
        printf("%s\n", "eglSwapBuffers_orig is null!!");
        eglSwapBuffers_orig = (decltype(eglSwapBuffers_orig)) dlsym(egl, "eglSwapBuffers");
    }
    if(!eglMakeCurrent_orig) {
        printf("%s\n", "eglMakeCurrent_orig is null!!");
        eglMakeCurrent_orig = (decltype(eglMakeCurrent_orig)) dlsym(egl, "eglMakeCurrent");
    }
    dlclose(egl);
}
