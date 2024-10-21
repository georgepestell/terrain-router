# Terrain Simplification
`CGAL::remesh_almost_planar_regions` was used to simplify the mesh whilst retaining the topology within the margin of error given by the dataset. There are four configuration values that determine the quality and level of simplification of the resulting mesh.

- Cosine of maximum angle for the region detection

The cosine of the maximum angle for region detection determines the maximum angle between the norms of faces before the faces are no longer considered part of the same plane. Increasing this value increases 

- Maximum distance for the region detection

The maximum distance for region detection sets a cut-off for the maximum deviation a point may have from the original mesh if considered part of the same plane and simplified.  This ensures that edges are not shifted away, retaining more detail with lower values.

- Cosine of maximum angle for sharp edge detection

Increasing this value means that only smaller angular deviations between normals will be clssified as sharp edges. This may detect more subtle sharp edges. Lower values result in only pronounced sharp edges being identified.

- Maximum distance for the sharp edge detection

The maximum distance for edge detection determines the size of the region to consider when looking at edge normals. An increased value will consider a wider area, and potentially miss small sharp edges. A smaller value will consider the differences between more face normals, thus often detecting more sharp edges.

# Feature Tagging / Terrain Representation

The initial concept was to do a Delaunay triangulation of the topography, and then add constraints to the TIN using the edges of the other data. I could store each of the data layers in their triangulated form first, as well as a final traversal TIN. However, the storage requirements may be great.