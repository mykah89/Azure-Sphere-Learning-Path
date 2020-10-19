﻿/*
 *   Please read the disclaimer and the developer board selection section below
 *
 *
 *   DISCLAIMER
 *
 *   The learning_path_libs functions provided in the learning_path_libs folder:
 *
 *	   1. are NOT supported Azure Sphere APIs.
 *	   2. are prefixed with lp_, typedefs are prefixed with LP_
 *	   3. are built from the Azure Sphere SDK Samples at https://github.com/Azure/azure-sphere-samples
 *	   4. are not intended as a substitute for understanding the Azure Sphere SDK Samples.
 *	   5. aim to follow best practices as demonstrated by the Azure Sphere SDK Samples.
 *	   6. are provided as is and as a convenience to aid the Azure Sphere Developer Learning experience.
 *
 *
 *   DEVELOPER BOARD SELECTION
 *
 *   The following developer boards are supported.
 *
 *	   1. AVNET Azure Sphere Starter Kit.
 *	   2. Seeed Studio Azure Sphere MT3620 Development Kit aka Reference Design Board or rdb.
 *	   3. Seeed Studio Seeed Studio MT3620 Mini Dev Board.
 *
 *   ENABLE YOUR DEVELOPER BOARD
 *
 *   Each Azure Sphere developer board manufacturer maps pins differently. You need to select the configuration that matches your board.
 *
 *   Follow these steps:
 *
 *	   1. Open CMakeLists.txt.
 *	   2. Uncomment the set command that matches your developer board.
 *	   3. Click File, then Save to save the CMakeLists.txt file which will auto generate the CMake Cache.
 */

 // Hardware definition
#include "hw/azure_sphere_learning_path.h"

// Learning Path Libraries
#include "azure_iot.h"
#include "config.h"
#include "exit_codes.h"
#include "inter_core.h"
#include "peripheral_gpio.h"
#include "terminate.h"
#include "timer.h"

// System Libraries
#include "applibs_versions.h"
#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/powermanagement.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Hardware specific
#ifdef OEM_AVNET
#include "board.h"
#include "imu_temp_pressure.h"
#include "light_sensor.h"
#endif // OEM_AVNET

// Hardware specific
#ifdef OEM_SEEED_STUDIO
#include "SEEED_STUDIO/board.h"
#endif // SEEED_STUDIO

#define LP_LOGGING_ENABLED FALSE
#define JSON_MESSAGE_BYTES 256  // Number of bytes to allocate for the JSON telemetry message for IoT Central

// Forward signatures
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer);
static void AzureIoTConnectionStatusHandler(EventLoopTimer* eventLoopTimer);
static void InterCoreHandler(LP_INTER_CORE_BLOCK* ic_message_block);

LP_USER_CONFIG lp_config;

static char msgBuffer[JSON_MESSAGE_BYTES] = { 0 };
LP_INTER_CORE_BLOCK ic_control_block;

// Declare GPIO
static LP_GPIO azureIotConnectedLed = {
	.pin = NETWORK_CONNECTED_LED,
	.direction = LP_OUTPUT,
	.initialState = GPIO_Value_Low,
	.invertPin = true,
	.name = "azureIotConnectedLed" };

// Timers
static LP_TIMER azureIotConnectionStatusTimer = {
	.period = { 5, 0 },
	.name = "azureIotConnectionStatusTimer",
	.handler = AzureIoTConnectionStatusHandler };

static LP_TIMER measureSensorTimer = {
	.period = { 6, 0 },
	.name = "measureSensorTimer",
	.handler = MeasureSensorHandler };

// Initialize Sets
LP_TIMER* timerSet[] = { &azureIotConnectionStatusTimer, &measureSensorTimer };
LP_GPIO* gpioSet[] = { &azureIotConnectedLed };
LP_DEVICE_TWIN_BINDING* deviceTwinBindingSet[] = { };

// Azure IoT Direct Methods
LP_DIRECT_METHOD_BINDING* directMethodBindingSet[] = { };

// Telemetry message template and properties
static const char* msgTemplate = "{ \"Temperature\": \"%3.2f\", \"Pressure\":\"%3.1f\", \"MsgId\":%d }";

static LP_MESSAGE_PROPERTY* appProperties[] = {
	&(LP_MESSAGE_PROPERTY) { .key = "appid", .value = "lab-monitor" },
	&(LP_MESSAGE_PROPERTY) {.key = "format", .value = "json" },
	&(LP_MESSAGE_PROPERTY) {.key = "type", .value = "telemetry" },
	&(LP_MESSAGE_PROPERTY) {.key = "version", .value = "1" }
};

/// <summary>
/// Check status of connection to Azure IoT
/// </summary>
static void AzureIoTConnectionStatusHandler(EventLoopTimer* eventLoopTimer)
{
	static bool toggleConnectionStatusLed = true;

	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
		return;
	}

	if (lp_azureConnect()) {
		lp_gpioStateSet(&azureIotConnectedLed, toggleConnectionStatusLed);
		toggleConnectionStatusLed = !toggleConnectionStatusLed;
	}
	else {
		lp_gpioStateSet(&azureIotConnectedLed, false);
	}
}

/// <summary>
/// Read sensor and send to Azure IoT
/// </summary>
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer)
{
	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0)
	{
		lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
		return;
	}

	// send request to Real-Time core app to read temperature, pressure, and humidity
	ic_control_block.cmd = LP_IC_ENVIRONMENT_SENSOR;
	lp_sendInterCoreMessage(&ic_control_block, sizeof(ic_control_block));
}

/// <summary>
/// Callback handler for Inter-Core Messaging - Does Device Twin Update, and Event Message
/// </summary>
static void InterCoreHandler(LP_INTER_CORE_BLOCK* ic_message_block)
{
	static int msgId = 0;

	switch (ic_message_block->cmd)
	{
	case LP_IC_ENVIRONMENT_SENSOR:
		if (snprintf(msgBuffer, JSON_MESSAGE_BYTES, msgTemplate, ic_message_block->temperature, ic_message_block->pressure, msgId++) > 0) {

			Log_Debug("%s\n", msgBuffer);
			lp_azureMsgSendWithProperties(msgBuffer, appProperties, NELEMS(appProperties));
		}
		break;
	default:
		break;
	}
}



/// <summary>
///  Initialize PeripheralGpios, device twins, direct methods, timers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static void InitPeripheralAndHandlers(void)
{
	lp_azureIdScopeSet(lp_config.scopeId);

	lp_gpioOpenSet(gpioSet, NELEMS(gpioSet));

	lp_deviceTwinOpenSet(deviceTwinBindingSet, NELEMS(deviceTwinBindingSet));
	lp_directMethodOpenSet(directMethodBindingSet, NELEMS(directMethodBindingSet));

	lp_timerStartSet(timerSet, NELEMS(timerSet));
	lp_azureToDeviceStart();

	lp_interCoreCommunicationsEnable(lp_config.rtComponentId, InterCoreHandler);  // Initialize Inter Core Communications

	ic_control_block.cmd = LP_IC_HEARTBEAT;		// Prime RT Core with Component ID Signature
	lp_sendInterCoreMessage(&ic_control_block, sizeof(ic_control_block));
}

/// <summary>
/// Close PeripheralGpios and handlers.
/// </summary>
static void ClosePeripheralAndHandlers(void)
{
	Log_Debug("Closing file descriptors\n");

	lp_timerStopSet(timerSet, NELEMS(timerSet));
	lp_azureToDeviceStop();

	lp_gpioCloseSet(gpioSet, NELEMS(gpioSet));

	lp_deviceTwinCloseSet();
	lp_directMethodCloseSet();

	lp_timerStopEventLoop();
}

int main(int argc, char* argv[])
{
	lp_registerTerminationHandler();
	
	lp_configParseCmdLineArguments(argc, argv, &lp_config);
	if (!lp_configValidate(&lp_config)) {
		return lp_getTerminationExitCode();
	}

	InitPeripheralAndHandlers();

	// Main loop
	while (!lp_isTerminationRequired())
	{
		int result = EventLoop_Run(lp_timerGetEventLoop(), -1, true);
		// Continue if interrupted by signal, e.g. due to breakpoint being set.
		if (result == -1 && errno != EINTR)
		{
			lp_terminate(ExitCode_Main_EventLoopFail);
		}
	}

	ClosePeripheralAndHandlers();

	Log_Debug("Application exiting.\n");
	return lp_getTerminationExitCode();
}