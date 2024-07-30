#pragma once
#include <exception>
#include <CoreVideo/CoreVideo.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

#define CV_CHECK(...) \
    do { \
        CVReturn status = __VA_ARGS__; \
        if (status != kCVReturnSuccess) { \
            printf("call %s failed: %d\n", #__VA_ARGS__, (int)status); \
            throw std::runtime_error("error: " + std::to_string((int)status)); \
        } \
    } while (0)




class IOSurfaceTexture
{
public:
    enum Format
    {
        BGRA = 'BGRA',
    };

private:
    IOSurfaceRef m_surface = nullptr;
    CVPixelBufferRef m_pixelBuffer = nullptr;
    CVOpenGLTextureCacheRef m_textureCache = nullptr;
    CVOpenGLTextureRef m_texture = nullptr;
    int m_width = 0;
    int m_height = 0;
    Format m_format = Format::BGRA;

    GLuint m_textureid = 0;
    GLuint m_target = 0;

private:
    // 创建 IOSurface
    static IOSurfaceRef createIOSurface(int width, int height, Format pixelFormat = Format::BGRA)
    {
        int bytesPerElement = 4;

        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);

        CFNumberRef widthRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &width);
        CFNumberRef heightRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &height);
        CFNumberRef bytesPerElementRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &bytesPerElement);
        CFNumberRef pixelFormatRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixelFormat);

        CFDictionarySetValue(dict, kIOSurfaceWidth, widthRef);
        CFDictionarySetValue(dict, kIOSurfaceHeight, heightRef);
        CFDictionarySetValue(dict, kIOSurfaceBytesPerElement, bytesPerElementRef);
        CFDictionarySetValue(dict, kIOSurfacePixelFormat, pixelFormatRef);
        CFDictionarySetValue(dict, kIOSurfaceIsGlobal, kCFBooleanTrue);

        IOSurfaceRef surface = IOSurfaceCreate(dict);

        CFRelease(dict);
        CFRelease(widthRef);
        CFRelease(heightRef);
        CFRelease(bytesPerElementRef);
        CFRelease(pixelFormatRef);

        // 打印 surface 的引用计数
        CFIndex refCount = CFGetRetainCount(surface);
        printf("Surface reference count: %ld\n", refCount);

        return surface;
    }

    void init(IOSurfaceRef surface)
    {
        CGLContextObj context = CGLGetCurrentContext();

        // 创建 CVPixelBuffer
        CV_CHECK(CVPixelBufferCreateWithIOSurface(kCFAllocatorDefault, surface, nullptr, &m_pixelBuffer));

        // https://developer.apple.com/documentation/iosurface?language=objc
        // https://developer.apple.com/documentation/corevideo/cvopengltexturecache-780?changes=l__9&language=objc
        // https://developer.apple.com/documentation/corevideo/cvopengltexture-782?changes=l__9&language=objc

        // 创建 OpenGL 纹理缓存
        // 定义像素缓冲区的属性
        CFDictionaryRef pixelBufferAttributes = nullptr;
        CV_CHECK(CVOpenGLTextureCacheCreate(kCFAllocatorDefault, NULL, context, CGLGetPixelFormat(context), pixelBufferAttributes, &m_textureCache));

        // 将像素缓冲区绑定到 OpenGL 纹理缓存
        CV_CHECK(CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, m_textureCache, m_pixelBuffer, NULL, &m_texture));

        m_surface = surface;
        m_width = (int)IOSurfaceGetWidth(surface);
        m_height = (int)IOSurfaceGetHeight(surface);
        m_textureid = CVOpenGLTextureGetName(m_texture);
        m_target = CVOpenGLTextureGetTarget(m_texture);

        OSType pixelFormat = IOSurfaceGetPixelFormat(surface);
        switch (pixelFormat)
        {
        case kCVPixelFormatType_32BGRA:
            m_format = Format::BGRA;
            break;
        default:
            throw std::runtime_error("unsupported pixel format");
        }
    }

public:
    IOSurfaceTexture(IOSurfaceRef surface)
    {
        CFRetain(surface);
        init(surface);
    }

    IOSurfaceTexture(int width, int height, Format format)
    {
        init(createIOSurface(width, height, format));
    }

    ~IOSurfaceTexture()
    {
        if (m_textureid != 0)
        {
            glDeleteTextures(1, &m_textureid);
            m_textureid = 0;
        }
        if (m_texture != nullptr)
        {
            CVOpenGLTextureRelease(m_texture);
            m_texture = nullptr;
        }
        if (m_textureCache != nullptr)
        {
            CVOpenGLTextureCacheRelease(m_textureCache);
            m_textureCache = nullptr;
        }
        if (m_pixelBuffer != nullptr)
        {
            CVPixelBufferRelease(m_pixelBuffer);
            m_pixelBuffer = nullptr;
        }
        if (m_surface != nullptr)
        {
            CFRelease(m_surface);
            m_surface = nullptr;
        }
    }

    IOSurfaceID GetSurfaceID() const
    {
        return IOSurfaceGetID(m_surface);
    }

    int GetWidth() const
    {
        return m_width;
    }

    int GetHeight() const
    {
        return m_height;
    }

    Format GetFormat() const
    {
        return m_format;
    }

    GLuint GetTexture() const
    {
        return m_textureid;
    }

    GLuint GetTarget() const
    {
        return m_target;
    }
};
