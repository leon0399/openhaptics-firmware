// Override you configs in this file (Ctrl+Click)
#include "config/all.h"

#include <Arduino.h>
#include <Wire.h>

#include "openhaptics.h"

#include <bh_utils.hpp>
#include <connection_bhble.hpp>
#include <output_writers/pwm.hpp>

#if defined(BATTERY_ENABLED) && BATTERY_ENABLED == true
#include <battery/adc_naive.hpp>
#endif

using namespace OH;
using namespace BH;

static const size_t bhLayoutSize = BH_LAYOUT_TACTOSYF_SIZE;
static const oh_output_point_t* bhLayout[bhLayoutSize] = BH_LAYOUT_TACTOSYF;

void setupMode(OpenHaptics* app) {
  // Configure PWM pins to their positions on the feet
  auto footOutputs = mapMatrixCoordinates<AbstractOutputWriter>({
      // clang-format off
      {new PWMOutputWriter(32), new PWMOutputWriter(33), new PWMOutputWriter(25)}
      // clang-format on
  });

  OutputComponent* foot = new ClosestOutputComponent(OUTPUT_PATH_ACCESSORY, footOutputs);
  app->getOutput()->addComponent(foot);

#if defined(BATTERY_ENABLED) && BATTERY_ENABLED == true
  AbstractBattery* battery = new ADCNaiveBattery(36, { .sampleRate = BATTERY_SAMPLE_RATE }, app, tskNO_AFFINITY);
  app->setBattery(battery);
#endif

  uint8_t serialNumber[BH_SERIAL_NUMBER_LENGTH] = BH_SERIAL_NUMBER;
  ConnectionBHBLE_Config config = {
      .deviceName = BLUETOOTH_NAME,
      .appearance = BH_BLE_APPEARANCE,
      .serialNumber = serialNumber,
  };
  AbstractConnection* bhBleConnection = new ConnectionBHBLE(&config, [app](std::string& value)->void {
    plainOutputTransformer(app->getOutput(), value, bhLayout, bhLayoutSize, OUTPUT_PATH_ACCESSORY);
  }, app);
  app->setConnection(bhBleConnection);
}
