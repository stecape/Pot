
#include "HMI.h"

_HMI HMI = {

	.Temperature = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
	.Moisture = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
	.Light = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
	.BatteryLevel = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
};

_HMI PLC = {

	.Temperature = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
	.Moisture = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
	.Light = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
	.BatteryLevel = {
		.Decimals = 0,
		.Act.HMIValue = 0,
		.Act.Value = 0,
		.Limit.Min = 0,
		.Limit.Max = 0,
	},
};

int id[20] = {
	312,
	337,
	338,
	339,
	340,
	309,
	333,
	334,
	335,
	336,
	318,
	345,
	346,
	347,
	348,
	303,
	325,
	326,
	327,
	328
};

int type[20] = {
	INT,
	REAL,
	REAL,
	REAL,
	REAL,
	INT,
	REAL,
	REAL,
	REAL,
	REAL,
	INT,
	REAL,
	REAL,
	REAL,
	REAL,
	INT,
	REAL,
	REAL,
	REAL,
	REAL
};

void *HMI_pointer[20] = {
	(void*)&HMI.Temperature.Decimals,
	(void*)&HMI.Temperature.Act.HMIValue,
	(void*)&HMI.Temperature.Act.Value,
	(void*)&HMI.Temperature.Limit.Min,
	(void*)&HMI.Temperature.Limit.Max,
	(void*)&HMI.Moisture.Decimals,
	(void*)&HMI.Moisture.Act.HMIValue,
	(void*)&HMI.Moisture.Act.Value,
	(void*)&HMI.Moisture.Limit.Min,
	(void*)&HMI.Moisture.Limit.Max,
	(void*)&HMI.Light.Decimals,
	(void*)&HMI.Light.Act.HMIValue,
	(void*)&HMI.Light.Act.Value,
	(void*)&HMI.Light.Limit.Min,
	(void*)&HMI.Light.Limit.Max,
	(void*)&HMI.BatteryLevel.Decimals,
	(void*)&HMI.BatteryLevel.Act.HMIValue,
	(void*)&HMI.BatteryLevel.Act.Value,
	(void*)&HMI.BatteryLevel.Limit.Min,
	(void*)&HMI.BatteryLevel.Limit.Max
};

void *PLC_pointer[20] = {
	(void*)&PLC.Temperature.Decimals,
	(void*)&PLC.Temperature.Act.HMIValue,
	(void*)&PLC.Temperature.Act.Value,
	(void*)&PLC.Temperature.Limit.Min,
	(void*)&PLC.Temperature.Limit.Max,
	(void*)&PLC.Moisture.Decimals,
	(void*)&PLC.Moisture.Act.HMIValue,
	(void*)&PLC.Moisture.Act.Value,
	(void*)&PLC.Moisture.Limit.Min,
	(void*)&PLC.Moisture.Limit.Max,
	(void*)&PLC.Light.Decimals,
	(void*)&PLC.Light.Act.HMIValue,
	(void*)&PLC.Light.Act.Value,
	(void*)&PLC.Light.Limit.Min,
	(void*)&PLC.Light.Limit.Max,
	(void*)&PLC.BatteryLevel.Decimals,
	(void*)&PLC.BatteryLevel.Act.HMIValue,
	(void*)&PLC.BatteryLevel.Act.Value,
	(void*)&PLC.BatteryLevel.Limit.Min,
	(void*)&PLC.BatteryLevel.Limit.Max
};
  