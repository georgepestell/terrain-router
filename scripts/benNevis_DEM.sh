OPEN_TOP_KEY=$(cat ../opentop.key)
curl "https://portal.opentopography.org/API/globaldem?demtype=COP30&south=56.7612&north=56.8646&west=-5.1272&east=-4.9651&outputFormat=GeoTiff&API_Key=${OPEN_TOP_KEY}" -o ../data/benNevis_DEM.tiff

# curl "https://portal.opentopography.org/API/globaldem?demtype=COP30&south=56.30&north=56.40&west=-2.84&east=-2.74&outputFormat=AAIGrid&API_Key=${OPEN_TOP_KEY}" -o ../data/stAndrews_COP30.asc

# curl "https://portal.opentopography.org/API/globaldem?demtype=COP30&south=56.00&north=57.00&west=-3.5&east=-1.50&outputFormat=AAIGrid&API_Key=${OPEN_TOP_KEY}" -o ../data/big_COP30.asc

rm ../data/benNevis_DEM.xyz
gdalwarp -ot UInt16 -s_srs WGS84 -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' ../data/benNevis_DEM.tiff ../data/benNevis_DEM.xyz
# gdalwarp -ot UInt16 -s_srs WGS84 -t_srs WGS84 ../data/benNevis_DEM.tiff ../data/benNevis_DEM.xyz

rm ../data/benNevis_DEM.tiff
