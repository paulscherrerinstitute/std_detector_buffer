---
sidebar_position: 4
id: documentation
title: Documentation
---

# Documentation

This documentation is built using [Docusaurus V2](https://www.docusaurus.io) using [Markdown](https://www.markdownguide.org/).

## Source code

The documentation code is hosted on https://git.psi.ch/HPDI/std_daq_doc/ (accessible only from PSI's network) and you are welcome to add/update/edit it.

### Remote development

Access https://git.psi.ch/HPDI/std_daq_doc, find the page you want to edit under https://git.psi.ch/HPDI/std_daq_doc/-/tree/master/std-daq-doc/docs and do the necessary changes.

:::note

These changes will go live after the commit is pushed to the main branch. Please, use it carefully.

:::

### Local Development

Clone the repo to your local machine 

```bash
$ git clone https://git.psi.ch/HPDI/std_daq_doc.git
```

cd to the `std-daq-doc` subfolder and start the local server:

```bash
$ cd std-daq-doc/std-daq-doc
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

The documentation is automatically built after each commit to the master branch using a dedicated gitlab-runner. The instructions for the build are described here: https://git.psi.ch/HPDI/std_daq_doc/-/blob/master/.gitlab-ci.yml

:::note

The new changes might take up to a few minutes until they are live. 

:::