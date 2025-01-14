#!/bin/sh
wget "https://lz4.overpass-api.de/api/interpreter?data=
[out:xml][timeout:25];
(node["natural"="water"]["type"="multipolygon"]({{bbox}});
way["natural"="water"]["type"="multipolygon"]({{bbox}});
relation["natural"="water"]["type"="multipolygon"]({{bbox}});
node["natural"="water"]({{bbox}});
way["natural"="water"]({{bbox}});
relation["natural"="water"]({{bbox}}););
(._;>;);out body;
" -O ../data/benNevis_water.xml

### Align
gdal_rasterize -l lines -l multipolygons -burn 255 -tr 0.00001 0.00001 -a_nodata 0.0 -te -5.127361122222 56.761527788889 -4.96513889987 56.864861122305 -ot Float32 -of GTiff -co "PROFILE=GeoTIFF" ../data/benNevis_water.xml ../data/benNevis_water.tiff

### Scale down
gdalwarp -r cubic -tap -ot Byte -srcnodata -1 -dstnodata 255 -s_srs WGS84 -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' -tr 3 3 ../data/benNevis_water.tiff ../data/benNevis_water_UTM.tiff