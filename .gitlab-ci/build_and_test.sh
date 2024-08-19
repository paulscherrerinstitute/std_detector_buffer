#!/bin/bash

set -ex

echo ">>>>> Release build:"

cmake -B release -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build release

echo ">>>>> Unit testing:"
ctest --test-dir release -VV

echo ">>>>> Integration testing:"
if [[ ! -d .venv ]]; then
  python3 -m venv .venv
fi

source .venv/bin/activate
python3 -m pip install -r requirements.txt
pytest testing --junitxml=release/integration_tests.xml

echo "All tasks completed successfully."
