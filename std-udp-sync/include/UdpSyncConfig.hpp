#ifndef STD_DETECTOR_BUFFER_UDP_SYNC_CONFIG_HPP
#define STD_DETECTOR_BUFFER_UDP_SYNC_CONFIG_HPP

#include <string>
#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

struct UdpSyncConfig
{
  static UdpSyncConfig from_json_file(const std::string& filename)
  {
    std::ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document config_parameters;
    config_parameters.ParseStream(isw);

    return {config_parameters["detector_name"].GetString(), config_parameters["n_modules"].GetInt(),
            config_parameters["bit_depth"].GetInt()};
  }

  const std::string detector_name;
  const int n_modules;
  const int bit_depth;
};

#endif // STD_DETECTOR_BUFFER_UDP_SYNC_CONFIG_HPP
