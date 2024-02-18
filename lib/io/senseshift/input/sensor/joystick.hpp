#pragma once

#include "senseshift/input/sensor.hpp"

namespace SenseShift::Input {
    /**
     * Joystick axis sensor decorator
     */
    template<typename _Tp>
    class JoystickAxisSensor : public ISimpleSensor<_Tp> {
      private:
        ISimpleSensor<_Tp>* sensor;
        float dead_zone;

        int filterDeadZone(int in)
        {
            // This function clamps the input to the center of the range if
            // the value is within the threshold. This is to eliminate at-rest
            // noise of the joystick.
            int center = ANALOG_MAX / 2;
            return abs(center - in) < dead_zone * ANALOG_MAX ? center : in;
        }

      public:
        JoystickAxisSensor(ISimpleSensor<_Tp>* sensor, float dead_zone) : sensor(sensor), dead_zone(dead_zone){};

        void init() override { this->sensor->init(); };

        uint16_t getValue(void) override
        {
            auto value = this->sensor->getValue();
            value = this->filterDeadZone(value);
            return value;
        }
    };
}; // namespace SenseShift::Input