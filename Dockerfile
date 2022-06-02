FROM paulscherrerinstitute/std-daq-buffer-base:1.0.4

ARG DETECTOR
RUN if [  -z $DETECTOR ];then \
  >&2 echo  "************* ERROR *************"; \
  >&2 echo "DETECTOR build-arg not set."; \
  >&2 echo "Use: docker build --build-arg DETECTOR=[detector_type]."; exit 1; \
  fi

COPY . /std_daq_buffer/

RUN mkdir /std_daq_buffer/build && \
    cd /std_daq_buffer/build && \
    # Build the project for a specific detector.
    cmake3 .. -DDETECTOR=$DETECTOR && \
    make

WORKDIR /std_daq_buffer/build

CMD ["bash"]
