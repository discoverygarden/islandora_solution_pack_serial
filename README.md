# Serial Solution Pack [![Build Status](https://travis-ci.org/discoverygarden/islandora_solution_pack_serial.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_solution_pack_serial)

## Introduction

Adds all required Fedora objects to allow users to model, ingest and view objects modelled as serials within the Islandora interface.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [jsTree](https://github.com/vakata/jstree) version 3.0.0. (Does not work with later versions.)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

Download/clone the [jsTree](https://github.com/vakata/jstree) library to `sites/all/libraries/jstree`. Version 3.0.0 is required.

## Configuration

* The "Islandora Serial Object Tree View" block may be enabled to view the full structure of a Serial object. Required to expose and manage serial object hierarchies.
* The "View: Islandora Serial Object Intermediate PDF View" block may be enabled if the [Islandora Solr Views](https://github.com/Islandora/islandora_solr_views) module is installed.
* To make PDFs visible when viewing serial objects, configure the solution pack (admin/islandora/solution_pack_config/serial) and check "Display PDFs and sections in the jsTree view".

## Preparing and ingesting serial objects

* To create a top-level container object (e.g. a journal), ingest an object with the Serial Object content model.
* Under Serial Level Configuration, set up the structure of your serial. (e.g. Level 1 might use the MODS detail type "volume", and the label "Volume". Level 2 might be "issue" and "Issue".)
* After clicking Next, the "Create Intermediate Children" screen comes up. Here you can add levels (e.g. volumes and issues), which will act as containers for your PDFs. 
** Click "Add Another" to create a Level 1 container. It is automatically assigned the Level 1 label. 
** Click "Add Another", and a second Level 1 container will appear. You can drag it to the right to make it a Level 2 container. The label will change.
* To ingest a PDF under a given level, click on that level in the tree, and click Manage. Then click PDF Manageement, and add PDFs or sections.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)


## Maintainers/Sponsors
Current maintainers:

* [discoverygarden](https://github.com/discoverygarden)

This project has been sponsored by:

* [Florida Virtual Campus](www.flvc.org)
FLVC provide access to online student and library support services, and to serve as a statewide resource and clearinghouse for technology-based public postsecondary education distance learning courses and degree programs.

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
