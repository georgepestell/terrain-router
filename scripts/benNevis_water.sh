#!/bin/sh

mkdir ../data

wget "https://lz4.overpass-api.de/api/interpreter?data=
[out:xml][timeout:25];
(
nwr[natural='water'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);
nwr[natural='bay'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);
);
(._;>;);out+body;
" -O ../data/benNevis_water.xml

### Align
rm ../data/benNevis_water.tiff
gdal_rasterize -l lines -l multipolygons -burn 255 -tr 0.00001 0.00001 -a_nodata 0.0 -te -5.127361122222 56.761527788889 -4.96513889987 56.864861122305 -ot Float32 -of GTiff -co "PROFILE=GeoTIFF" ../data/benNevis_water.xml ../data/benNevis_water.tiff

### Scale down
rm ../data/benNevis_water_scaled.tiff
# gdalwarp -r cubic -tap -s_srs WGS84 -t_srs WGS84 -tr 0.0001 0.0001 ../data/benNevis_water.tiff ../data/benNevis_water_scaled.tiff
gdalwarp -r cubic -tap -ot Byte -s_srs WGS84 -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' -tr 3 3 ../data/benNevis_water.tiff ../data/benNevis_water_UTM.tiff
rm ../data/benNevis_water_scaled.tiff

### Convert to XYZ
# rm ../data/benNevis_water_scaled.xyz
# gdalwarp ../data/benNevis_water_scaled.tiff ../data/benNevis_water_scaled.xyz
# gdalwarp -s_srs WGS84 -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' ../data/benNevis_water.tiff ../data/benNevis_water.xyz

### Filter postiive values
# grep -v ".* .* 0$" ../data/benNevis_water_scaled.xyz > ../data/benNevis_water_scaled_filtered.xyz

# rm ../data/benNevis_water_scaled.xyz
# rm ../data/benNevis_water.tiff
rm ../data/benNevis_water.xml

mv ../data/benNevis_water_UTM.tiff ../data/benNevis_water.tiff

# mv ../data/benNevis_water_scaled.tiff ../data/benNevis_water.tiff
# mv ../data/benNevis_water_scaled_filtered.xyz ../data/benNevis_water.xyz