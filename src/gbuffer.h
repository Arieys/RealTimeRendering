#include "base/gl_utility.h"
#include "base/framebuffer.h"

#define GBUFFER_LAYER_SIZE GBuffer::GBUFFER_TEXTURE_TYPE::GBUFFER_NUM_TEXTURES

class GBuffer
{
public:
    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };
    GBuffer() = default;
    ~GBuffer()
    {
        glDeleteFramebuffers(1,&m_fbo);
        glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
        glDeleteTextures(1, &m_depthTexture);
    }
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
    void BindForWriting();
    void BindForReading();
    void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);
private:
    GLuint m_fbo;
    GLuint m_textures[GBUFFER_NUM_TEXTURES];
    GLuint m_depthTexture;
};
