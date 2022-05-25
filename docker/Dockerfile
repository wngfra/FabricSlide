# Copyright (c) 2020 wngfra
# Use of this source code is governed by the Apache-2.0 license, see LICENSE

ARG BASE_IMAGE=nvcr.io/nvidia/pytorch:22.04-py3
FROM $BASE_IMAGE

RUN conda update conda --yes && \
    conda install -c conda-forge --quiet --yes \
    bidict \
    pandas \
    ipympl \
    matplotlib \
    scipy \
    seaborn \
    scikit-learn \
    nodejs \
    jupyterlab && \
    conda clean -a -q -y && \
    npm cache clean --force && \
    jupyter lab clean

RUN pip3 install -U --no-cache-dir \
    scikit-fda

WORKDIR /workspace

# Add Tini. Tini operates as a process subreaper for jupyter. This prevents kernel crashes.
ENV TINI_VERSION v0.19.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /usr/bin/tini
RUN chmod +x /usr/bin/tini

EXPOSE 8888
ENTRYPOINT ["/usr/bin/tini", "--"]
CMD ["jupyter-lab", "--ip='0.0.0.0'", "--port=8888", "--no-browser", "--allow-root"]
