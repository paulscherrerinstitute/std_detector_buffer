#!/bin/bash

NAME="std_detector_buffer_base"
VERSION=1.0.0

docker build --no-cache=true -f buffer_base.Dockerfile -t paulscherrerinstitute/$NAME .
docker tag paulscherrerinstitute/$NAME paulscherrerinstitute/$NAME:$VERSION

docker login
docker push paulscherrerinstitute/$NAME:$VERSION
docker push paulscherrerinstitute/$NAME:
