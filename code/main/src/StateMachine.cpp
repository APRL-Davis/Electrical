#include "StateMachine.h"
#include "Arduino.h" 

StateMachine::StateMachine(const int keroIso, const int loxIso, const int keroMain, const int loxMain,
                            const int keroVent, const int loxVent, const int purge)
{
    state = DEFAULT;
    _keroIsolation = keroIso;
    _loxIsolation = loxIso;
    _keroMain = keroMain;
    _loxMain = loxMain;
    _keroVent = keroVent;
    _loxVent = loxVent;
    _purge = purge;
}

void StateMachine::changeState(State newState)
{
    state = newState;
}

void StateMachine::reset()
{
    digitalWrite(_keroIsolation, LOW);
    digitalWrite(_loxIsolation, LOW);
    digitalWrite(_keroMain, LOW);
    digitalWrite(_loxMain, LOW);
    digitalWrite(_keroVent, LOW);
    digitalWrite(_loxVent, LOW);
    digitalWrite(_purge, LOW);
}

void StateMachine::pressurize()
{
    digitalWrite(_keroIsolation, HIGH);
    digitalWrite(_loxIsolation, HIGH);
    digitalWrite(_keroVent, HIGH);
    digitalWrite(_loxVent, HIGH);
}

void StateMachine::depressurize()
{
    digitalWrite(_keroIsolation, LOW);
    digitalWrite(_loxIsolation, LOW);
    digitalWrite(_keroVent, LOW);
    digitalWrite(_loxVent, LOW);
}

void StateMachine::fire()
{
    digitalWrite(_keroMain, HIGH);
    digitalWrite(_loxMain, HIGH);
}

void StateMachine::abort()
{
    digitalWrite(_keroMain, LOW);
    digitalWrite(_loxMain, LOW);
    digitalWrite(_keroIsolation, LOW);
    digitalWrite(_loxIsolation, LOW);
    digitalWrite(_keroVent, LOW);
    digitalWrite(_loxVent, LOW);
}

void StateMachine::processCommand(Command command)
{
    switch(state)
    {
        case DEFAULT:
            if(command == PRESSURIZE)
            {
                changeState(ARMED);   
                pressurize();
            }
            break;
        case ARMED:
            if(command == FIRE)
            {
                changeState(HOT);
                fire();
            }
        case HOT:
            if(command == ABORT)
            {
                changeState(DEFAULT);
                abort();
            }

    }
}

int StateMachine::getState()
{
    // default = 0
    // check = 1
    // armed = 2
    // hot = 3
    // manual = 4
    return state;
}

    