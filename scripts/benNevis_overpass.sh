#!/bin/sh

wget "https://lz4.overpass-api.de/api/interpreter?data=
[out:xml][timeout:25];
(
nwr[natural='water'](56.750440999999995,-5.1407810000000005,56.871590999999995,-4.957440999999999);
nwr[natural='bay'](56.750440999999995,-5.1407810000000005,56.871590999999995,-4.957440999999999);
);
(._;>;);out+body;
" -O ../data/benNevis.xml

gdal_rasterize -l lines -l multipolygons -burn 1.0 -tr 0.000277 0.000277 -a_nodata 0.0 -te -5.127361122222 56.761527788889 -4.96513889987 56.864861122305 -ot Float32 -of GTiff ../data/benNevis.xml ../data/benNevis_water.tiff#

gdalwarp ../data/benNevis_water.tiff ../data/benNevis_water.xyz
