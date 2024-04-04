#pragma once

class StateMachine{

public:
    bool endFireFlag = 0;
    bool valveStateChange = 1;

    // relay states/
    bool isok_state = 0;
    bool isol_state = 0;
    bool maink_state = 0;
    bool mainl_state = 0;
    bool ventk_state = 0;
    bool ventl_state = 0;
    bool purge_state = 0;

    enum State {DEFAULT, ARMED, HOT, MANUAL};
    enum Command {ORIGIN = 0, PRESSURIZE = 12, FIRE = 14, DEPRESSURIZE = 15, ABORT = 16, FULL = 17};    

    StateMachine(const int keroIso, const int loxIso, const int keroMain, const int loxMain,
                            const int keroVent, const int loxVent, const int purge);

    State state;

    void initializeMachina();
    void processCommand(int command, long targetTime, long purgeTime, long fireDuration, long purgeDuration); //what to do when receive command
    void changeState(State newState); // given state and command, change to appropriate state STATUS
    int getState(); // returns the current state

    void reset();
    void pressurize();
    void fire();
    void depressurize();
    void abort();
    void endFire();
    void purge();

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
