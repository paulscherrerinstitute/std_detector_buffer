---
sidebar_position: 8
id: streamvis
title: Streamvis
---

# Overview

The online visualization tool used is [streamvis](https://github.com/paulscherrerinstitute/streamvis). It receives the stream generate by the [std-stream-send](../Components/std-stream-send) and displays on the browser.

## How to access

It runs on the daq node and is available at the port 5006. To access it navigate to http:daq-node:5006

:::tip

To access it remotely, one can forward port 5006 from the daq node to your local machine using: `ssh -N -f -L localhost:5006:localhost:5006 <username>@<daq-node>` and access localhost:5006 on your browser

:::
