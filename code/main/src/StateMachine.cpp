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

void StateMachine::initializeMachina()
{
    pinMode(_keroIsolation,OUTPUT);
    pinMode(_loxIsolation,OUTPUT);
    pinMode(_keroMain,OUTPUT);
    pinMode(_loxMain,OUTPUT);
    pinMode(_keroVent,OUTPUT);
    pinMode(_loxVent,OUTPUT);
    pinMode(_purge,OUTPUT);    
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

    isok_state = 0;
    isol_state = 0;
    maink_state = 0;
    mainl_state = 0;
    ventk_state = 0;
    ventl_state = 0;
    purge_state = 0;
}

void StateMachine::pressurize()
{
    digitalWrite(_keroIsolation, HIGH);
    digitalWrite(_loxIsolation, HIGH);
    digitalWrite(_keroVent, HIGH);
    digitalWrite(_loxVent, HIGH);

    isok_state = 1;
    isol_state = 1;
    ventk_state = 1;
    ventl_state = 1;
}

void StateMachine::depressurize()
{
    digitalWrite(_keroIsolation, LOW);
    digitalWrite(_loxIsolation, LOW);
    digitalWrite(_keroVent, LOW);
    digitalWrite(_loxVent, LOW);

    isok_state = 0;
    isol_state = 0;
    ventk_state = 0;
    ventl_state = 0;
}

void StateMachine::fire()
{
    digitalWrite(_keroMain, HIGH);
    digitalWrite(_loxMain, HIGH);

    maink_state = 1;
    mainl_state = 1;
}

void StateMachine::endFire()
{
    digitalWrite(_keroMain, LOW);
    digitalWrite(_loxMain, LOW);

    maink_state = 0;
    mainl_state = 0;
}

void StateMachine::purge()
{
    digitalWrite(_purge, HIGH);

    purge_state = 1;
}

void StateMachine::abort()
{
    digitalWrite(_keroMain, LOW);
    digitalWrite(_loxMain, LOW);
    digitalWrite(_keroIsolation, LOW);
    digitalWrite(_loxIsolation, LOW);
    digitalWrite(_keroVent, LOW);
    digitalWrite(_loxVent, LOW);

    isok_state = 0;
    isol_state = 0;
    maink_state = 0;
    mainl_state = 0;
    ventk_state = 0;
    ventl_state = 0;
}

void StateMachine::processCommand(int command, long targetTime, long purgeTime, long fireDuration, long purgeDuration)
{
    switch(state)
    {
        case DEFAULT:
            if(command == PRESSURIZE)
            {
                changeState(ARMED);   
                pressurize();
                valveStateChange = 1;
            }
            else if (command == FULL)
            {
                changeState(MANUAL);
            }
            break;

        case ARMED:
            if(command == FIRE)
            {
                changeState(HOT);
                fire();
                fireDuration = 0;
                valveStateChange = 1;
            }
            if(command == DEPRESSURIZE)
            {
                changeState(DEFAULT);
                depressurize();
                valveStateChange = 1;
            }
            else if (command == FULL)
            {
                changeState(MANUAL);
            }
            break;

        case HOT:
            if(command == ABORT)
            {
                abort();
                purge();
                purgeDuration = 0;
                endFireFlag = 1;
                valveStateChange = 1;
            }
            else if (fireDuration >= targetTime && endFireFlag == 0)
            {
                endFire();
                purge();
                purgeDuration = 0;
                endFireFlag = 1;
                valveStateChange = 1;
            }
            else if (purgeDuration >= 3000 && endFireFlag)
            {
                reset();
                changeState(DEFAULT);
                endFireFlag = 0;
                valveStateChange = 1;
            }
            else if (command == FULL)
            {
                changeState(MANUAL);
            }          
            break;

        case MANUAL:
            if(command == 1) // relay 1 on
            {
                isok_state = !isok_state;
                digitalWrite(_keroIsolation, isok_state);
                valveStateChange = 1;
            }
            else if(command == 2) // relay 2 on
            {
                isol_state = !isol_state;
                digitalWrite(_loxIsolation, isol_state);
                valveStateChange = 1;
            }
            else if(command == 3) // relay 3 on
            {
                maink_state = !maink_state;
                digitalWrite(_keroMain, maink_state);
                valveStateChange = 1;
            }  
            else if(command == 4) // relay 4 on
            {
                mainl_state = !mainl_state;
                digitalWrite(_loxMain, mainl_state);
                valveStateChange = 1;
            }  
            else if(command == 5) // relay 5 on
            {
                ventk_state = !ventk_state;
                digitalWrite(_keroVent, ventk_state);
                valveStateChange = 1;
            }
            else if(command == 6) // relay 6 on
            {
                ventl_state = !ventl_state;
                digitalWrite(_loxVent, ventl_state);
                valveStateChange = 1;
            }
            else if(command == 7) 
            {
                purge_state = !purge_state;
                digitalWrite(_purge, purge_state);
                valveStateChange = 1;
            }
            else if(command == ORIGIN)
            {
                reset();
                changeState(DEFAULT);
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

    