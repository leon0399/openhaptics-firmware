#pragma once

#include <senseshift/opengloves/interface.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <frozen/map.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>

#include <og_protocol.hpp>
#include <senseshift/logging.hpp>

namespace SenseShift::OpenGloves {
    class AlphaEncodingService {
      public:
        inline static constexpr frozen::string valueSymbols = "0123456789";
        inline static const auto prefixToCommandMap = frozen::make_map<std::string, Sensor_t>({
          // clang-format off
          { "A", Joint::ThumbCurl },
          { "(AB)", Joint::ThumbSplay },
          { "B", Joint::IndexCurl },
          { "(BB)", Joint::IndexSplay },
          { "C", Joint::MiddleCurl },
          { "(CB)", Joint::MiddleSplay },
          { "D", Joint::RingCurl },
          { "(DB)", Joint::RingSplay },
          { "E", Joint::LittleCurl },
          { "(EB)", Joint::LittleSplay },
          { "O", DigitalSensorType::ButtonCalibrate },
          // clang-format on
        });

        inline static constexpr auto commandToPrefixMap = frozen::make_map<Sensor_t, frozen::string>({
          { Joint::ThumbCurl, "A" },
          { Joint::ThumbSplay, "(AB)" },
          { Joint::ThumbMCP_Curl, "(AAA)" },
          { Joint::ThumbIP_Curl, "(AAB)" },

          { Joint::IndexCurl, "B" },
          { Joint::IndexSplay, "(BB)" },
          { Joint::IndexMCP_Curl, "(BAA)" },
          { Joint::IndexPIP_Curl, "(BAB)" },
          { Joint::IndexDIP_Curl, "(BAC)" },

          { Joint::MiddleCurl, "C" },
          { Joint::MiddleSplay, "(CB)" },
          { Joint::MiddleMCP_Curl, "(CAA)" },
          { Joint::MiddlePIP_Curl, "(CAB)" },
          { Joint::MiddleDIP_Curl, "(CAC)" },

          { Joint::RingCurl, "D" },
          { Joint::RingSplay, "(DB)" },
          { Joint::RingMCP_Curl, "(DAA)" },
          { Joint::RingPIP_Curl, "(DAB)" },
          { Joint::RingDIP_Curl, "(DAC)" },

          { Joint::LittleCurl, "E" },
          { Joint::LittleSplay, "(EB)" },
          { Joint::LittleMCP_Curl, "(EAA)" },
          { Joint::LittlePIP_Curl, "(EAB)" },
          { Joint::LittleDIP_Curl, "(EAC)" },

          { AnalogSensorType::JoystickX, "F" },
          { AnalogSensorType::JoystickY, "G" },
          { DigitalSensorType::JoystickButton, "H" },

          { DigitalSensorType::GestureTrigger, "I" },
          { DigitalSensorType::GestureGrab, "L" },
          { DigitalSensorType::GesturePinch, "M" },

          { DigitalSensorType::ButtonA, "J" },
          { DigitalSensorType::ButtonB, "K" },
          { DigitalSensorType::ButtonMenu, "N" },
          { DigitalSensorType::ButtonCalibrate, "O" },

          { AnalogSensorType::Trigger, "P" },
        });

        AlphaEncodingService() = default;

        static const std::vector<Command_t> decode(const std::string& buffer);
        static const std::string encode(const std::vector<Command_t>& commands);

      private:
        static void splitCommand(const std::string& buffer, size_t start, size_t end, std::vector<Command_t>& dest);
    };
} // namespace SenseShift::OpenGloves