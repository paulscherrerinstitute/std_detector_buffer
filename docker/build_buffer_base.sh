#!/bin/bash

NAME="paulscherrerinstitute/std_detector_buffer_centos8"
VERSION=0.9.0

docker build --no-cache -f buffer_base_centos8.Dockerfile -t ${NAME} .
docker tag ${NAME} ${NAME}:${VERSION}

docker login
docker push ${NAME}:${VERSION}
docker push ${NAME}:latest
