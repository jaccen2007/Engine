#include "Engine.h"
#include "Logger.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#ifdef EMSCRIPTEN
  #include <emscripten.h>

  static Engine *instance = NULL;
#endif


Engine::Engine(Game *game)
{
  log_info("Initializing SDL");
  window = new Window();

  log_info("Initializing GLEW");
  glew_manager = new GLEWManager();

  log_info("Initializing GL");
  gl_manager = new GLManager(window->getWidth(), window->getHeight());

  this->game = game;

  quit = false;
}

Engine::~Engine(void)
{
  delete window;
  delete glew_manager;
  delete gl_manager;
}

void Engine::start(void)
{
  game->setEngine(this);

  log_info("Initializing game");
  game->init(gl_manager);

#ifdef EMSCRIPTEN
  instance = this;

  emscripten_set_main_loop(Engine::loop, 0, 1);
#else
  while (!quit) {
    tick();

    SDL_Delay(1);
  }
#endif
}

#ifdef EMSCRIPTEN
void Engine::loop(void)
{
  instance->tick();
}
#endif

void Engine::tick(void)
{
  window->tick();
  Uint32 delta_time = window->getDeltaTime();

  quit = window->shouldQuit();

  game->updateInput(window->getInput(), delta_time);

  game->update(delta_time);

  game->render(gl_manager);

  if (window->getInput()->mouseIsPressed(SDL_BUTTON_LEFT)) {
    glm::vec3 v1, v2;

    projectLine(&v1, &v2, window->getInput()->getMousePosition(), gl_manager->getViewMatrix(), gl_manager->getProjectionMatrix());

    log_info("line from: %s to %s", glm::to_string(v1).c_str(), glm::to_string(v2).c_str());
    gl_manager->drawLine(v1, glm::vec3(0,0,0));
    // gl_manager->drawLine(glm::vec3(10,10,10), glm::vec3(0,0,0));
  }

  window->swapBuffer();
}

Window *Engine::getWindow(void)
{
  return window;
}

void Engine::projectLine(glm::vec3* v1, glm::vec3* v2, glm::vec2 mousePosition, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
  glm::vec4 viewport = glm::vec4(0.0f, 0.0f, 1679.000000, 1049.000000);

  float xRel = mousePosition.x / 1679.000000;
  float yRel = mousePosition.y / 1049.000000;

  log_info("x: %f y %f w %i", mousePosition.x, mousePosition.y, window->getWidth());

  *v1 = glm::unProject(glm::vec3(xRel, yRel, 0.0f), viewMatrix, projectionMatrix, viewport);
  *v2 = glm::unProject(glm::vec3(xRel, yRel, 1.0f), viewMatrix, projectionMatrix, viewport);
}
