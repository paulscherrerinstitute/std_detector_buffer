#!/bin/bash

NAME="paulscherrerinstitute/std_detector_buffer_base"
VERSION=3.1.1

docker build --no-cache -f buffer_base.Dockerfile -t ${NAME} .
docker tag ${NAME} ${NAME}:${VERSION}

docker login
docker push ${NAME}:${VERSION}
docker push ${NAME}:latest
