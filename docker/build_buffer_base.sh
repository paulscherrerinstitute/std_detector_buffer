#!/bin/bash

NAME="std_detector_buffer_base"
VERSION=2.1.0

docker build -f buffer_base.Dockerfile -t paulscherrerinstitute/$NAME .
docker tag paulscherrerinstitute/$NAME paulscherrerinstitute/$NAME:$VERSION

docker login
docker push paulscherrerinstitute/$NAME:$VERSION
docker push paulscherrerinstitute/$NAME:latest
