mkdir ../data

wget "https://lz4.overpass-api.de/api/interpreter?data=
[out:xml][timeout:25];
(
  way['highway'~'footway|path|pedestrian|track|service']['foot'!~'no'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);
  way['foot'='yes'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);
);
(._;>;);out+body;
" -O ../data/benNevis_paths.xml

# Warp
ogr2ogr -f "GeoJSON" -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' ../data/benNevis_paths_warped.json ../data/benNevis_paths.xml lines

# ogr2ogr -f "GeoJSON" ../data/benNevis_paths.json ../data/benNevis_paths_warped.xml


#ogr2ogr -f "ESRI Shapefile" -dialect sqlite -sql "SELECT ST_Buffer(geometry, 2) AS geometry, * FROM lines" ../data/benNevis_paths.shp ../data/benNevis_paths_warped.xml
gdal_rasterize  -burn 255 -tr 3 3 -a_nodata 0.0 -te -5.127361122222 56.761527788889 -4.96513889987 56.864861122305 -ot Float32 -of GTiff -co "PROFILE=GeoTIFF" ../data/benNevis_paths_warped.json ../data/benNevis_paths.tiff