#ifndef STD_DETECTOR_BUFFER_DET_WRITER_CONFIG_HPP
#define STD_DETECTOR_BUFFER_DET_WRITER_CONFIG_HPP

#include <string>
#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

struct DetWriterConfig {
    static DetWriterConfig from_json_file(const std::string& filename) {
        std::ifstream ifs(filename);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document config_parameters;
        config_parameters.ParseStream(isw);

        return {
                config_parameters["detector_name"].GetString(),
                config_parameters["image_height"].GetInt(),
                config_parameters["image_width"].GetInt(),
                config_parameters["bit_depth"].GetInt(),
        };
    }

    const std::string detector_name;
    const int image_height;
    const int image_width;
    const int bit_depth;
};

#endif // STD_DETECTOR_BUFFER_DET_WRITER_CONFIG_HPP
