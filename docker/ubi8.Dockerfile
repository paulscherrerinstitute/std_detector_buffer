FROM redhat/ubi8

LABEL maintainer="Maciej Patro"

RUN dnf update -y \
    dnf install -y --setopt=tsflags=nodocs https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm \
    && dnf -y install bzip2 \
        git \
        zstd \
        openssh-clients \
        patch \
        gcc-toolset-13-gcc  \
        gcc-toolset-13-gcc-c++\
        rpm-build \
        wget \
        python3.11-pip \
        zeromq-devel \
        hdf5-devel \
    && dnf clean all \
    && rm -rf /var/cache/dnf

RUN python3 -m pip install --upgrade pip
RUN python3 -m pip install cmake conan==1.59 ninja pyinstaller numpy

RUN echo "source scl_source enable gcc-toolset-13" >> /etc/profile.d/gcc-toolset-13.sh
RUN chmod +x /etc/profile.d/gcc-toolset-13.sh
CMD ["/bin/bash"]
