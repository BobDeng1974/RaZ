#include "RaZ/Render/Mesh.hpp"

namespace Raz {

Mesh::Mesh(const Vec3f& pos1, const Vec3f& pos2, const Vec3f& pos3) {
  // TODO: Naive normal creation, need to compute them

  Vertex vert1 {};
  vert1.positions = pos1;
  vert1.texcoords = Vec2f({ 0.f, 1.f });
  vert1.normals = Vec3f({ 0.f, 1.f, 0.f });

  Vertex vert2 {};
  vert2.positions = pos2;
  vert2.texcoords = Vec2f({ 0.5f, 0.f });
  vert2.normals = Vec3f({ 0.f, 1.f, 0.f });

  Vertex vert3 {};
  vert3.positions = pos3;
  vert3.texcoords = Vec2f({ 1.f, 1.f });
  vert3.normals = Vec3f({ 0.f, 1.f, 0.f });

  m_vbo.getVertices().resize(3);
  m_vao.getEbo().getIndices().resize(3);

  m_vbo.getVertices()[0] = vert1;
  m_vbo.getVertices()[1] = vert2;
  m_vbo.getVertices()[2] = vert3;

  m_vao.getEbo().getIndices()[0] = 0;
  m_vao.getEbo().getIndices()[1] = 1;
  m_vao.getEbo().getIndices()[2] = 2;

  load();
}

Mesh::Mesh(const Vec3f& posTopLeft, const Vec3f& posTopRight, const Vec3f& posBottomRight, const Vec3f& posBottomLeft) {
  // TODO: Naive normal creation, need to compute them

  Vertex topLeft {};
  topLeft.positions = posTopLeft;
  topLeft.texcoords = Vec2f({ 0.f, 0.f });
  topLeft.normals = Vec3f({ 0.f, 1.f, 0.f });

  Vertex topRight {};
  topRight.positions = posTopRight;
  topRight.texcoords = Vec2f({ 1.f, 0.f });
  topRight.normals = Vec3f({ 0.f, 1.f, 0.f });

  Vertex bottomRight {};
  bottomRight.positions = posBottomRight;
  bottomRight.texcoords = Vec2f({ 1.f, 1.f });
  bottomRight.normals = Vec3f({ 0.f, 1.f, 0.f });

  Vertex bottomLeft {};
  bottomLeft.positions = posBottomLeft;
  bottomLeft.texcoords = Vec2f({ 0.f, 1.f });
  bottomLeft.normals = Vec3f({ 0.f, 1.f, 0.f });

  m_vbo.getVertices().resize(4);
  m_vao.getEbo().getIndices().resize(6);

  m_vbo.getVertices()[0] = topLeft;
  m_vbo.getVertices()[1] = topRight;
  m_vbo.getVertices()[2] = bottomRight;
  m_vbo.getVertices()[3] = bottomLeft;

  m_vao.getEbo().getIndices()[0] = 1;
  m_vao.getEbo().getIndices()[1] = 0;
  m_vao.getEbo().getIndices()[2] = 2;

  m_vao.getEbo().getIndices()[3] = 2;
  m_vao.getEbo().getIndices()[4] = 0;
  m_vao.getEbo().getIndices()[5] = 3;

  load();
}

} // namespace Raz