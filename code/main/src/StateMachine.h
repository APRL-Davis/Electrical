// #ifndef _StateMachine_h
// #define _StateMachine_h

// class StateMachine{

// public:
//     enum State {DEFAULT, CHECK, ARMED, HOT, MANUAL};
//     enum Command {RESET, PRESSURIZE, FIRE, DEPRESSURIZE, ABORT};    

//     StateMachine(){};

//     void processCommand(Command command);
//     void changeState(State newState); // given state and command, change to appropriate state
//     int getState(); // returns the current state

//     void reset();
//     void pressurize();
//     void fire();
//     void depressurize();
//     void abort();

// private:

//     /*======== Relays ========*/
//     const int _keroIsolation; // isok
//     const int _loxIsolation; // isol
//     const int _keroMain; // main k
//     const int _loxMain; // main l
//     const int _keroVent; // vent k
//     const int _loxVent; // vent l
//     const int _purge; //purge    

// };

// #endif