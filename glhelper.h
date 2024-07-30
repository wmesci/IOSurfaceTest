#pragma once
#include <string>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

#define GL_CHECK(...) \
    do { \
        __VA_ARGS__; \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            printf("OpenGL %s error: %d\n", #__VA_ARGS__, error); \
            throw std::runtime_error("OpenGL error: " + std::to_string(error)); \
        } \
    } while (0)

GLuint generateTexture(int width, int height, const unsigned char* pixel)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLint error = glGetError();
    if (error) {
        throw std::runtime_error("inputTexture failure");
        return 0;
    }
    return texture;
}

void blit(GLuint inputTexture, int input_width, int input_height, GLuint outputTexture, int output_width, int output_height)
{
    GLuint fboIds[2] = {0};
    glGenFramebuffers(2, fboIds);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboIds[0]);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputTexture, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboIds[1]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

    glBlitFramebuffer(0, 0, input_width, input_height, 0, 0, output_width, output_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glDeleteFramebuffers(2, fboIds);
}

int GetFormatSize(GLenum format)
{
    switch (format)
    {
    case GL_RED:
    case GL_RED_INTEGER:
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_STENCIL:
        return 1;
    case GL_RG:
    case GL_RG_INTEGER:
        return 2;
    case GL_RGB:
    case GL_RGB_INTEGER:
        return 3;
    case GL_RGBA:
    case GL_RGBA_INTEGER:
        return 4;
    default:
        return 0;
    }
}

class GLTexture
{
private:
    GLuint m_texture = 0;
    int m_width = 0;
    int m_height = 0;
    GLint m_format = 0;
    GLuint m_target = 0;
    bool m_own = true;

public:
    GLTexture(int width, int height, GLint format = GL_RGBA, const unsigned char* pixel = nullptr, GLuint target = GL_TEXTURE_2D)
    {
        glGenTextures(1, &m_texture);
        glBindTexture(target, m_texture);
        glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixel);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(target, 0);

        m_width = width;
        m_height = height;
        m_format = format;
        m_target = target;
        m_own = true;
    }

    GLTexture(GLuint texture, int width, int height, GLint format, GLuint target = GL_TEXTURE_2D)
    {
        m_texture = texture;
        m_width = width;
        m_height = height;
        m_format = format;
        m_target = target;
        m_own = false;
    }

    ~GLTexture()
    {
        if (m_own)
        {
            glDeleteTextures(1, &m_texture);
            m_texture = 0;
        }
    }

    GLuint GetTexture() const
    {
        return m_texture;
    }

    GLuint GetTarget() const
    {
        return m_target;
    }

    int GetWidth() const
    {
        return m_width;
    }

    int GetHeight() const
    {
        return m_height;
    }

    int GetFormat() const
    {
        return m_format;
    }

    void ReadPixels(int width, int height, void* buffer)
    {
        GLuint fbo = 0;
        GL_CHECK(glGenFramebuffers(1, &fbo));

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_target, m_texture, 0));

        GL_CHECK(glReadPixels(0, 0, width, height, m_format, GL_UNSIGNED_BYTE, buffer));

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE));

        GL_CHECK(glDeleteFramebuffers(1, &fbo));
    }
};
