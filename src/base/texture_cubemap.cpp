#include <cassert>
#include <stb_image.h>

#include "texture_cubemap.h"

TextureCubemap::TextureCubemap(
    GLint internalFormat, int width, int height, GLenum format, GLenum dataType) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, _handle);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (uint32_t i = 0; i < 6; ++i) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format,
            dataType, nullptr);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

TextureCubemap::TextureCubemap(TextureCubemap&& rhs) noexcept : Texture(std::move(rhs)) {}

void TextureCubemap::bind(int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _handle);
}

void TextureCubemap::unbind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCubemap::generateMipmap() const {
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void TextureCubemap::setParamterInt(GLenum name, int value) const {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, name, value);
}

ImageTextureCubemap::ImageTextureCubemap(const std::vector<std::string>& filepaths)
    : _uris(filepaths) {
    assert(filepaths.size() == 6);
    // TODO: load six images and generate the texture cubemap
    // hint: you can refer to Texture2D(const std::string&) for image loading
    // write your code here
    // -----------------------------------------------
    // ...
    // -----------------------------------------------

    for (int i = 0; i < 6; ++i) {
        int width = 0, height = 0, channels = 0;
        unsigned char* data = stbi_load(_uris[i].c_str(), &width, &height, &channels, 0);
        if (data == nullptr) {
            cleanup();
            throw std::runtime_error("load " + filepaths[i] + " failure");
        }

        GLenum format = GL_RGB;
        switch (channels) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            cleanup();
            stbi_image_free(data);
            throw std::runtime_error("unsupported format");
        }
        GLint internalFormat = static_cast<GLint>(format);

        // bind texture
        glBindTexture(GL_TEXTURE_CUBE_MAP, _handle);

        // set texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // transfer data to gpu
        // 1. set the alignment for data transfer
        GLint alignment = 1;
        size_t pitch = width * channels * sizeof(unsigned char);
        if (pitch % 8 == 0)
            alignment = 8;
        else if (pitch % 4 == 0)
            alignment = 4;
        else if (pitch % 2 == 0)
            alignment = 2;
        else
            alignment = 1;

        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

        // 2. transfer data
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);

        // 3. restore the alignment
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        // unbind
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        // free data
        stbi_image_free(data);
    }

    check();
}

ImageTextureCubemap::ImageTextureCubemap(ImageTextureCubemap&& rhs) noexcept
    : TextureCubemap(std::move(rhs)), _uris(std::move(rhs._uris)) {
    rhs._uris.clear();
}

const std::vector<std::string>& ImageTextureCubemap::getUris() const {
    return _uris;
}