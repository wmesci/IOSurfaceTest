#pragma once
#include "glhelper.h"

class IRenderer
{
public:
    virtual bool Init() = 0;

    virtual void UnInit() = 0;

    virtual void OnRender() = 0;
};

class ImageRenderer : public IRenderer
{
private:
    GLuint m_vbo = 0;
    GLuint m_vao = 0;
    GLuint m_shaderProgram = 0;
    std::shared_ptr<GLTexture> m_texture;

public:
    bool Init() override
    {
        // Create Vertex Array Object
        {
            GL_CHECK(glGenVertexArrays(1, &m_vao));
        }

        // Create Vertex Buffer
        {
            float vertices[] = {
                -1.0f, 1.0f, 0.0f,   -1.0f, -1.0f, 0.0f,  1.0f, -1.0f, 0.0f,
                -1.0f, 1.0f, 0.0f,    1.0f, -1.0f, 0.0f,  1.0f,  1.0f, 0.0f,
            };

            glGenBuffers(1, &m_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // Create Shader Program
        {
            const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            out vec2 fragTexCoord;
            void main()
            {
                gl_Position = vec4(position, 0.0, 1.0);
                fragTexCoord = position.xy * 0.5 + 0.5;
                fragTexCoord.y = 1.0 - fragTexCoord.y;
            }
            )";

            const char* fragmentShaderSource = R"(
            #version 330 core
            #ifdef GL_ES
            #ifdef GL_FRAGMENT_PRECISION_HIGH
            precision highp float;
            #else
            precision mediump float;
            #endif
            #endif
            in vec2 fragTexCoord;
            layout (location = 0) out vec4 fragColor;
            uniform sampler2DRect inputTexture;
            void main()
            {
                fragColor = texture(inputTexture, gl_FragCoord.xy);
            }
            )";

            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
            glCompileShader(vertexShader);

            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
            glCompileShader(fragmentShader);

            m_shaderProgram = glCreateProgram();
            glAttachShader(m_shaderProgram, vertexShader);
            glAttachShader(m_shaderProgram, fragmentShader);
            glLinkProgram(m_shaderProgram);

            GLint linkStatus;
            glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linkStatus);
            if (linkStatus == GL_FALSE)
            {
                GLint logLength;
                glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
                GLchar* log = new GLchar[logLength];
                glGetProgramInfoLog(m_shaderProgram, logLength, NULL, log);
                printf("Shader program link error: %s", log);
                delete[] log;
            }

            glBindAttribLocation(m_shaderProgram, 0, "position");

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }
    
        return true;
    }

    void UnInit() override
    {
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_vao != 0)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_shaderProgram != 0)
        {
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
        }
    }

    const std::shared_ptr<GLTexture>& GetTexture() const
    {
        return m_texture;
    }

    void SetTexture(const std::shared_ptr<GLTexture>& texture)
    {
        m_texture = texture;
    }

    void OnRender() override
    {
        GL_CHECK(glUseProgram(m_shaderProgram));

        GL_CHECK(glUniform1i(glGetUniformLocation(m_shaderProgram, "inputTexture"), 0));

        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
        GL_CHECK(glBindTexture(GL_TEXTURE_RECTANGLE, 0));
        if (m_texture != nullptr)
        {
            GL_CHECK(glBindTexture(m_texture->GetTarget(), m_texture->GetTexture()));
        }

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

        GL_CHECK(glBindVertexArray(m_vao));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
    }
};

class TestRenderer : public IRenderer
{
private:
    std::chrono::high_resolution_clock::time_point m_time;
    GLuint m_vbo = 0;
    GLuint m_vao = 0;
    GLuint m_shaderProgram = 0;

public:
    bool Init() override
    {
        m_time = std::chrono::high_resolution_clock::now();

        // Create Vertex Array Object
        {
            GL_CHECK(glGenVertexArrays(1, &m_vao));
        }

        // Create Vertex Buffer
        {
            float vertices[] = {
                -1.0f, 1.0f, 0.0f,   -1.0f, -1.0f, 0.0f,  1.0f, -1.0f, 0.0f,
                -1.0f, 1.0f, 0.0f,    1.0f, -1.0f, 0.0f,  1.0f,  1.0f, 0.0f,
            };

            glGenBuffers(1, &m_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // Create Shader Program
        {
            const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            out vec2 fragTexCoord;
            void main()
            {
                gl_Position = vec4(position, 0.0, 1.0);
                fragTexCoord = position.xy * 0.5 + 0.5;
                fragTexCoord.y = 1.0 - fragTexCoord.y;
            }
            )";

            const char* fragmentShaderSource = R"(
            #version 330 core
            #ifdef GL_ES
            #ifdef GL_FRAGMENT_PRECISION_HIGH
            precision highp float;
            #else
            precision mediump float;
            #endif
            #endif
            in vec2 fragTexCoord;
            layout (location = 0) out vec4 fragColor;
            uniform float t;
            void main()
            {
                fragColor = vec4(mix(fragTexCoord, 1.0f - fragTexCoord, t), 0.0, 1.0);
            }
            )";

            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
            glCompileShader(vertexShader);

            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
            glCompileShader(fragmentShader);

            m_shaderProgram = glCreateProgram();
            glAttachShader(m_shaderProgram, vertexShader);
            glAttachShader(m_shaderProgram, fragmentShader);
            glLinkProgram(m_shaderProgram);

            GLint linkStatus;
            glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linkStatus);
            if (linkStatus == GL_FALSE)
            {
                GLint logLength;
                glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
                GLchar* log = new GLchar[logLength];
                glGetProgramInfoLog(m_shaderProgram, logLength, NULL, log);
                printf("Shader program link error: %s", log);
                delete[] log;
            }

            glBindAttribLocation(m_shaderProgram, 0, "position");

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }
    
        return true;
    }

    void UnInit() override
    {
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_vao != 0)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_shaderProgram != 0)
        {
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
        }
    }

    void OnRender() override
    {
        GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        GL_CHECK(glUseProgram(m_shaderProgram));
        
        auto duration = std::chrono::high_resolution_clock::now() - m_time;
        auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0f;

        float t = (sin(sec * 3.1415926f) + 1.0f) / 2.0f;

        GL_CHECK(glUniform1f(glGetUniformLocation(m_shaderProgram, "t"), t));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

        GL_CHECK(glBindVertexArray(m_vao));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
    }
};