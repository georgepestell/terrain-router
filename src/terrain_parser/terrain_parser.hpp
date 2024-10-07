#pragma once 

#include <charconv>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

#include <cassert>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_3.h>

#define DEFAULT_TERRAIN_FILE "../data/benNevis_COP30.asc"

#define DENSITY 1

namespace terrainParserLib {

void parseFile(std::string filepath);

}