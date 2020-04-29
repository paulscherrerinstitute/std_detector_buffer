#include <stdexcept>
#include <SFWriter.hpp>
#include "FastQueue.hpp"

using namespace std;

template <class T>
FastQueue<T>::FastQueue(
        const size_t slot_data_n_bytes,
        const uint16_t n_slots) :
            slot_n_bytes_(slot_data_n_bytes + sizeof(T)),
            n_slots_(n_slots)
{
    buffer_ = new char[slot_n_bytes_ * n_slots_];
    buffer_status_ = new atomic_int[n_slots];

    // TODO: Are atomic variables initialized?
    for (size_t i=0; i < n_slots_; i++) {
        buffer_status_[i] = 0;
    }

    write_slot_id_ = 0;
    read_slot_id_ = 0;
}

template <class T>
FastQueue<T>::~FastQueue()
{
    delete[] buffer_;
    delete[] buffer_status_;
}

template<class T>
T* FastQueue<T>::get_metadata_buffer(const int slot_id)
{
    return (T*)(buffer_ + (slot_id * slot_n_bytes_));
}

template<class T>
char* FastQueue<T>::get_data_buffer(const int slot_id)
{
    return (char*)(buffer_ + (slot_id * slot_n_bytes_) + sizeof(T));
}

template<class T>
int FastQueue<T>::reserve()
{
    // If (buffer_status==SLOT_EMPTY) buffer_status=SLOT_RESERVED.
    bool slot_reserved =
            buffer_status_[write_slot_id_].compare_exchange_strong(
                SLOT_EMPTY, SLOT_RESERVED);

    if (!slot_reserved) {
        return -1;
    }

    return write_slot_id_;
}

template<class T>
void FastQueue<T>::commit()
{
    // If (buffer_status==SLOT_RESERVED) buffer_status=SLOT_READY.
    bool slot_ready =
            buffer_status_[write_slot_id_].compare_exchange_strong(
                    SLOT_RESERVED, SLOT_READY);

    if (!slot_ready) {
        throw runtime_error("Slot should be reserved first.");
    }

    write_slot_id_++;
    write_slot_id_ %= n_slots_;
}

template<class T>
int FastQueue<T>::read()
{
    if (buffer_status_[read_slot_id_] != SLOT_READY) {
        return -1;
    }

    return read_slot_id_;
}

template<class T>
void FastQueue<T>::release()
{
    // If (buffer_status==SLOT_RESERVED) buffer_status=SLOT_READY.
    bool slot_empty =
            buffer_status_[read_slot_id_].compare_exchange_strong(
                    SLOT_READY, SLOT_EMPTY);

    if (!slot_empty) {
        throw runtime_error("Slot should be ready first.");
    }

    read_slot_id_++;
    read_slot_id_ %= n_slots_;
}

template class FastQueue<DetectorFrame>;