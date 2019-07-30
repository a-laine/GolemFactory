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
bool Event::check(InputType call,int key,int action)
{
	//	check if event is activated or not
    bool activated = true;
    for(unsigned int i=0;i<inputList.size();i++)
    {
        if(inputList[i].first.callback == call && inputList[i].first.key == key)
            inputList[i].second = (action != 0);
        activated = (inputList[i].second != 0) && activated;
    }

	//	check for user event publication
    bool upDown = false;
    if(!activated && (configuration&ACTIVATED_FLAG) && (configuration&UP_FLAG))							//	publish user event for up
        upDown = true;
    else if(action == 2 || (activated && (~configuration&ACTIVATED_FLAG) && (configuration&DOWN_FLAG)))	//	publish user event for down
        upDown = true;

	//	change flag if needed
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
