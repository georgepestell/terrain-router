# Description

Traditional GPS routing technologies are ubiquitous in today's urbanized car-centred society. These routing technologies rely upon pre-defined paths and roads to produce route suggestions, optimizing for time and (more recently) fuel efficiency. However, radical problems such the climate and health emergencies requires radical changes to the ways we get around. Humans have the capacity to traverse a far wider range of terrains than cars and public transport - often not requiring the use of paths at all. In rural environments, the existence of paths can often be sparce, leaving a small number of roads connecting most of the landscape. This leaves pedestrians less experienced with the safe ways to get across the terrain to use the longer, more aduous, and potentially dangerous road routes. Services such as OS Maps and Google Map's Terrain View offer difficulty ranking using terrain data, yet these yet again provide little benefit where there are little paths and cannot offer the flexibility of a true terrain sensitive routing algorithim.

The need for a more generalizable form of routing algorithm is clear. One not restricted to pre-defined paths are roads, and one that can take into consideration the needs and abilities of different users. This project aims to create an algorithmic framework for recommending arbitrary routes over terrain. The ranking of potential routes will use a tunable algorithm, allowing different speed-impact relationships to be defined between different data sources for each potential step along the route.

# Objectives
## Primary - Routing Algorithm
The key artefact will be the core routing algorithm - taking two points, and outputting a suggested best-route.

This suggestion should be based upon the following data sources:
- Terrain type
- Gradient
- Weather history
- Terrain features
- Human features
- Street lighting
It should also allow the following preference inputs:
- Preferred difficulty
- Mobility
- Other user preferences (e.g. no forests)
## Primary Modular Ranking Algorithm
An essential feature the success of this algorithm rests upon is the ability for it to model complex relationships between data in determining the best route. A modular design of the ranking algorithm allows for this because the ability to add new data inputs, and to define an re-define 

## Secondary - Output Visualization

# Ethics
There are no ethical issues raised by this project.
# Resources
Nothing beyond standard lab provision.