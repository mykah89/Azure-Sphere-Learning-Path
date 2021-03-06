# Lab 1: Introduction to Azure Sphere development

<!-- ![Azure Sphere with shield](resources/azure-sphere-shield.png) -->

---

**Author**: [Dave Glover](https://developer.microsoft.com/en-us/advocates/dave-glover?WT.mc_id=julyot-azd-dglover), Microsoft Cloud Developer Advocate, [@dglover](https://twitter.com/dglover)

---

## Azure Sphere Learning Path

Each module assumes you have completed the previous module.

[Home](../../../README.md)

* Lab 0: [Lab Set Up](../Lab_0_Introduction_and_Lab_Set_Up/README.md)
* Lab 1: [Introduction to Azure Sphere development](../Lab_1_Visual_Studio_and_Azure_Sphere/README.md)
* Lab 2: [Connect a room environment monitor to Azure IoT](../Lab_2_Send_Telemetry_to_Azure_IoT_Central/README.md)
* Lab 3: [Set the room virtual thermostat with Azure IoT Device Twins](../Lab_3_Control_Device_with_Device_Twins/README.md)
* Lab 4: [Remote reboot your Azure Sphere with Azure IoT Direct Methods](../Lab_4_Control_Device_with_Direct_Methods/README.md)
* Lab 5: [Integrate FreeRTOS Real-time room sensors with Azure IoT](../Lab_5_FreeRTOS_and_Inter-Core_Messaging/README.md)
* Lab 6: [Integrate Azure RTOS Real-time room sensors with Azure IoT](../Lab_6_AzureRTOS_and_Inter-Core_Messaging/README.md)
* Lab 7: [Connect and control your room environment monitor with Azure IoT](../Lab_7_Azure_IoT_Central_RTOS_Integration/README.md)
<!-- * Lab 8: [Over-the-air (OTA) Deployment](/docs/Lab_8_Over-the-air-deployment/README.md) -->

---

## What you will learn

In this lab, you will learn how to secure, build, deploy, and debug your first High-Level (HL) Azure Sphere application onto the Azure Sphere A7 Core.

---

## Prerequisites

This lab assumes you have completed **Lab 0: Lab set up, installation and configuration**

---

## Introduction to the Learning Path Labs

There are a number of Learning Path libraries that support these labs. These Learning Path C functions are prefixed with **lp_**, typedefs and enums are prefixed with **LP_**.

The Learning Path libraries are open source and contributions are welcome.

The Learning Path libraries are built from the [Azure Sphere Samples](https://github.com/Azure/azure-sphere-samples) and aim to demonstrate best practices.

The Learning Path libraries are **not** part of the official Azure Sphere libraries or samples.

---

## Tutorial Overview

1. Understand GPIO peripheral and Event Timer concepts.
2. Open the lab project with Visual Studio Code
3. Review the High-Level (A7) Core application security requirements.
4. Understand device manufacturer pin mappings.
5. Application deployment and debugging

---

## Key Concepts

In this lab you will learn about GPIO hardware interfaces, and event driven programming.

---

## Understanding Hardware

Most IoT solutions are designed to interface with hardware and interact with the real world. The most common interfaces on a device are GPIO, PWM, I2C, SPI, ADC, and UART.

### GPIO or General Purpose Input Output

Any GPIO pin can be designated (in software) as an input or output pin and can be used for a wide range of purposes. These labs use single pin GPIO peripherals for input and output, so the following is an introduction to GPIO pins.

**GPIO Output**

If a GPIO pin is designated as an *output* pin, then the software running on the Azure Sphere can set a pin to be either on or off. This equates to 3.3 volts for a pin that has been turned on, and zero volts when turned off. It is important to check that a peripheral connected to the pin is 3.3 volts tolerant. Otherwise you may destroy the peripheral. Lots of peripherals use single pins such as LEDs, relays, reed switches.

**GPIO Input**

If a GPIO pin is designated as an *input* pin, then the software running on the Azure Sphere can read a pin to determine if the voltage has been set to 3.3 volts, or zero volts/ground. It is essential to check that a peripheral will not set a voltage on a pin to anything higher than 3.3 volts **before** you connect it. Be warned, if you set a pin to a voltage higher than 3.3 volts you will destroy the Azure Sphere. An example of a GPIO peripheral that uses a single pin is a push button.

### Other Peripheral Interface Types

The following is a list of other common peripheral interfaces found on devices, including the Azure Sphere. To learn more about each interface type, right mouse click and open the link in a new browser window.

* [PWM - Pulse width modulation](https://en.wikipedia.org/wiki/Pulse-width_modulation)
* [I2C - Inter-Integrated Circuit](https://en.wikipedia.org/wiki/I%C2%B2C)
* [SPI - Serial Peripheral Interface](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)
* [ADC - Analog-to-digital converter](https://en.wikipedia.org/wiki/Analog-to-digital_converter)
* [UART - Universal asynchronous receiver-transmitter](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)

You will see references to **ISU** in the Azure Sphere and Mediatec documentation. An ISU is a Serial interface block and is an acronym for "**I**2C, **S**PI, **U**ART.". For more information refer to the [MT3620 Support Status](https://docs.microsoft.com/en-us/azure-sphere/hardware/mt3620-product-status?WT.mc_id=julyot-azd-dglover) page.

You can read more about the peripheral interfaces supported the Azure Sphere supports from the [MT3620 Support Status](https://docs.microsoft.com/en-us/azure-sphere/hardware/mt3620-product-status?WT.mc_id=julyot-azd-dglover) page.

---

## Developer Board Peripheral Interfaces

You can find detailed peripheral interface information for each board by clicking on the board image.

|Azure Sphere MT3620 Starter Kit Hardware User Guide|Seeed Studio Azure Sphere MT3620 Development Kit | Seeed Studio Azure Sphere MT3620 Mini Dev Board |
|-----|----|----|
| [![](resources/avnet-pinout.jpg)](https://www.avnet.com/opasdata/d120001/medias/docus/196/Azure%20Sphere%20Starter%20Kit%20User%20Guide%20(v1.3).pdf) | [![](resources/seeed-studio-rdb-pinmap.png)](http://wiki.seeedstudio.com/Azure_Sphere_MT3620_Development_Kit/) | [![](resources/seeed-studio-mini-pinmap.jpg)](http://wiki.seeedstudio.com/MT3620_Mini_Dev_Board/) |

---

## Input and Output Peripherals

In the Azure Sphere Learning Path labs there are several GPIO peripheral variables declared, including LEDs and a button. Variables of type **LP_GPIO** declare a GPIO model for **input** and **output** of single pin peripherals, such as LEDs, buttons, reed switches, and relays.

A GPIO peripheral variable holds the GPIO pin number, the initial state of the pin when the program starts, and whether the pin logic needs to be inverted.

The following example declares an LED **output** peripheral.

```c
static LP_GPIO alertLed = {
    .pin = ALERT_LED,                // The GPIO pin number
    .direction = LP_OUTPUT,          // for OUTPUT
    .initialState = GPIO_Value_Low,  // Set the initial state on the pin when opened
    .invertPin = true,               // Should the switching logic be reverse for on/off, high/low
    .name = "alertLed" };            // An arbitrary name for the peripheral
```

The following example declares a button **input** peripheral.

```c
static LP_GPIO buttonA = {
    .pin = BUTTON_A,
    .direction = LP_INPUT,
    .name = "buttonA" };
```

### Event Timers

Event Timers generate events which are bound to handler functions which implement desired actions. For example, blink an LED every second, or read a sensor every 10 seconds. [Event-driven programming](https://en.wikipedia.org/wiki/Event-driven_programming) helps to simplify application design.

![](resources/timer-events.png)

The labs make extensive use of timers, so there is a generalized model to simplify working with timers. There are two types of timers, **periodic timers**, and **one-shot timers**.

#### Periodic Timers

The following example is a variable named **measureSensorTimer** of type **LP_TIMER**. This event timer is initialized with a period of 10 seconds **{ 10, 0 }**. When the event timer triggers, the handler function **MeasureSensorHandler** is called to implement the action.

> There are two values used to initialize the **.period** variable. The first is the number of seconds, followed by the number of nanoseconds. If you wanted the timer to trigger events every half a second (500 milliseconds), you would set the .period to be { 0, 500000000 }.

```c
static LP_TIMER measureSensorTimer = {
    .period = { 10, 0 },	// Fire the timer event every 10 seconds + zero nanoseconds.
    .name = "measureSensorTimer",	// An arbitrary name for the timer, used for error handling
    .handler = MeasureSensorHandler	// The function handler called when the timer triggers.
};
```

The following code is the implementation of the **MeasureSensorHandler** handler function. This functions reads sensor data, serializes the data as JSON, then displays the JSON in the debug console.

```c
/// <summary>
/// Read sensor and send to Azure IoT
/// </summary>
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer)
{
    static int msgId = 0;
    static LP_ENVIRONMENT environment;

    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0)
    {
        lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
    }
    else {
        if (lp_readTelemetry(&environment) &&
            snprintf(msgBuffer, JSON_MESSAGE_BYTES, msgTemplate,
                environment.temperature, environment.humidity, environment.pressure, msgId++) > 0)
        {
            Log_Debug(msgBuffer);
        }
    }
}
```

#### One-Shot Timers

The following code uses a one-shot timer to blink the alert LED if button A is pressed. The LED turns on, and then a one-shot timer is set. When the one-shot timer triggers, its handler function is called to turn off the LED.

The advantage of this event-driven pattern is that the device can continue to service other events such as checking if a user has pressed a button.

The following is an example of a one-shot timer. The variable named **alertLedOffOneShotTimer** is of type **LP_TIMER**. This timer is initialized with a period of { 0, 0 }. Timers initialized with a period of 0 seconds are one-shot timers.

```c
static LP_TIMER alertLedOffOneShotTimer = {
    .period = { 0, 0 },
    .name = "alertLedOffOneShotTimer",
    .handler = AlertLedOffToggleHandler };
```

In the **ButtonPressCheckHandler** function, the alert LED is turned on when button A is pressed. Then a call is made to the **lp_timerOneShotSet** function to set the timer to 1 second. After 1 second, the **AlertLedOffToggleHandler** function is called.

```c
/// <summary>
/// Handler to check for Button Presses
/// </summary>
static void ButtonPressCheckHandler(EventLoopTimer* eventLoopTimer)
{
    static GPIO_Value_Type buttonAState;

    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
    }
    else {
        if (lp_gpioStateGet(&buttonA, &buttonAState))
        {
            lp_gpioOn(&alertLed);
            lp_timerOneShotSet(&alertLedOffOneShotTimer, &(struct timespec){1, 0});
        }
    }
}
```

When the one-shot timer triggers, the handler function **AlertLedOffToggleHandler** is called to turn off the alert LED.

```c
/// <summary>
/// One shot timer handler to turn off Alert LED
/// </summary>
static void AlertLedOffToggleHandler(EventLoopTimer* eventLoopTimer) {
    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
    }
    else {
        lp_gpioOff(&alertLed);
    }
}
```

### Automatic Initialization of GPIO Peripherals and Event Timers

GPIO peripherals and timers referenced in a **Set** will be automatically opened and closed.

```c
LP_GPIO* gpioSet[] = { &buttonA, &networkConnectedLed, &alertLed };
LP_TIMER* timerSet[] = { &buttonPressCheckTimer, &networkConnectionStatusTimer, &measureSensorTimer, &alertLedOffOneShotTimer };
```

These sets are referenced when calling **lp_gpioSetOpen**, and **lp_timerSetStart** from the **InitPeripheralsAndHandlers** function. The sets are also referenced when closing the gpio peripheral and timer sets in the **ClosePeripheralsAndHandlers** function.

```c
static void InitPeripheralsAndHandlers(void)
{
	lp_initializeDevKit();

	lp_gpioSetOpen(gpioSet, NELEMS(gpioSet));
	lp_timerSetStart(timerSet, NELEMS(timerSet));
}
```

### Easy to Extend

This model makes it easy to declare another gpio peripheral or timer and add them to the **gpio** or **timer** sets. The following is an example of adding a GPIO output peripheral.

```c
static LP_GPIO fanControl = {
    .pin = FAN1,                      // The GPIO pin number
    .direction = LP_OUTPUT,           // for OUTPUT
    .initialState = GPIO_Value_Low,   // Set the initial state on the pin when opened
    .invertPin = true,                // Should the switching logic be reverse for on/off, high/low
    .name = "FanControl"              // An arbitrary name for the senor.
};
```

Remember to add this new gpio peripheral to the **gpio set**. Adding the gpio peripheral to the set ensures automatic opening and closing.

```c
LP_GPIO* gpioSet[] = { &buttonA, &networkConnectedLed, &alertLed, &fanControl };
```

---

## Open Lab 1

Ensure you have followed all the instructions in the [lab set-up guide](../Lab_0_Introduction_and_Lab_Set_Up).

### Step 1: Start Visual Studio Code

![](resources/vs-code-start.png)

### Step 2: Open the lab project

1. Click **Open folder**.
2. Open the Azure-Sphere lab folder.
3. Select the **Lab_1_Visual_Studio_and_Azure_Sphere** folder and open.
4. Click **Select folder** or **OK** to open the folder.

![](resources/vs-code-open-project.png)

### Step 3: Set your developer board configuration

These labs support developer boards from AVNET and Seeed Studio. You need to set the configuration that matches your developer board.

The default developer board configuration is for the AVENT Azure Sphere Starter Kit. If you have this board, there is no additional configuration required.

1. Open CMakeList.txt
    ![](resources/vs-code-open-cmake.png)
2. Add a # at the beginning of the set AVNET line to disable it.
3. Uncomment the **set** command that corresponds to your Azure Sphere developer board.

    ```text
    set(AVNET TRUE "AVNET Azure Sphere Starter Kit")
    # set(SEEED_STUDIO_RDB TRUE "Seeed Studio Azure Sphere MT3620 Development Kit (aka Reference Design Board or rdb)")
    # set(SEEED_STUDIO_MINI TRUE "Seeed Studio Azure Sphere MT3620 Mini Dev Board")
    ```

4. Save the file. This will auto-generate the CMake cache.

### Step 4: Verify the Project Opened Correctly

From the **Solution Explorer**, open the **main.c** file.

![](resources/vs-code-open-main.png)

### Step 5: Check CMake Cache Builds Correctly

The CMake cache automatically builds when you open a CMake project. But given this is the first lab to be opened, it is a good idea to rerun the **CMake Cache Generator** to check that it builds correctly.

1. Right mouse click the **CMakeLists.txt** file and select **Clean Reconfigure All Projects**.

    ![](resources/vs-code-cmake-generate.png)

2. Check the **Output** window to verify that the CMake generation was successful. There should be a message to say **CMake generation finished**.

    ![](resources/vs-code-cmake-generate-completed.png)

    >If the CMake Generation fails, the directory paths may be too long. Move the labs to a directory closer to the root directory on your local drive. Such as c:\labs or ~/labs.

---

## Understanding Azure Sphere Security

Applications on Azure Sphere are locked down by default. You must grant capabilities to the application. Granting capabilities is key to Azure Sphere security and is also known as the [Principle of least privilege](https://en.wikipedia.org/wiki/Principle_of_least_privilege). You should only grant the capabilities the Azure Sphere application needs to run correctly, and no more.

Application capabilities include what hardware can be accessed, what network endpoints can be called (including Azure IoT Central and the Azure Device Provisioning Service), and what inter-core communications are allowed.

### Open the Application Manifest File

From Visual Studio Code, open the **app_manifest.json** file. The resources this application can access are limited to those listed in the **Capabilities** section.

```json
{
  "SchemaVersion": 1,
  "Name": "AzureSphereIoTCentral",
  "ComponentId": "25025d2c-66da-4448-bae1-ac26fcdd3627",
  "EntryPoint": "/bin/app",
  "CmdArgs": [ ],
  "Capabilities": {
    "Gpio": [
      "$BUTTON_A",
      "$ALERT_LED",
      "$NETWORK_CONNECTED_LED"
    ],
    "I2cMaster": [ "$I2cMaster2" ],
  },
  "ApplicationType": "Default"
}
```

<!-- ![](resources/visual-studio-application-capabilities.png) -->

## Understanding pin mappings

Each Azure Sphere manufacturer maps pins differently. Follow these steps to understand how the pins are mapped for your developer board.

1. Ensure you have the **main.c** file open. Place the cursor on the line that reads **#include "hw/azure_sphere_learning_path.h"**, then press <kbd>F12</kbd> to open the header file.

    <!-- ![](resources/visual-studio-open-azure-sphere-learning-path-pin-mappings.png) -->
    </br>

    ![](resources/open-azure-sphere-learning-path-pin-mappings.png)

3. Review the pin mappings set up for the Azure Sphere Learning Path using the Avnet Starter Kit.

    >Azure Sphere hardware is available from multiple vendors, and each vendor may expose features of the underlying chip in different ways. Azure Sphere applications manage hardware dependencies by using hardware definition files. For further information, review the [Managing target hardware dependencies](https://docs.microsoft.com/en-us/azure-sphere/app-development/manage-hardware-dependencies?WT.mc_id=julyot-azd-dglover) article.

    ```c
    /* Copyright (c) Microsoft Corporation. All rights reserved.
    Licensed under the MIT License. */

    // This file defines the mapping from the MT3620 reference development board (RDB) to the
    // 'sample hardware' abstraction used by the samples at https://github.com/Azure/azure-sphere-samples.
    // Some peripherals are on-board on the RDB, while other peripherals must be attached externally if needed.
    // https://docs.microsoft.com/en-us/azure-sphere/app-development/manage-hardware-dependencies
    // to enable apps to work across multiple hardware variants.

    // This file is autogenerated from ../../azure_sphere_learning_path.json.  Do not edit it directly.

    #pragma once
    #include "avnet_mt3620_sk.h"

    // Button A
    #define BUTTON_A AVNET_MT3620_SK_USER_BUTTON_A

    // Button B
    #define BUTTON_B AVNET_MT3620_SK_USER_BUTTON_B

    // LED 1
    #define LED1 AVNET_MT3620_SK_USER_LED_BLUE

    // LED 2
    #define LED2 AVNET_MT3620_SK_APP_STATUS_LED_YELLOW

    // AVNET: Network Connected
    #define NETWORK_CONNECTED_LED AVNET_MT3620_SK_WLAN_STATUS_LED_YELLOW

    // Click Relay
    #define RELAY AVNET_MT3620_SK_GPIO0
    ```

4. Next, from Visual Studio Code, open the main.c file to bring back into focus.



---

## Deploy the application to the Azure Sphere

### Connect the Azure Sphere device to your computer

1. Connect the Azure Sphere to your computer via USB.
2. Ensure you have [claimed](https://docs.microsoft.com/en-au/azure-sphere/install/claim-device?WT.mc_id=julyot-azd-dglover), [connected](https://docs.microsoft.com/en-au/azure-sphere/install/configure-wifi?WT.mc_id=julyot-azd-dglover), and [developer enabled](https://docs.microsoft.com/en-au/azure-sphere/install/qs-blink-application?WT.mc_id=julyot-azd-dglover) your Azure Sphere.
3. Ensure you have enabled developer mode on the Azure Sphere.
4. **For Linux users only**. Connect the Azure Sphere. From a terminal window, run the following command.

    ```bash
    sudo /opt/azurespheresdk/Tools/azsphere_connect.sh
    ```
    This command should return the following message: "**Azure Sphere device connected**".

    > Note: You need to rerun this command if you disconnect or reset the Azure Sphere or restart your computer.

### Start the app build deploy process

1. Ensure main.c is open.
2. Select **CMake: [Debug]: Ready** from the Visual Studio Code Status Bar.

    ![](resources/vs-code-start-application.png).

3. From Visual Studio Code, press <kbd>F5</kbd> to build, deploy, start, and attached the remote debugger to the application now running the Azure Sphere device.

---

## View the Debugger Output

1. Open the Visual Studio Code **Output** tab to view the output from **Log_Debug** statements in the code.

    >You can open the output window by using the Visual Studio Code <kbd>Ctrl+K Ctrl+H</kbd> shortcut or click the **Output** tab.

3. Observe every 10 seconds the output window will be updated with new data.

    ![Visual Studio View Output](resources/vs-code-view-output.png)

---

## Set a Debugger Breakpoint

1. In the **main.c** file, scroll down until you find the function named **MeasureSensorHandler**.

2. Set a breakpoint, click in the margin to the left of the line that reads **ConsumeEventLoopTimerEvent**.

    ![](resources/vs-code-set-breakpoint.png)

3. When the next timer event triggers, the debugger will stop at the line where you set the breakpoint.
4. You can inspect variable values, step over code <kbd>F10</kbd>, step into code <kbd>F11</kbd>, and continue code execution <kbd>F5</kbd>.
5. For more information on debugging then read the [Visual Studio Code Debugger](https://code.visualstudio.com/docs/editor/debugging?WT.mc_id=julyot-azd-dglover) article.

---

## Stop the Debugger

**Stop** the debugger by using the Visual Studio <kbd>Shift+F5</kbd> keyboard shortcut or click the **Stop Debugging** icon.

![](resources/vs-code-stop-debugger.png)


## Close Visual Studio Code

Now close **Close Visual Studio Code**.

---

## Finished 已完成 fertig 完了 finito समाप्त terminado

Congratulations, you secured, built, deployed, and debugged your first Azure Sphere application.

![](resources/finished.jpg)

---

**[NEXT](../Lab_2_Send_Telemetry_to_Azure_IoT_Central/README.md)**

---
