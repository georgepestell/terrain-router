---
author: George Pestell (200007413)
date: 2024-10-05
title: |-
  Terrain Sensitive Routing

  Context Survey
---
# Route Planners

Typical route planners aim to allow users to specify two points on the
earth's surface as latitude and longitude coordinates, and then
determine and display a suggested route across earth's surface between
those two points. The most popular consumer applications such as Google
Maps, Apple Maps, and Open Street Maps rely on a graph where edges
between nodes represent road/path sections, and nodes represent
intersections where someone could change the path they are on. Basic
implementations weight the cost of edges based on the distance between
the two points represented by the nodes.

Modern advances in specialist applications such as Google's Terrain
View, and OS-Maps consider elevation of the paths and roads, offering
walking difficulty scores and ranking for hikers. Services such as Ways
can consider both permanent , and transient data through community
sourcing. This includes speed-cameras, road-works, congestion, and more.
However, this data is still limited to improving the suggestions of
roads and paths between two points.

	# DEMs

The topology of the terrain is a crucial factor in determining the speed
and feasibility of routes. Digital elevation models (DEMs) are raster
data structures representing this data. It contains a set of points in a
given space, along with their elevation [@DEMS].

Regular DEMs have elevation readings evenly spaced along a grid
[@regularDEMS]. Often, the X and Y position is omitted from each point,
instead a lower left X and Y position of the grid is given, along with a
row and column count, and cell size giving the X,Y are each grid
position represents. This allows the X and Y to be calculated by
multiplying the column and row by the cell size.

# Graph Representation

Typical routing algorithms such as Dijkstra's algorithm and A\* require
a graph structure where nodes represent points on a 2D surface, with
connecting edges representing the possible connections between them.

One method to create this graph is to first construct a mesh
representing the terrain, and then considering the center of each face a
node, with edges connecting adjacent face centers. This requires taking
the raster DEM data, and using it to create a mesh.

There are various methods for creating meshes which will be discussed
here. A popular approach is to create a triangle irregular network
(TIN). TINs use a triangulation algorithm such as Delaunay triangulation
to create a mesh where each face is a triangle [@tins]. They can very
accurately represent different topological features, and so are used
widely in 3D modelling [@tins3DModels].

Another method for mesh derivation is the square-based mesh-derivation.
This creates a mesh consisting of square/rectangle faces. This approach
is particularly suited to regular grid DEMs, because the data is equally
spaced, and so the resulting mesh consists of regularly sized square
faces. [@squareMesh]

Alternative mesh shapes can also be used for mesh derivation, UBER's H2
positioning library shows the potential to create a mesh from regular
hexagons [@uberH3]. Like with regular square meshes, this has the
benefit that each face is the same size, but has the added benefit that
the distance between hexagons is uniform in all directions, unlike
square meshes which have a different distance for adjacent and diagonal
faces, and unlike TINs which can have three different distances: one for
adjacent faces, and two different distances for the diagonal faces.

The mesh that will be generated from the DEM data will be 2.5D, as each
point on the DEM grid gets a single elevation value.

# TIN Derivation

There are an infinite number of way to use triangles to represent a
surface, and so an infinite number of TINs that would represent the same
DEM. For regular DEMs, the simplest way to triangulate points is to
connect each point as a grid, and then add a single edge in each square
to create evenly sized triangles. For irregular DEMs, or those with
missing points, a more sophisticated algorithm is required: Delaunay
triangulation.

Delaunay triangulation is the process of taking a set of 2D points and
forming a triangular mesh. It aims to limit very thin and long
triangles, in favour of more equilateral, evenly sized triangles
[@delaunayBasics].

2.5D Delaunay triangulation generates these meshes by conducting a 2D
Delaunay triangulation on the X and Y positions of the points given, and
then applies the Z values to each of the points afterwards
[@25DDelaunay]. This method is popular for TIN mesh derivation as the
DEM data is given as 2.5D data.

3D Delaunay Triangulation is an adaptation to the basic algorithm,
creating 3D structures from 3D tetrahedra instead of 2D triangles
[@3DDelaunay]. The is especially useful for accurate 3D modelling where
rich data is available. Unfortunately, it is often not required to
create terrain representations as the DEM data is often limited to 2.5D.

Constrained Delaunay Triangulation extends the algorithm through the
addition of constrained edges. Constrained edges cannot be removed or
altered in the triangulation algorithm, allowing edges such as the
boundaries of features (e.g. sides of rivers, buildings, etc.) to be
maintained [@constrainedDelaunay].

::: {#existing-libraries}
## C++ Libraries {#existing-libraries}
:::

A substantial amount of work has been put into creating libraries which
extract the complexities of creating an efficient Delaunay
implementation. The following is an exploration into C/C++ libraries
designed for Delaunay triangulation.

-   **CGAL** [@cgal] - This is a complete suite for computational
    geometry. It is able to do 2D, 2.5D, and 3D Delaunay triangulations,
    and also offers a multitude of simplification and smoothing tools.
    In addition to constrained Delaunay triangulation.

A drawback to this library is the amount of tools included can make it
difficult to determine the correct one to use, and the amount of
configuration options available can be difficult to understand. This is
also a benefit, as it reduces the requirements for additional
dependencies.

However, the benefits are that the documentation is extensive, and
covers many examples. In addition, the CGAL library itself is licenced
using the GPL v3+ open source licence giving the freedom to use the
software for any purposes.

-   **tin-terrain** [@tinTerrain] - This open-source tool has been
    specifically created for the generation of terrain TINs. It is no
    longer in active development.

A drawback to this library is the lack of additional tools for TIN
processing. It's main benefit is that it has a permissive GPL licence,
giving access to use it for any purposes.

-   **Fade25D** [@fade25d] - This set of tools is especially aimed at
    Delaunay triangulation of DEMs. It offers simple abstractions for
    simplifying and smoothing TINs, as well as the ability to add
    constraints through \"breakpoints\".

The benefits to using this library is the lack of explicit configuration
required. The amount of tools available specifically for terrain TIN
derivation makes it quick and easy to integrate. The documentation is
also complete. However, it doesn't contain a permissible open source
licence. A student licence is available, enabling restricted use of it's
tools for research and education, but one of the restrictions imposed on
this licence is a limit on the number of vertices and faces the TIN
contains.

# GIS Software

The amount of data that is reasonable to represent on a map is immense.
Geographic Information System (GIS) software is a category of
technologies used to create, visualize, and analyse this data. Data is
represented by layers which can include raster or vector data. This
organization allows users to easily manage and work with their data
[@GIS].

# Coordinate Systems

There are two primary categories of coordinate systems that can be used
to represent points on the earths surface [@coordinateCategories].

-   **Global Coordinate Systems**

-   **Projected Coordinate Systems**

A global coordinate systems (GCS) defines where data is located on the
earth's surface. The World Geodetic System from 1984 (WGS84) uses
latitude/longitude values referencing the relative angle away from the
equator (0 to 90 degrees of latitude) and from the prime meridian (0 to
180 degrees of longitude) [@GCS].

A projected coordinate system (PCS) defines how to flatten the positions
of the 3D globe onto a 2D surface such as a map. Many projections exist,
each with their own characteristics.

One of the drawbacks of a PCS is distortion: the pseudo Plate Carrée
projection distorts all angles, shapes, and distances to some degree
[@plateCarree]. This means that many PCS are unsuitable for distance
calculations or other spatial analysis tasks. However, WGS84 UTM
(Universal Transverse Mercator) [@WGS84UTM] is a projected coordinate
scheme based on WGS84 aiming to accurate project latitude longitude
coordinates to regular x and y coordinates where 1 x,y value is
equivalent to 1 meter . This makes it much more suitable for distance
calculations. The only exception to this uniformity is on the southwest
coast of Norway over open water, and around Svalbard.

# Shortest Path Algorithms

The goal of shortest path algorithms is to calculate the "shortest" path
along a graph data structure from a source node, across edges connecting
nodes, to a target node. Famous examples include Dijkstra's algorithm
and it's successor A\*. These algorithms use cost functions to weight
the edges of the graph, determining how "good" it is to travel across
that edge. This "goodness" is defined by the specific cost function
used. At its most basic, the cost function can reference the edge's
distance in real life. However, the cost function can utilize any
quantifiable characteristic, and model complex relationships between
different characteristics.

These algorithms begin from the source node (or alternatively the target
node in some implementations) and add all of the adjacent nodes to a
search queue. Then, a current and total cost score is calculated for
that node - current being the result of the cost function, and total
being the sum of the previous costs along the route. It's neighbours not
already on the route are then pushed to the queue.

The original Dijkstra's algorithm had no way to rank potential next
steps based on their likely goodness, and so had to complete a full
search of all nodes before finding an optimal path making it
inefficient. However, it is guaranteed to find the best path.

A\*'s main contribution was the introduction of heuristics -
computationally easy characteristics which estimate the cost. With an
admissible heuristic (i.e. one that never over-estimates the true cost),
A is guaranteed to find the optimal path first, saving much unnecessary
computation.

Whilst A\* is effective in static environments, where features are known
and do not change, it fails to adapt to changing information after it
has begun. Dynamic A\* (D\*) was developed [@dStar] to account for this
possibility - allowing the path to be updated without recalculating from
scratch where required. This lead to Field D\* [@fieldDStar], which
removed the restriction of following exactly between nodes and edges.
Instead, the environment is represented as a continuous field, allowing
cost functions to be defined for traversing across a face of the graph.

Field D\* is used extensively in applications such as Robotics and
real-time navigation. In Robotics in particular, there is a requirement
to prioritize terrain the robot is capable of following. Specific
research has used Field D\* for application over TINs [@Perkins2013].

# W. Evans' Terrain Sensitive Routing {#previous-research-on-terrain-sensitive-routing}

Previous work on creating a terrain sensitive routing application was
done by Will Evans for their dissertation project [@Evans2023]. Much of
their initial exploration into the topic serves as the groundwork for
this one and a summary of their work's findings will be given here.

Their routing algorithm was a basic implementation of the Dijkstra's
algorithm as they identified no admissible heuristic. They represented
the terrain using a regular square mesh. The characteristics considered
in the cost functions were limited to: binary features such as deep
water or paths, which are guaranteed to be untraversable/traversable;
and topological elevation/gradient data which affected how traversable an area is based on cost curves defining the effect of
certain gradients on walking speed.

Evans' approach was successful in generating routes between two points.
Unfortunately, no attempt was made to evaluate the quality of the routes
generated.

Many areas for potential improvements to improve the performance and
overall design of their code. Given that Dijkstra's original algorithm
must complete a full search before the optimal path is determined,
implementation of heuristics would greatly improve its efficiency.

Additionally, Evans identified improvements to the cost functions to
improve the quality of routes generated. First, dynamically adjusting
the impact of certain features based on the form of transportation (e.g.
cycling over some terrain is faster/slower than when on-foot). They
identified weather as a potentially important factor for determining
which routes are fast/safe. Additionally, instead of a simple effect
curve for the impact of gradient on walking speed, maintaining a
gradient history may model a users level of cardio-vascular stamina
remaining.

Another major improvement identified was in the mesh itself. Their
approach used a regular square grid, which resulted in unnecessary
computation for large areas with a consistent gradient. Evans suggests a
dynamic mesh, similar to TINs, could be used to add more nodes where
there is significant gradient change, and reduce the number of nodes
where there is not.

Finally, they suggest natively supporting vector formats for paths and
boundaries, reducing the likelihood of a lack of resolution causing
holes in the feature boundaries.

- Generating realistic terrains with higher-order Delaunay triangulations, Thierry de Kok, Marc van Kreveld, Maarten Löffler