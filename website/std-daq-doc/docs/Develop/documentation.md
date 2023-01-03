---
sidebar_position: 4
id: documentation
title: Documentation
---

# Documentation

This documentation is built using [Docusaurus V2](https://www.docusaurus.io) using [Markdown](https://www.markdownguide.org/).

## Source code

The documentation code is hosted on https://git.psi.ch/controls-ci/std_detector_buffer (accessible only from PSI's network) and you are welcome to add/update/edit it.

### Remote development

Access https://git.psi.ch/controls-ci/std_detector_buffer, find the page you want to edit website folder
:::note

These changes will go live after the commit is pushed to the main branch. Please, use it carefully.

:::

### Local Development

Clone the repo to your local machine 

```bash
$ git clone https://git.psi.ch/controls-ci/std_detector_buffer
```

cd to the `website/std-daq-doc` subfolder and start the local server:

```bash
$ cd std_detector_buffer/website/std-daq-doc
$ yarn start
```

This command starts a local development server and opens up a browser window. Most changes are reflected live without having to restart the server.

After the changes are done and verified, commit and push the changes to see them live.

```bash
git add . && git commit -m "descriptive commit message" && git push
```

:::tip

Alternatively, one can create a fork of the documentation and create a Merge Request to merge changes back to the main branch.

:::

## Build

The documentation is automatically built after each commit to the master branch using a dedicated gitlab-runner.
:::note

The new changes might take up to a few minutes until they are live. 

:::
