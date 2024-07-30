#include <thread>

#include <Cocoa/Cocoa.h>

#include "glhelper.h"
#include "renderer.h"
#include "IOSurfaceTexture.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) NSOpenGLContext *openGLContext;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Create NSWindow
    NSRect frame = NSMakeRect(0, 0, 800, 600);
    NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    self.window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    [self.window setTitle:@"OpenGL Window"];
    [self.window makeKeyAndOrderFront:nil];
    [self.window center];
    [self.window makeFirstResponder:nil];
    
    // Create NSOpenGLContext
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    self.openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [self.openGLContext setView:[self.window contentView]];
    [self.openGLContext makeCurrentContext];
}

@end

int execCommand(const std::string& cmd) {
    printf("execCommand %s\n", cmd.c_str());

    int pid = fork();
    if (pid == -1) {
        // Fork failed
        return 0;
    } else if (pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
        exit(1);
        return 0;
    } else {
        // Parent process
        return pid;
    }
}

int main(int argc, const char * argv[])
{
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
    [NSApp finishLaunching];

    std::shared_ptr<IOSurfaceTexture> surfaceTexture;
    std::shared_ptr<ImageRenderer> renderer;
    int pid = 0;

    while (true) {
        auto startTime = std::chrono::high_resolution_clock::now();

        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (event != nil) {
            [NSApp sendEvent:event];
        }
        
        [appDelegate.openGLContext makeCurrentContext];

        if (renderer == nullptr)
        {
            renderer = std::make_shared<ImageRenderer>();
            renderer->Init();

            NSRect viewFrame = [appDelegate.openGLContext.view frame];
            int viewWidth = viewFrame.size.width;
            int viewHeight = viewFrame.size.height;

            surfaceTexture = std::make_shared<IOSurfaceTexture>(viewWidth, viewHeight, IOSurfaceTexture::Format::BGRA);

            auto tex = std::make_shared<GLTexture>(surfaceTexture->GetTexture(), surfaceTexture->GetWidth(), surfaceTexture->GetHeight(), GL_BGRA, surfaceTexture->GetTarget());
            renderer->SetTexture(tex);

            pid = execCommand("./build/Debug/server " + std::to_string(surfaceTexture->GetSurfaceID()));
        }

        GL_CHECK(glClearColor(1.0f, 0.0f, 0.0f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        renderer->OnRender();
        
        GL_CHECK(glFinish());
        GL_CHECK(glFlush());

        [appDelegate.openGLContext flushBuffer];

        // Calculate frame rate
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        if (elapsedTime < 1000 / 60)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60 - elapsedTime));
        }

        printf("client elapsedTime: %.2f ms\n", (float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());

        if ([NSApp windows].count == 0) {
            break;
        }
    }

    if (pid != 0) {
        kill(pid, SIGKILL);
    }

    return 0;
}