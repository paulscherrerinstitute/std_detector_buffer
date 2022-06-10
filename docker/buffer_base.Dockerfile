FROM centos:7

LABEL description="Package with devtools-10 and conda"
LABEL maintainer="Maciej Patro"

RUN yum update -y \
    && yum install -y centos-release-scl \
    && yum -y install bzip2 \
        git \
        openssh-clients \
        patch \
        centos-release-scl \
        devtoolset-10 \
        devtoolset-10-make \
        rpm-build \
        vim \
        mpich-devel \
        wget \
        zeromq-devel\
    && yum clean all \
    && rm -rf /var/cache/yum

ARG CONDA_PREFIX=/opt/conda
ARG CONDA_CHECKSUM="935d72deb16e42739d69644977290395561b7a6db059b316958d97939e9bdf3d"

RUN curl https://repo.anaconda.com/miniconda/Miniconda3-py38_4.10.3-Linux-x86_64.sh -o miniconda.sh \
    && sha256sum miniconda.sh | grep -q "${CONDA_CHECKSUM}" \
    && sh miniconda.sh -b -p ${CONDA_PREFIX} \
    && rm miniconda.sh \
    && ${CONDA_PREFIX}/bin/conda config --set show_channel_urls True \
    && ${CONDA_PREFIX}/bin/conda config --set path_conflict prevent \
    && ${CONDA_PREFIX}/bin/conda config --set notify_outdated_conda false \
    && ${CONDA_PREFIX}/bin/conda update -c conda-forge --yes --all \
    && ${CONDA_PREFIX}/bin/conda install -c conda-forge --yes conda-build ninja conan cmake conda-verify coverage coverage-fixpaths \
    && ${CONDA_PREFIX}/bin/conda clean -tipy \
    && ${CONDA_PREFIX}/bin/conda-build purge-all \
    && ${CONDA_PREFIX}/bin/conda init --all

ENV PATH=/${CONDA_PREFIX}/bin:$PATH
ENV PATH="/usr/lib64/mpich/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/lib64/mpich/lib:${LD_LIBRARY_PATH}"

RUN echo "source /opt/rh/devtoolset-10/enable" >> /etc/bashrc
RUN echo "source activate" >> /etc/bashrc
SHELL [ "/bin/bash", "-c", "-l" ]

RUN wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.gz && \
    tar -xzf hdf5-1.12.0.tar.gz

WORKDIR /hdf5-1.12.0
RUN ./configure --enable-parallel && make install
RUN ln -v -s `pwd`/hdf5/lib/* /usr/lib64/ && \
    ln -v -s `pwd`/hdf5/include/* /usr/include/ && \
    ln -v -s /usr/include/mpich-x86_64/* /usr/include/
