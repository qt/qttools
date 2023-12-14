How to recolor icons for doc.qt.io
==================================

You can view Qt Documentation online at https://doc.qt.io in dark and light
modes. To make the icons you use in the docs visible in both modes, save icon
files as gray-scale images with a transparent background.

If you receive a large number of icons that are not visible in either light
or dark mode or have a solid background, run the ``recolordocsicons.py``
Python script.

The script runs ImageMagick to recolor the images and optipng to optimize the
icon files.

## Preparation

To run the script, you will need to install the following tools and add them
to the PATH:

- Python 3.x (the script has been tested to work with 3.10)
- ImageMagick
- optipng

## Running the script

Use the `-docspath` option to specify the path to the folder where you stored
the icons.

For example, if you saved the new icons in `C:\iconconversions`, switch to
the `qttools\util\recolordocsicons` folder and enter:

`recolordocsicons.py -docspath C:\iconconversions`
