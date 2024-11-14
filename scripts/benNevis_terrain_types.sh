wget "https://catalogue.ceh.ac.uk/maps/4728dd2d-064f-4532-be85-ecafc283bdcf?language=eng&SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&CRS=CRS:84&LAYERS=LC.10m.GB&STYLE=default&BBOX=-5.1272,56.7612,-4.9651,56.8646&FORMAT=image/tiff&WIDTH=2048&HEIGHT=1305" -O ../data/benNevis_terrain_types.tiff

gdalwarp -r cubic -ot Byte -tap -tr 10 10 -s_srs WGS84 -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' ../data/benNevis_terrain_types.tiff ../data/benNevis_terrain_types_aligned.tiff
# gdalwarp -r cubic -tap -tr 0.0001 0.0001 -s_srs WGS84 -t_srs WGS84 -te -5.127361122222 56.761527788889 -4.96513889987 56.864861122305 ../data/benNevis_terrain_types.tiff ../data/benNevis_terrain_types_aligned.tiff

rm ../data/benNevis_terrain_types.tiff
mv ../data/benNevis_terrain_types_aligned.tiff ../data/benNevis_terrain_types.tiff