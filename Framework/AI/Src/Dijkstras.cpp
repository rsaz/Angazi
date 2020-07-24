#include "Precompiled.h"
#include "Dijkstras.h"

using namespace Angazi::AI;

Path Dijkstras::Search(const Graph & graph, const Coord& start, const Coord& end, std::function<bool(Coord)> isBlocked, std::function<float(Coord, Coord)> getCost)
{
	openList.clear();
	closedList.clear();
	parent.clear();
	g.clear();
	opened.clear();
	closed.clear();

	const int nodeCount = graph.GetColumns()*graph.GetRows();
	parent.resize(nodeCount);
	g.resize(nodeCount, 0.0f);
	opened.resize(nodeCount);
	closed.resize(nodeCount);

	// Add start to the open list
	openList.push_back(start);
	opened[graph.GetIndex(start)] = true;

	// Keep searching until we are done
	bool found = false;
	while (!found && !openList.empty())
	{
		// Pick the next node from the open list
		Coord current = openList.front();
		openList.pop_front();
		// If node is end, we are done
		if (current == end)
		{
			found = true;
		}
		else
		{
			// else, expand node
			auto currentNode = graph.GetNode(current);
			for (auto neighbor : currentNode->neighbors)
			{
				//If the neighbour is blocked, skip it
				const int neighborIndex = graph.GetIndex(neighbor);
				if (isBlocked(neighbor) || closed[neighborIndex])
					continue;

				const float cost = g[graph.GetIndex(current)] + getCost(current, neighbor);
				if (!opened[neighborIndex])
				{
					openList.push_back(neighbor);
					opened[neighborIndex] = true;
					parent[neighborIndex] = current;
					g[neighborIndex] = cost;

					//Wrong! Cannot just add to back
					// openList.push_back(neighbour);

					//for()
					//	check if cost < g[i]
					//		insert

					std::list<Coord>::iterator iter = openList.begin();
					for (; iter != openList.end(); iter++)
					{
						if (cost < g[graph.GetIndex(*iter)])
						{
							openList.remove(neighbor);
							openList.insert(iter, neighbor);
							break;
						}
					}
				}
				else if (cost < g[neighborIndex])
				{
					// update parent
					parent[neighborIndex] = current;
					// update g
					g[neighborIndex] = cost;
					// remove and re-insert using new g to sort
					openList.remove(neighbor);
					std::list<Coord>::iterator iter = openList.begin();
					for (; iter != openList.end(); iter++)
					{
						if (cost < g[graph.GetIndex(*iter)])
						{
							openList.insert(iter, neighbor);
							break;
						}
					}
				}
			}
		}
		// Close node
		closedList.push_back(current);
		closed[graph.GetIndex(current)] = true;
	}
	Path path;
	if (found)
	{
		std::list<Coord> trace;
		Coord next = end;
		while (next.IsValid())
		{
			trace.push_front(next);
			next = parent[graph.GetIndex(next)];
		}
		path.reserve(trace.size());
		for (auto node : trace)
			path.push_back(node);
	}

	return path;
}