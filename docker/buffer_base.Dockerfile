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

RUN echo "source /opt/rh/devtoolset-10/enable" >> /root/.bashrc
RUN echo "conda activate" >> /root/.bashrc

SHELL [ "/bin/bash", "-c", "-l" ]