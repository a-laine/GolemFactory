#include "EventSequence.h"

//  Static attributes
float EventSequence::timeout = 0.4f; //in second
//

//  Default
EventSequence::EventSequence() : Event(Event::SEQUENCE),state(0),lastCheckTime(0){}
EventSequence::~EventSequence(){}
//

//  Public functions
bool EventSequence::isActivated() const {return false;}
bool EventSequence::check(InputType call,int key,int action)
{
    if(action == GLFW_RELEASE) return false;
    float time = glfwGetTime();

    if(call!=inputList[state].first.callback || key!=inputList[state].first.key || (state && time-lastCheckTime>timeout)) //wrong touch or timeout
    {
        state = 0;
        return false;
    }
    else //good touch && good timing
    {
        state++;
        lastCheckTime = time;
        if(state>=inputList.size())
        {
            state = 0;
            return true;
        }
        else return false;
    }
}
//
