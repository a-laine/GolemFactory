#include "UserInputManager.h"

#include <Utiles/Assert.hpp>

#include "Events/UserInputListener.h"


UserInputManager* UserInputManager::This = nullptr;


UserInputManager::UserInputManager()
    : m_repeatKeys(false)
    , m_textInput(false)
    , m_cursorEnabled(true)
    , m_viewportChanged(false)
    , m_viewportSize(0.f, 0.f)
{
    This = this;
}

void UserInputManager::setRepeatMode(bool enable)
{
    m_repeatKeys = enable;
}

void UserInputManager::setTextInput(bool enable)
{
    m_textInput = enable;
}

void UserInputManager::setCursorMode(bool enable)
{
    m_mutex.lock();
    int cursor = enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
    for(GLFWwindow* window : m_windowList)
    {
        glfwSetInputMode(window, GLFW_CURSOR, cursor);
    }
    m_cursorEnabled = enable;
    m_mutex.unlock();
}

bool UserInputManager::getRepeatMode()
{
    return m_repeatKeys;
}

bool UserInputManager::getTextInput()
{
    return m_textInput;
}

bool UserInputManager::getCursorMode()
{
    return m_cursorEnabled;
}

void UserInputManager::addWindow(GLFWwindow* window)
{
    GF_ASSERT(window);

    //	add window to the window list and set the window cursor
    m_mutex.lock();
    m_windowList.push_back(window);
    int cursor = m_cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
    glfwSetInputMode(window, GLFW_CURSOR, cursor);
    m_mutex.unlock();

    //	attach callback to the window
    glfwSetKeyCallback(window, UserInputManager::keyCallback);
    glfwSetCharCallback(window, UserInputManager::charCallback);
    glfwSetCursorPosCallback(window, UserInputManager::cursorPositionCallback);
    glfwSetMouseButtonCallback(window, UserInputManager::mouseButtonCallback);
    glfwSetScrollCallback(window, UserInputManager::scrollingCallback);
    glfwSetFramebufferSizeCallback(window, UserInputManager::framebufferResizeCallback);
}

void UserInputManager::removeWindow(GLFWwindow* window)
{
    GF_ASSERT(window);

    //	remove windo from list
    m_mutex.lock();
    auto it = std::find(m_windowList.begin(), m_windowList.end(), window);
    if(it == m_windowList.end())
    {
        GF_ASSERT(0, "trying to remove an unknown window");
        m_mutex.unlock();
        return;
    }
    m_windowList.erase(it);
    m_mutex.unlock();

    //	detach window callback
    glfwSetKeyCallback(window, NULL);
    glfwSetCharCallback(window, NULL);
    glfwSetCursorPosCallback(window, NULL);
    glfwSetCursorEnterCallback(window, NULL);
    glfwSetMouseButtonCallback(window, NULL);
    glfwSetScrollCallback(window, NULL);
    glfwSetDropCallback(window, NULL);
    glfwSetWindowFocusCallback(window, NULL);
}

void UserInputManager::clearWindows()
{
    m_mutex.lock();

    for(GLFWwindow* window : m_windowList)
    {
        glfwSetKeyCallback(window, NULL);
        glfwSetCharCallback(window, NULL);
        glfwSetCursorPosCallback(window, NULL);
        glfwSetCursorEnterCallback(window, NULL);
        glfwSetMouseButtonCallback(window, NULL);
        glfwSetScrollCallback(window, NULL);
        glfwSetDropCallback(window, NULL);
        glfwSetWindowFocusCallback(window, NULL);
    }

    m_windowList.clear();

    m_mutex.unlock();
}

void UserInputManager::pollEvents()
{
    m_mutex.lock();
    glfwPollEvents();
    // TODO : controller events
    m_mutex.unlock();
}

void UserInputManager::processInputs(std::vector<UserInputListener*>& listeners)
{
    std::vector<InputEvent> frameInputs;
    std::vector<uint32_t> frameChar;
    bool viewportChanged;
    glm::ivec2 viewportSize;

    m_mutex.lock();
    frameInputs = m_frameInputs;
    m_frameInputs.clear();
    frameChar = m_frameChar;
    m_frameChar.clear();
    viewportChanged = m_viewportChanged;
    m_viewportChanged = false;
    viewportSize = m_viewportSize;
    m_mutex.unlock();

    if(viewportChanged)
    {
        for(UserInputListener* listener : listeners)
        {
            listener->onViewportResize(viewportSize.x, viewportSize.y);
        }
    }

    for(const InputEvent& event : frameInputs)
    {
        switch(event.inputType)
        {
            case InputEvent::BUTTON:
                for(UserInputListener* listener : listeners)
                {
                    listener->onButtonInput(event.device, event.input, event.action);
                }
                break;
            case InputEvent::AXIS:
                for(UserInputListener* listener : listeners)
                {
                    listener->onAxisInput(event.device, event.input, event.action);
                }
                break;
            default:
                GF_ASSERT(0);
        }
    }

    for(uint32_t codepoint : frameChar)
    {
        for(UserInputListener* listener : listeners)
        {
            listener->onCharacterInput(codepoint);
        }
    }
}

void UserInputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(!This->m_repeatKeys && action == GLFW_REPEAT)
        return;

    This->m_frameInputs.push_back(InputEvent(glfwKeyToGfInput(key), DEVICE_ID_KEYBOARD, glfwActionToGfAction(action)));
}

void UserInputManager::charCallback(GLFWwindow* window, unsigned int codepoint)
{
    if(!This->m_textInput)
        return;

    This->m_frameChar.push_back(codepoint);
}

void UserInputManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    // TODO : change coord system (window / viewport / normalized)

    This->m_frameInputs.push_back(InputEvent(MOUSE_AXIS_X, DEVICE_ID_MOUSE, xpos));
    This->m_frameInputs.push_back(InputEvent(MOUSE_AXIS_Y, DEVICE_ID_MOUSE, ypos));
}

void UserInputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    This->m_frameInputs.push_back(InputEvent(glfwMouseBtnToGfButton(button), DEVICE_ID_MOUSE, glfwActionToGfAction(action)));
}

void UserInputManager::scrollingCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    This->m_frameInputs.push_back(InputEvent(MOUSE_SCROLL_X, DEVICE_ID_MOUSE, xoffset));
    This->m_frameInputs.push_back(InputEvent(MOUSE_SCROLL_Y, DEVICE_ID_MOUSE, yoffset));
}

void UserInputManager::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    This->m_viewportChanged = true;
    This->m_viewportSize = glm::ivec2(width, height);
}
