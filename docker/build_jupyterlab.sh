#!/bin/bash

NAME="paulscherrerinstitute/jupyterlab"
VERSION=1.0.0

docker build --no-cache -f jupyterlab.Dockerfile -t ${NAME} .
docker tag ${NAME} ${NAME}:${VERSION}

docker login
docker push ${NAME}:${VERSION}
docker push ${NAME}:latest
