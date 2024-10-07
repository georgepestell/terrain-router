# Feature Tagging / Terrain Representation

The initial concept was to do a Delaunay triangulation of the topography, and then add constraints to the TIN using the edges of the other data. I could store each of the data layers in their triangulated form first, as well as a final traversal TIN. However, the storage requirements may be great.