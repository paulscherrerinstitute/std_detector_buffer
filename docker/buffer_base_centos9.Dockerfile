FROM quay.io/centos/centos:stream9

LABEL maintainer="Maciej Patro"

RUN dnf update -y \
    && dnf install -y epel-release \
    && dnf -y install bzip2 \
        git \
        zstd \
        openssh-clients \
        patch \
        gcc-toolset-13 \
        hdf5-mpich-devel \
        rpm-build \
        wget \
        python3.11-pip \
    && dnf clean all \
    && rm -rf /var/cache/dnf

RUN alternatives --install /usr/bin/python3 python3 /usr/bin/python3.11 1
RUN python3 -m pip install --upgrade pip
RUN python3 -m pip install cmake conan==1.59 ninja pyinstaller numpy

RUN echo "source scl_source enable gcc-toolset-13" >> /etc/profile.d/gcc-toolset-13.sh
RUN chmod +x /etc/profile.d/gcc-toolset-13.sh
CMD ["/bin/bash"]
