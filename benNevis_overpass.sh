# wget "https://lz4.overpass-api.de/api/interpreter?data=
# [out:xml][timeout:25];
# (
# nwr[natural='water'](56.750440999999995,-5.1407810000000005,56.871590999999995,-4.957440999999999);
# nwr[natural='bay'](56.750440999999995,-5.1407810000000005,56.871590999999995,-4.957440999999999);
# );
# (._;>;);out+body;
# "

gdal_rasterize -l lines -l multipolygons -burn 1.0 -tr 0.0002 0.0002 -a_nodata 0.0 -ot Float32 -of GTiff test.xml test.tif
