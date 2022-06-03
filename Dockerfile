FROM paulscherrerinstitute/std-daq-buffer-base:1.0.4

COPY . /std_daq_buffer/

RUN mkdir /std_daq_buffer/build && \
    cd /std_daq_buffer/build && \
    # Build the project for a specific detector.
    cmake3 .. \
    make

WORKDIR /std_daq_buffer/build

CMD ["bash"]
