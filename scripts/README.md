# Testing Scripts

These scripts are helpful for testing the fetching and processing of GDAL data from various API sources, and were used to construct the parsing in the main tsr library.

- For `benNevis_DEM.sh`, an API key from opentopography is required. This should be set as an environmental variable:

```bash 
# Set key value for this shell
$ export OPEN_TOP_KEY=<VALUE>
$ ./benNevis_DEM.sh

# Set key value for this command
$ OPEN_TOP_KEY=<VALUE> ./benNevis_DEM.sh
```

