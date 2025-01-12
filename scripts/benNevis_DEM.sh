OPEN_TOP_KEY=$(cat ../opentop.key)

curl "https://portal.opentopography.org/API/globaldem?demtype=COP30&south=56.761527788889&west=-5.127361122222&north=56.864861122305&east=-4.96513889987&outputFormat=GeoTiff&API_Key=${OPEN_TOP_KEY}" -o ../data/benNevis_DEM.tiff

gdalwarp -ot UInt16 -s_srs WGS84 -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' ../data/benNevis_DEM.tiff ../data/benNevis_DEM.xyz

