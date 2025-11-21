
#ifndef HMI_h
#define HMI_h

#include "time.h"
#include <stdbool.h>
#include <stdint.h>

#define REAL 1
#define INT 3
#define BOOL 4
#define STRING 5
#define TIMESTAMP 6


typedef struct {
	float InputValue;
	float Value;
} _Set;

typedef struct {
	float HMIValue;
	float Value;
} _Act;

typedef struct {
	float Min;
	float Max;
} _Limit;

typedef struct {
	_Set Set;
	_Limit Limit;
	int Decimals;
	bool Init;
} Set;

typedef struct {
	_Act Act;
	_Limit Limit;
	int Decimals;
} Act;

typedef struct {
	_Set Set;
	_Act Act;
	_Limit Limit;
	int Decimals;
	bool Init;
} SetAct;

typedef struct {
	int Command;
	int Status;
} LogicSelection;

typedef struct {
	int Status;
} LogicStatus;

typedef struct {
	int Status;
	int Reaction;
	time_t Ts;
	bool Q;
} Alarm;

typedef struct {
	Alarm LowWaterLevel;
	Alarm LowLight;
	Alarm LowBattery;
	Alarm HighMoisture;
	Alarm LowMoisture;
} PotFaults;


typedef struct {
	Act Temperature;
	SetAct Moisture;
	LogicStatus Watering;
	Act WaterLevel;
	SetAct Light;
	Act BatteryLevel;
	PotFaults Faults;
} _HMI;

extern _HMI HMI;
extern _HMI PLC;

extern uint64_t id[52];
extern int type[52];
extern void *HMI_pointer[52];
extern void *PLC_pointer[52];

#endif
  