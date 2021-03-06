#include <iostream>

#include "RaZ/Utils/Window.hpp"

namespace Raz {

namespace {

void GLAPIENTRY callbackDebugLog(GLenum source,
                                 GLenum type,
                                 unsigned int id,
                                 GLenum severity,
                                 int /* length */,
                                 const char* message,
                                 const void* /* userParam */) {
  std::cerr << "OpenGL Debug - ";

  switch (source) {
    case GL_DEBUG_SOURCE_API: std::cerr << "Source: OpenGL\t"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cerr << "Source: Windows\t"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader compiler\t"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: std::cerr << "Source: Third party\t"; break;
    case GL_DEBUG_SOURCE_APPLICATION: std::cerr << "Source: Application\t"; break;
    case GL_DEBUG_SOURCE_OTHER: std::cerr << "Source: Other\t"; break;
    default: break;
  }

  switch (type) {
    case GL_DEBUG_TYPE_ERROR: std::cerr << "Type: Error\t"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated behavior\t"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cerr << "Type: Undefined behavior\t"; break;
    case GL_DEBUG_TYPE_PORTABILITY: std::cerr << "Type: Portability\t"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: std::cerr << "Type: Performance\t"; break;
    case GL_DEBUG_TYPE_OTHER: std::cerr << "Type: Other\t"; break;
    default: break;
  }

  std::cout << "ID: " << id << "\t";

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: std::cerr << "Severity: High\t"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: std::cerr << "Severity: Medium\t"; break;
    case GL_DEBUG_SEVERITY_LOW: std::cerr << "Severity: Low\t"; break;
    default: break;
  }

  std::cerr << "Message: " << message << std::endl;
}

} // namespace

Window::Window(unsigned int width, unsigned int height, const std::string& title, uint8_t AASampleCount) : m_width{ width },
                                                                                                           m_height{ height } {
  glfwSetErrorCallback([] (int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << std::endl;
  });

  if (!glfwInit())
    throw std::runtime_error("Error: Failed to initialize GLFW");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, AASampleCount);

#if defined(__APPLE__) // Setting the OpenGL forward compatibility is required on macOS
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), nullptr, nullptr);
  if (!m_window) {
    close();
    throw std::runtime_error("Error: Failed to create GLFW Window");
  }

  glfwMakeContextCurrent(m_window);

  glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
    std::cerr << "Error: Failed to initialize GLEW." << std::endl;

#if !defined(__APPLE__) // Setting the debug message callback provokes a crash on macOS
  glDebugMessageCallback(&callbackDebugLog, nullptr);
#endif
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  enableFaceCulling();
  glEnable(GL_DEPTH_TEST);
}

void Window::setIcon(const Image& img) const {
  const GLFWimage icon = { static_cast<int>(img.getWidth()),
                           static_cast<int>(img.getHeight()),
                           const_cast<unsigned char*>(static_cast<const uint8_t*>(img.getDataPtr())) };
  glfwSetWindowIcon(m_window, 1, &icon);
}

void Window::enableFaceCulling(bool value) const {
  if (value)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
}

bool Window::recoverVerticalSyncState() const {
#if defined(_WIN32)
  if (wglGetExtensionsStringEXT())
    return static_cast<bool>(wglGetSwapIntervalEXT());
#elif defined(__gnu_linux__)
  if (glXQueryExtensionsString(glXGetCurrentDisplay(), 0)) {
    unsigned int interval;
    glXQueryDrawable(glXGetCurrentDisplay(), glXGetCurrentDrawable(), GLX_SWAP_INTERVAL_EXT, &interval);

    return static_cast<bool>(interval);
  }
#endif

  std::cerr << "Warning: Vertical synchronisation unsupported." << std::endl;
  return false;
}

void Window::enableVerticalSync(bool value) const {
#if defined(_WIN32)
  if (wglGetExtensionsStringEXT())
    wglSwapIntervalEXT(static_cast<int>(value));
  else
    std::cerr << "Warning: Vertical synchronisation unsupported." << std::endl;
#elif defined(__gnu_linux__)
  if (glXQueryExtensionsString(glXGetCurrentDisplay(), 0)) {
    glXSwapIntervalEXT(glXGetCurrentDisplay(), glXGetCurrentDrawable(), value);
    glXSwapIntervalMESA(static_cast<unsigned int>(value));
  } else {
    std::cerr << "Warning: Vertical synchronisation unsupported." << std::endl;
  }
#endif
}

void Window::addKeyCallback(Keyboard::Key key, std::function<void(float)> actionPress,
                                               Input::ActionTrigger frequency,
                                               std::function<void()> actionRelease) {
  std::get<0>(m_callbacks).emplace_back(key, std::move(actionPress), frequency, std::move(actionRelease));
  glfwSetWindowUserPointer(m_window, &m_callbacks);

  updateCallbacks();
}

void Window::addMouseButtonCallback(Mouse::Button button, std::function<void(float)> actionPress,
                                                          Input::ActionTrigger frequency,
                                                          std::function<void()> actionRelease) {
  std::get<1>(m_callbacks).emplace_back(button, std::move(actionPress), frequency, std::move(actionRelease));
  glfwSetWindowUserPointer(m_window, &m_callbacks);

  updateCallbacks();
}

void Window::addMouseScrollCallback(std::function<void(double, double)> func) {
  std::get<2>(m_callbacks) = std::move(func);
  glfwSetWindowUserPointer(m_window, &m_callbacks);

  updateCallbacks();
}

void Window::addMouseMoveCallback(std::function<void(double, double)> func) {
  std::get<3>(m_callbacks) = std::make_tuple(m_width / 2, m_height / 2, std::move(func));
  glfwSetWindowUserPointer(m_window, &m_callbacks);

  updateCallbacks();
}

void Window::updateCallbacks() const {
  // Keyboard inputs
  if (!std::get<0>(m_callbacks).empty()) {
    glfwSetKeyCallback(m_window, [] (GLFWwindow* window, int key, int /*scancode*/, int action, int /*mode*/) {
      auto& callbacks = *static_cast<InputCallbacks*>(glfwGetWindowUserPointer(window));
      const auto& keyCallbacks = std::get<0>(callbacks);

      for (const auto& callback : keyCallbacks) {
        if (key == std::get<0>(callback)) {
          if (action == GLFW_PRESS) {
            std::get<4>(callbacks).emplace(key, std::make_pair(std::get<1>(callback), std::get<2>(callback)));
          } else if (action == GLFW_RELEASE) {
            std::get<4>(callbacks).erase(key);

            if (std::get<3>(callback))
              std::get<3>(callback)();
          }
        }
      }
    });
  }

  // Mouse buttons inputs
  if (!std::get<1>(m_callbacks).empty()) {
    glfwSetMouseButtonCallback(m_window, [] (GLFWwindow* window, int button, int action, int /* mods */) {
      auto& callbacks = *static_cast<InputCallbacks*>(glfwGetWindowUserPointer(window));
      const auto& mouseCallbacks = std::get<1>(callbacks);

      for (const auto& callback : mouseCallbacks) {
        if (button == std::get<0>(callback)) {
          if (action == GLFW_PRESS) {
            std::get<4>(callbacks).emplace(button, std::make_pair(std::get<1>(callback), std::get<2>(callback)));
          } else if (action == GLFW_RELEASE) {
            std::get<4>(callbacks).erase(button);

            if (std::get<3>(callback))
              std::get<3>(callback)();
          }
        }
      }
    });
  }

  // Mouse scroll input
  if (std::get<2>(m_callbacks)) {
    glfwSetScrollCallback(m_window, [] (GLFWwindow* window, double xOffset, double yOffset) {
      const auto& scrollCallback = std::get<2>(*static_cast<InputCallbacks*>(glfwGetWindowUserPointer(window)));

      scrollCallback(xOffset, yOffset);
    });
  }

  // Mouse move input
  if (std::get<2>(std::get<3>(m_callbacks))) {
    glfwSetCursorPosCallback(m_window, [] (GLFWwindow* window, double xPosition, double yPosition) {
      auto& moveCallback = std::get<3>(*static_cast<InputCallbacks*>(glfwGetWindowUserPointer(window)));

      double& xPrevPos = std::get<0>(moveCallback);
      double& yPrevPos = std::get<1>(moveCallback);

      std::get<2>(moveCallback)(xPosition - xPrevPos, yPosition - yPrevPos);

      xPrevPos = xPosition;
      yPrevPos = yPosition;
    });
  }
}

void Window::addOverlayElement(OverlayElementType type, const std::string& text,
                               std::function<void()> actionOn, std::function<void()> actionOff) {
  m_overlay->addElement(type, text, std::move(actionOn), std::move(actionOff));
}

void Window::addOverlayText(const std::string& text) {
  m_overlay->addText(text);
}

void Window::addOverlayButton(const std::string& text, std::function<void()> action) {
  m_overlay->addButton(text, std::move(action));
}

void Window::addOverlayCheckbox(const std::string& text, bool initVal,
                                std::function<void()> actionOn, std::function<void()> actionOff) {
  m_overlay->addCheckbox(text, initVal, std::move(actionOn), std::move(actionOff));
}

void Window::addOverlaySeparator() {
  m_overlay->addSeparator();
}

void Window::addOverlayFrameTime(const std::string& formattedText) {
  m_overlay->addFrameTime(formattedText);
}

void Window::addOverlayFpsCounter(const std::string& formattedText) {
  m_overlay->addFpsCounter(formattedText);
}

bool Window::run(float deltaTime) {
  if (glfwWindowShouldClose(m_window))
    return false;

  glfwPollEvents();

  // Process actions belonging to pressed keys & mouse buttons
  auto& actions   = std::get<4>(m_callbacks);
  auto actionIter = actions.begin();

  while (actionIter != actions.end()) {
    auto& action = actionIter->second;

    // An action consists of two parts:
    //   - a callback associated to the triggered key or button
    //   - a value indicating if it should be executed only once or every frame

    action.first(deltaTime);

    // Removing the current action if ONCE is given, or simply increment the iterator
    if (action.second == Input::ONCE)
      actionIter = actions.erase(actionIter); // std::unordered_map::erase(iter) returns an iterator on the next element
    else
      ++actionIter;
  }

  if (m_overlay)
    m_overlay->render();

  glfwSwapBuffers(m_window);

  glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return true;
}

Vec2f Window::recoverMousePosition() const {
  double xPos {}, yPos {};
  glfwGetCursorPos(m_window, &xPos, &yPos);

  return Vec2f({ static_cast<float>(xPos), static_cast<float>(yPos) });
}

void Window::close() {
  disableOverlay();
  glfwTerminate();
}

} // namespace Raz
