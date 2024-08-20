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

static const std::array<Position, BH_LAYOUT_TACTVISOR_SIZE> bhLayout = { BH_LAYOUT_TACTVISOR };

// Configure PWM pins to their positions on the face
auto faceOutputs = PlaneMapper_Margin::mapMatrixCoordinates<FloatPlane::Actuator*>({
  // clang-format off
  { new LedcOutput(32), new LedcOutput(33), new LedcOutput(25), new LedcOutput(26) },
  // clang-format on
});

void setup()
{
    app->getVibroBody()->addTarget(Target::FaceFront, new FloatPlane(faceOutputs));

    app->getVibroBody()->setup();

    auto* bhBleConnection = new BLE::Connection(
      {
        .deviceName = BLUETOOTH_NAME,
        .appearance = BH_BLE_APPEARANCE,
        .serialNumber = BH_SERIAL_NUMBER,
      },
      [](std::string& value) -> void {
          Decoder::applyPlain(app->getVibroBody(), value, bhLayout, Effect::Vibro, Target::FaceFront);
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
