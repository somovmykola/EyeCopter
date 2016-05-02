#ifndef AUTONOMOUS_FLIGHT_FINAL_H
#define AUTONOMOUS_FLIGHT_FINAL_H

// -----
// DEFINE MACROS
// -----

#define SAMPLE_RATE 5   // microseconds, used for PIGPIO timing

#define QC_MASS 1.1     // Quad-copter mass in kg
#define G       9.8     // Gravity constant



// ------
// MISC STRUCTS AND ENUMS
// ------

// Channel used for writing to I2C
enum port_t {
    pitch = 0,
    yaw,
    roll,
    throttle
};

// 3D vector type
typedef struct {
    double x, y, z;
} vec3;

// Pitch, Yaw, Roll angles
typedef struct {
    double p, y, r;
} ang3;

// PID controller constants
typedef struct {
    double p, i, d;
} PID;

// Controller state
// TODO: give these meaningful names
enum ctrl_state_t {
    Hover = 0,
    MoveRight,
    Land,
    Turn,
    Off,
};

// -----
// PID CONSTANTS
// -----

// TODO: give these meangingful names
const PID K = {0.01, 0.0, 0.5};
const PID PK = {0.001, 0.0, 0.0015};
const PID VXK = {0.2, 0.0, 0.35};
const PID VZK = {0.2, 0.0, 0.35};

// -----
// VARIABLES
// -----

double currTime, lastTime;
double neutralThrottle;

double lastErrY;
double errAccY;

double lastErrF;
double errAccF;

double lastErrVX;
double errAccVX;

double lastErrVZ;
double errAccVZ;

double goalYaw;     //  Never init'd / referenced
double errYaw;      //  Never init'd / referenced
double lastErrYaw;  //  Never init'd / referenced
double errAccYaw;   //  Never init'd / referenced

double lastYaw;
double lastDist;

vec3 currPos;
vec3 currVel;

ang3 currAng;
ang3 lastAng;
ang3 setAng;
double setThrottle;

double lastTime;



// Mutexes
pthread_mutex_t lockTrig;
pthread_mutex_t lockP, lockY, lockR, lockT;
pthread_mutex_t lockUSF, lockUSD, lockOpt;

// Optical sensor object
ADNS_Sensor sensor;

// Control state
enum ctrl_state_t state, lastState;

#endif