FROM jupyter/minimal-notebook:latest

RUN pip install jupyterlab_h5web[full]
