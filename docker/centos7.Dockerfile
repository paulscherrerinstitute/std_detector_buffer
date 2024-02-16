FROM centos:7

LABEL maintainer="Maciej Patro"

RUN yum update -y \
    && yum install -y centos-release-scl epel-release \
    && yum -y install bzip2 \
        git \
        zstd \
        openssh-clients \
        patch \
        centos-release-scl \
        devtoolset-11 \
        devtoolset-11-make \
        rpm-build \
        vim \
        hdf5-mpich-devel \
        wget \
        rh-python38-python-pip \
    && yum clean all \
    && rm -rf /var/cache/yum

RUN scl enable rh-python38 "python3 -m pip install --upgrade pip"
RUN scl enable rh-python38 "python3 -m pip install cmake conan ninja pyinstaller numpy"

RUN echo "source /opt/rh/devtoolset-11/enable" >> /etc/bashrc
RUN echo "source /opt/rh/rh-python38/enable" >> /etc/bashrc
RUN echo "module load mpi" >> /etc/bashrc
SHELL [ "/bin/bash", "-c", "-l" ]
