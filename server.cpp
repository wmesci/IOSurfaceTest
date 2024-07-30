#include <chrono>
#include <string>
#include <unistd.h>
#include <thread>

#include "renderer.h"
#include "IOSurfaceTexture.h"

using namespace std::literals::chrono_literals;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    IOSurfaceRef surface = NULL;

    if (argc > 1)
    {
        IOSurfaceID surfaceID = (unsigned int)std::stoul(argv[1]);

        printf("SurfaceID: %d\n", surfaceID);

        surface = IOSurfaceLookup(surfaceID);
        if (surface == NULL) {
            printf("Failed to get IOSurfaceRef");
            return -1;
        }
    }
    else
    {
        printf("Usage: %s <surfaceID>\n", argv[0]);
        return -1;
    }

    // 初始化 OpenGL 上下文
    CGLContextObj context;
    {
        CGLPixelFormatAttribute attributes[] = {
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_GL4_Core,
            kCGLPFAAccelerated,
            (CGLPixelFormatAttribute)0
        };

        CGLPixelFormatObj pix;
        GLint num;
        CGLError errorCode = CGLChoosePixelFormat(attributes, &pix, &num);
        if (errorCode != kCGLNoError)
            throw std::runtime_error("CGLChoosePixelFormat failure");

        errorCode = CGLCreateContext(pix, NULL, &context);
        if (errorCode != kCGLNoError)
            throw std::runtime_error("CGLCreateContext failure");

        CGLDestroyPixelFormat(pix);
    }

    std::shared_ptr<IOSurfaceTexture> surfaceTexture;
    std::shared_ptr<IRenderer> renderer;

    while (true)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        CGLError errorCode = CGLSetCurrentContext(context);
        if (errorCode != kCGLNoError)
            throw std::runtime_error("CGLSetCurrentContext failure");

        if (renderer == nullptr)
        {
            // 创建渲染器
            renderer = std::make_shared<TestRenderer>();
            renderer->Init();

            surfaceTexture = std::make_shared<IOSurfaceTexture>(surface);
        }

        errorCode = CGLSetCurrentContext(context);
        if (errorCode != kCGLNoError)
            throw std::runtime_error("CGLSetCurrentContext failure");

        GLuint framebuffer;
        GL_CHECK(glGenFramebuffers(1, &framebuffer));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, surfaceTexture->GetTarget(), surfaceTexture->GetTexture(), 0));

        GL_CHECK(glViewport(0, 0, surfaceTexture->GetWidth(), surfaceTexture->GetHeight()));
        GL_CHECK(glClearColor(1.0f, 0.0f, 0.0f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        renderer->OnRender();

        GL_CHECK(glFinish());
        GL_CHECK(glFlush());

        GL_CHECK(glDeleteFramebuffers(1, &framebuffer));

        CGLFlushDrawable(context);

        // Calculate frame rate
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        if (elapsedTime < 1000 / 60)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60 - elapsedTime));
        }

        printf("server elapsedTime: %.2f ms\n", (float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
    }

    renderer->UnInit();

    CFRelease(surface);

    CGLDestroyContext(context);
}
