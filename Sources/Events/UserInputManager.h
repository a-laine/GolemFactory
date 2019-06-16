#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <atomic>

#include <Utiles/Singleton.h>
#include <Utiles/Mutex.h>
#include <Events/InputType.h>


class UserInputListener;


class UserInputManager : public Singleton<UserInputManager>
{
    friend class Singleton<UserInputManager>;

    public:
        void setRepeatMode(bool enable);
        void setTextInput(bool enable);
        void setCursorMode(bool enable);

        bool getRepeatMode();
        bool getTextInput();
        bool getCursorMode();

        void addWindow(GLFWwindow* window);
        void removeWindow(GLFWwindow* window);
        void clearWindows();
        void pollEvents();
        void processInputs(std::vector<UserInputListener*>& listeners);

    private:
        UserInputManager();
        ~UserInputManager() = default;

        // callbacks
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void charCallback(GLFWwindow* window, unsigned int codepoint);
        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void scrollingCallback(GLFWwindow* window, double xoffset, double yoffset);
        // TODO : controller callback
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);


        static UserInputManager* This;

        struct InputEvent
        {
            InputEvent(GameInput i, InputDeviceId d, InputAction a)
                : input(i), device(d), inputType(BUTTON), action(a) {}
            InputEvent(GameInput i, InputDeviceId d, double v)
                : input(i), device(d), inputType(AXIS), value(v) {}

            GameInput input;
            InputDeviceId device;
            enum
            {
                BUTTON,
                AXIS
            } inputType;
            union
            {
                InputAction action;
                double value;
            };
        };

        std::atomic_bool m_repeatKeys;
        std::atomic_bool m_textInput;
        std::atomic_bool m_cursorEnabled;

        Mutex m_mutex;

        std::vector<InputEvent> m_frameInputs;
        std::vector<uint32_t> m_frameChar;
        bool m_viewportChanged;
        glm::ivec2 m_viewportSize;

        std::vector<GLFWwindow*> m_windowList;
};
