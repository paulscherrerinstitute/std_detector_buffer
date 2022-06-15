FROM paulscherrerinstitute/std_detector_buffer_base:2.1.2

COPY . /std_detector_buffer/
ENV CONAN_USER_HOME=/connan_cache

RUN mkdir /build && \
    cd /std_detector_buffer && \
    cmake -B /build -DCMAKE_BUILD_TYPE=Debug && \
    cd /build && \
    make && \
    ctest

WORKDIR /build

CMD ["bash"]
