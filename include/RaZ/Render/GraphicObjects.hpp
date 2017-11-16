#pragma once

#ifndef RAZ_GRAPHICOBJECTS_HPP
#define RAZ_GRAPHICOBJECTS_HPP

#include <vector>

#include "glew/include/GL/glew.h"

namespace Raz {

struct Vertex {
  Vec3f positions;
  Vec2f texcoords;
  Vec3f normals;

  std::size_t operator()(const Vertex&, const Vertex&) { return normals.hash(texcoords.hash(positions.hash(0))); }
};

class ElementBuffer {
public:
  ElementBuffer() { glGenBuffers(1, &m_index); }

  GLuint getIndex() const { return m_index; }
  const std::vector<unsigned int>& getIndices() const { return m_indices; }
  std::vector<unsigned int>& getIndices() { return m_indices; }

  void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index); }
  void unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

  ~ElementBuffer() { glDeleteBuffers(1, &m_index); }

private:
  GLuint m_index;
  std::vector<unsigned int> m_indices;
};

class VertexBuffer {
public:
  VertexBuffer() { glGenBuffers(1, &m_index); }

  GLuint getIndex() const { return m_index; }
  const std::vector<Vertex>& getVertices() const { return m_vertices; }
  std::vector<Vertex>& getVertices() { return m_vertices; }

  void bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_index); }
  void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  ~VertexBuffer() { glDeleteBuffers(1, &m_index); }

private:
  GLuint m_index;
  std::vector<Vertex> m_vertices;
};

class VertexArray {
public:
  VertexArray() { glGenVertexArrays(1, &m_index); }

  GLuint getIndex() const { return m_index; }
  const ElementBuffer& getEbo() const { return m_ebo; }
  ElementBuffer& getEbo() { return m_ebo; }

  void bind() const { glBindVertexArray(m_index); m_ebo.bind(); }
  void unbind() const { glBindVertexArray(0); m_ebo.unbind(); }

  ~VertexArray() { glDeleteVertexArrays(1, &m_index); }

private:
  GLuint m_index;
  ElementBuffer m_ebo;
};

} // namespace Raz

#endif // RAZ_GRAPHICOBJECTS_HPP
