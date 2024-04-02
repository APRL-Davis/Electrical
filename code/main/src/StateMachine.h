#pragma once

class StateMachine{

public:
    enum State {DEFAULT, CHECK, ARMED, HOT, MANUAL};
    enum Command {RESET, PRESSURIZE, FIRE, DEPRESSURIZE, ABORT};    

    long targetTime = 10000;

    StateMachine(const int keroIso, const int loxIso, const int keroMain, const int loxMain,
                            const int keroVent, const int loxVent, const int purge);

    State state;

    void processCommand(Command command); //what to do when receive command
    void changeState(State newState); // given state and command, change to appropriate state STATUS
    int getState(); // returns the current state

    void reset();
    void pressurize();
    void fire();
    void depressurize();
    void abort();

private:

    /*======== Relays ========*/
    int _keroIsolation; // isok
    int _loxIsolation; // isol
    int _keroMain; // main k
    int _loxMain; // main l
    int _keroVent; // vent k
    int _loxVent; // vent l
    int _purge; //purge    

};
