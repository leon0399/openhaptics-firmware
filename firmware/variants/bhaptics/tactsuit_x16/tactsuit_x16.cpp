// Override you configs in this file (Ctrl+Click)
#include "config/all.h"

#include <Arduino.h>
#include <Wire.h>

#include "senseshift.h"

#include "senseshift/arduino/input/sensor/analog.hpp"
#include "senseshift/arduino/output/ledc.hpp"
#include "senseshift/battery/input/battery_sensor.hpp"
#include "senseshift/bh/ble/connection.hpp"
#include "senseshift/bh/devices.hpp"
#include "senseshift/bh/encoding.hpp"
#include "senseshift/freertos/task.hpp"

using namespace SenseShift;
using namespace SenseShift::Input;
using namespace SenseShift::Input::Filter;
using namespace SenseShift::Arduino::Output;
using namespace SenseShift::Arduino::Input;
using namespace SenseShift::Battery;
using namespace SenseShift::Battery::Input;
using namespace SenseShift::BH;
using namespace SenseShift::Body::Haptics;

Application App;
Application* app = &App;

static const std::array<OutputLayout, BH_LAYOUT_TACTSUITX16_SIZE> bhLayout = { BH_LAYOUT_TACTSUITX16 };

// Ouput indices, responsible for x40 => x16 grouping
static const std::array<std::uint8_t, BH_LAYOUT_TACTSUITX16_GROUPS_SIZE> layoutGroups = BH_LAYOUT_TACTSUITX16_GROUPS;

void setup()
{
    // Configure PWM pins to their positions on the vest
    auto frontOutputs = PlaneMapper_Margin::mapMatrixCoordinates<FloatPlane::Actuator*>({
      // clang-format off
      { new LedcOutput(32), new LedcOutput(33), new LedcOutput(25), new LedcOutput(26) },
      { new LedcOutput(27), new LedcOutput(14), new LedcOutput(12), new LedcOutput(13) },
      // clang-format on
    });
    auto backOutputs = PlaneMapper_Margin::mapMatrixCoordinates<FloatPlane::Actuator*>({
      // clang-format off
      { new LedcOutput(19), new LedcOutput(18), new LedcOutput(5), new LedcOutput(17) },
      { new LedcOutput(16), new LedcOutput(4), new LedcOutput(2), new LedcOutput(15)  },
      // clang-format on
    });

    app->getVibroBody()->addTarget(Target::ChestFront, new FloatPlane(frontOutputs));
    app->getVibroBody()->addTarget(Target::ChestBack, new FloatPlane(backOutputs));

    app->getVibroBody()->setup();

    auto* bhBleConnection = new BLE::Connection(
      {
        .deviceName = BLUETOOTH_NAME,
        .appearance = BH_BLE_APPEARANCE,
        .serialNumber = BH_SERIAL_NUMBER,
      },
      [](std::string& value) -> void {
          Decoder::applyVestGrouped(app->getVibroBody(), value, bhLayout, layoutGroups);
      },
      app
    );
    bhBleConnection->begin();

#if defined(SS_BATTERY_ENABLED) && SS_BATTERY_ENABLED == true
    auto* batteryVoltageSensor = new SimpleSensorDecorator(new AnalogSimpleSensor(36));
    batteryVoltageSensor->addFilters({
      new MultiplyFilter(3.3F),                      // Convert to raw pin voltage
      new VoltageDividerFilter(27000.0F, 100000.0F), // Convert to voltage divider voltage
    });
    auto* batteryTask = new ::SenseShift::FreeRTOS::ComponentUpdateTask(
      batteryVoltageSensor,
      SS_BATTERY_SAMPLE_RATE,
      { "ADC Battery", 4096, SS_BATTERY_TASK_PRIORITY, tskNO_AFFINITY }
    );
    batteryTask->begin();

    auto* batterySensor = new LookupTableInterpolateBatterySensor(batteryVoltageSensor, &VoltageMap::LiPO_1S_42);
    batterySensor->addValueCallback([](BatteryState value) -> void {
        app->postEvent(new BatteryLevelEvent(value));
    });
    batterySensor->init();
#endif
}

void loop()
{
    // Free up the Arduino loop task
    vTaskDelete(NULL);
}
