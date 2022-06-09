#ifndef STD_DETECTOR_BUFFER_SYNC_CONFIG_HPP
#define STD_DETECTOR_BUFFER_SYNC_CONFIG_HPP

namespace sync_config
{
    // Number of times we try to re-sync in case of failure.
    const int SYNC_RETRY_LIMIT = 3;
}

#endif // STD_DETECTOR_BUFFER_SYNC_CONFIG_HPP
