#!/bin/bash

NAME="paulscherrerinstitute/std_detector_buffer_base"
VERSION=2.1.2

docker build -f buffer_base.Dockerfile -t ${NAME} .
docker tag ${NAME} ${NAME}:${VERSION}

docker login
docker push ${NAME}:${VERSION}
docker push ${NAME}:latest
