#include "EventSequence.h"

//  Static attributes
float EventSequence::timeout = 0.4f; //in second
//

//  Default
EventSequence::EventSequence() : Event(Event::SEQUENCE), state(0), lastCheckTime(0) {}
EventSequence::~EventSequence() {}
//

//  Public functions
bool EventSequence::isActivated() const { return false; }
bool EventSequence::check(InputType call, int key, int action)
{
    if(action == GLFW_RELEASE) return false;
	float time = (float)glfwGetTime();

	//	reset state if timeout occur
	if(time - lastCheckTime > timeout)
		state = 0;
	lastCheckTime = time;

	if (call != inputList[state].first.callback || key != inputList[state].first.key) //wrong touch
    {
        state = 0;
        return false;
    }
    else //good touch && good timing
    {
        state++;
		if (state < inputList.size()) return false;

		//	end of sequence reached -> need a user event emission
		state = 0;
		return true;
    }
}
//
