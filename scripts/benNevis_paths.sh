wget "https://lz4.overpass-api.de/api/interpreter?data=
[out:xml][timeout:25];
(
  way['highway'~'trunk|secondary|unclassified|residential|tertiary|trunk_link|primary_link|secondary_link|tertiary_link|living_street|steps|road|footway|via_ferrata|sidewalk|crossing|cycleway|traffic_island|path|pedestrian|track|service']['foot'!~'no']['route'!~'ferry'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);
  way['foot'='yes']['route'!~'ferry'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);
  way['bridge'='yes']['foot'='yes'](56.761527788889, -5.127361122222, 56.864861122305, -4.96513889987);

  way["highway"~"footway|path|pedestrian|track|cycleway"](999);way["highway"]["foot"~"yes|designated|permissive|no"](999);way["highway"]["access"~"yes|designated|permissive"](999);way["sidewalk"~"yes|both|left|right"](999);way["bridge"]["sidewalk"~"yes|both|left|right"](999);way["bridge"]["passenger_lines"](if:t["passenger_lines"] > 0)(999);

);
(._;>;);out+body;
" -O ../data/benNevis_paths.xml

# Warp
ogr2ogr -f "GeoJSON" -t_srs '+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs' ../data/benNevis_paths.json ../data/benNevis_paths.xml lines