FROM centos:centos7

RUN yum -y install centos-release-scl epel-release && \
    yum -y update && \
    yum -y install  \
            devtoolset-10  \
            git  \
            vim  \
            python3 && \
    pip3 install --no-cache-dir \
		    cmake \
		    conan \
		    ninja \
		    gcovr \

SHELL ["scl", "enable", "devtoolset-10"]
