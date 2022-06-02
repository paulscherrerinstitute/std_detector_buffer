#ifndef STD_DETECTOR_BUFFER_DETECTOR_HPP
#define STD_DETECTOR_BUFFER_DETECTOR_HPP

#ifdef FOR_GIGAFROST
    #include "gigafrost.hpp"
#elif FOR_JUNGFRAU
    #include "jungfrau.hpp"
#elif FOR_EIGER
    #include "eiger.hpp"
#elif FOR_JFJOCH
    #include "jungfraujoch.hpp"
#endif

#endif //STD_DETECTOR_BUFFER_DETECTOR_HPP
