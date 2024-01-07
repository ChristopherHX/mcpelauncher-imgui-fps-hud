#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string> 
#include <EGL/egl.h>


static void (*mcpelauncher_preinithook)(const char*sym, void*val, void **orig);

static decltype(&eglSwapBuffers) eglSwapBuffers_orig;
static decltype(&eglMakeCurrent) eglMakeCurrent_orig;

static EGLBoolean eglSwapBuffers_(EGLDisplay dpy, EGLSurface surface) {
    printf("%s\n", "eglSwapBuffers_");
    return eglSwapBuffers_orig(dpy, surface);
}

static EGLBoolean eglMakeCurrent_(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    printf("%s\n", "eglMakeCurrent_");
    return eglMakeCurrent_orig(dpy, draw, read, ctx);
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
