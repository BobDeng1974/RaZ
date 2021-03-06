#include <iostream>

#include "RaZ/Render/Framebuffer.hpp"

namespace Raz {

Framebuffer::Framebuffer(unsigned int width, unsigned int height, const std::string& fragShaderPath) {
  const std::string vertSource = R"(
    #version 330 core

    layout (location = 0) in vec2 vertPosition;
    layout (location = 1) in vec2 vertTexcoords;

    out vec2 fragTexcoords;

    void main() {
      fragTexcoords = vertTexcoords;

      gl_Position = vec4(vertPosition, 0.0, 1.0);
    }
  )";

  m_program.setShaders(VertexShader::loadFromSource(vertSource), FragmentShader::create(fragShaderPath));

  m_depthBuffer  = Texture::create(width, height, ImageColorspace::DEPTH);
  m_colorBuffer  = Texture::create(width, height, ImageColorspace::RGBA);
  m_normalBuffer = Texture::create(width, height, ImageColorspace::RGB);
  glGenFramebuffers(1, &m_index);

  const std::array<GLenum, 2> colorBuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer->getIndex(), 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, colorBuffers[0], GL_TEXTURE_2D, m_colorBuffer->getIndex(), 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, colorBuffers[1], GL_TEXTURE_2D, m_normalBuffer->getIndex(), 0);

  glDrawBuffers(2, colorBuffers.data());

  initBuffers();

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cerr << "Error: Framebuffer is not complete." << std::endl;
  unbind();
}

void Framebuffer::initBuffers() const {
  m_program.sendUniform("uniSceneBuffers.depth",  0);
  m_program.sendUniform("uniSceneBuffers.color",  1);
  m_program.sendUniform("uniSceneBuffers.normal", 2);
}

void Framebuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_index);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::display() const {
  glClear(GL_COLOR_BUFFER_BIT);

  Texture::activate(0);
  m_depthBuffer->bind();

  Texture::activate(1);
  m_colorBuffer->bind();

  Texture::activate(2);
  m_normalBuffer->bind();

  m_program.use();
  Mesh::drawUnitQuad();
}

} // namespace Raz
