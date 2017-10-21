#pragma once

#ifndef RAZ_TEXTURE_HPP
#define RAZ_TEXTURE_HPP

#include <string>

#include "GL/glew.h"
#include "RaZ/Utils/Image.hpp"

namespace Raz {

class Texture {
public:
  Texture() { glGenTextures(1, &m_index); }

  GLuint getIndex() const { return m_index; }

  void load(const std::string& fileName) { m_image.read(fileName); }
  void bind() const { glBindTexture(GL_TEXTURE_2D, m_index); }

  ~Texture() { glDeleteTextures(1, &m_index); }

private:
  GLuint m_index;
  Image m_image;
};

} // namespace Raz

#endif // RAZ_TEXTURE_HPP
