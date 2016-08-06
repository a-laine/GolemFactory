#include "Event.h"

//  Default
Event::Event(uint8_t config) : configuration(config) {}
Event::~Event() {}
//

//  Public functions
bool Event::isActivated() const
{
	return (configuration&ACTIVATED_FLAG) != 0;
}
bool Event::check(Input in,int action)
{
	return check(in.callback,in.key,action);
}
bool Event::check(InputType call,int key,int action)
{
    bool activated = true;
    for(unsigned int i=0;i<inputList.size();i++)
    {
        if(inputList[i].first.callback == call && inputList[i].first.key == key)
            inputList[i].second = (action != 0);
        activated = (inputList[i].second != 0) && activated;
    }

    bool upDown = false;
    if(!activated && (configuration&ACTIVATED_FLAG) && (configuration&UP_FLAG))
        upDown = true;//send up
    else if(action == 2 || (activated && (~configuration&ACTIVATED_FLAG) && (configuration&DOWN_FLAG)))
        upDown = true;//send down

    if(activated) configuration |=  ACTIVATED_FLAG;
    else configuration &= ~ACTIVATED_FLAG;

    return upDown;
}
void Event::addInput(Input in)
{
    inputList.insert(inputList.end(),std::pair<Input,bool>(in,false));
    inputList.shrink_to_fit();
}
void Event::addInput(InputType call,int key)
{
    Input in;
        in.callback = call; in.key = key;
    addInput(in);
}
//
