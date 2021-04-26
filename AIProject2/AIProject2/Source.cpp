#include <vector>x
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <Windows.h>
#include <thread>
#include <mutex>    
#include <random>
typedef bool geneSet[100];
std::mutex mtx;
struct chromosome
{
	std::vector<std::vector<int>> paths;
	geneSet genes;
	int fitnessValue = 0;
	bool operator < (const chromosome& str) const
	{
		return (fitnessValue > str.fitnessValue);
	}
};

typedef std::vector<chromosome> population;

struct parameters
{
	int maximumPathNumber = 3; // Maximum number of paths
	int mutateChance = 10;	   // Chance new populations genes will be mutated
	int crossChance = 50;	   // Chance new population will be crossed together
	double pathContributeValue = 40; //How much a single found path will contribute to the overall fitness value of a chromosome
};

std::vector<int> FindPath(chromosome* cm)
{
	struct node
	{
		node(int tile, bool busy) : tile(tile), busy(busy)
		{

		}
		const int tile;
		bool busy;
		int hcost = 1000;
		int gcost = 1000;
		int fcost = 1000;
		node* prev = nullptr;
	};
	std::vector<node> grid;
	std::vector<node*> open;
	std::vector<node*> closed;
	
	for (int i = 0; i < 100; i++)
		grid.push_back({i, cm->genes[i]});

	node* current = &grid[90]; //bottom left tile

	current->fcost = 0;
	current->hcost = 0;
	current->gcost = 0;

	open.push_back(current);

	while (!open.empty() && !current->tile != 9)
	{
		open.erase(std::find(open.begin(), open.end(), current));
		closed.push_back(current);

		if (std::floor(current->tile / 10) > 0)
		{
			node* up = nullptr;
			node* left = nullptr;
			node* right = nullptr;
			node* down = nullptr;
			for (auto& tile : grid)
			{

				if(std::find(closed.begin(), closed.end(), &tile) == closed.end() && !tile.busy)
				{ 
				if (tile.tile == (current->tile - 10) )
				{
					up = &tile;
					up->prev = current;
					up->gcost = 10 + current->gcost;
					up->hcost = std::abs(up->tile - 9);
					up->fcost = up->gcost + up->hcost;
					if (std::find(open.begin(), open.end(), &tile) == open.end())
					open.push_back(up);
				}

				else if (tile.tile == (current->tile + 10))
				{
					down = &tile;
					down->prev = current;
					down->gcost = 10 + current->gcost;
					down->hcost = std::abs(down->tile - 9);
					down->fcost = down->gcost + down->hcost;
					if (std::find(open.begin(), open.end(), &tile) == open.end())
					open.push_back(down);
				}
				else if (tile.tile == (current->tile + 1) && current->tile%10 < 9)
				{
					right = &tile;
					right->prev = current;
					right->gcost = 10 + current->gcost;
					right->hcost = std::abs( right->tile - 9);
					right->fcost = right->gcost + right->hcost;
					if (std::find(open.begin(), open.end(), &tile) == open.end())
					open.push_back(right);
				}
				else if (tile.tile == (current->tile - 1) && current->tile%10 > 0)
				{
					left = &tile;
					left->prev = current;
					left->gcost = 10 + current->gcost;
					left->hcost = std::abs( left->tile - 9);
					left->fcost = left->gcost + left->hcost;
					if (std::find(open.begin(), open.end(), &tile) == open.end())
						open.push_back(left);
				}
				}
			}
		}
			
		int currentValue = INT_MAX;
		for (auto& i : open)
		{
			if (i->fcost < currentValue)
			{
				current = i;
				currentValue = current->fcost;
			}}}

	std::vector<int> path;
	path.push_back(current->tile);
	while (current->prev != nullptr)
	{
		path.push_back(current->prev->tile);
		current = current->prev;
	}
	std::reverse(path.begin(), path.end());

	auto back = path.back();


	if (back != 9)
		path.clear();
	return path;
}

std::vector<std::vector<int>> CalculatePaths(chromosome* cm, int pathEndLength, const parameters& params)
{
	std::vector<std::vector<int>> foundPaths;
	//convert to allow availiability boolean

	chromosome tempMap = *cm;

	while (true)
	{
		std::vector<int> path = FindPath(&tempMap);
		std::vector<int> modified;
		if (!path.empty() && path.size() > 4)
		{
			foundPaths.push_back(path);
			for (int i = path.size()/4; i < path.size()-3; ++i)
				tempMap.genes[path[i]] = true;
		}
		else
			break;
	}

	return foundPaths;
}

void FitnessTest(chromosome* cm, const parameters& params)
{

	for (int i = 0; i < 5; i++) //Sets the top 5 right and bottom left genes to be empty
	{
		cm->genes[90 + i] = false;
		cm->genes[9 - i] = false;
	}

	int pathVal = 0;
	int objectVal = 0;
	auto path = CalculatePaths(cm, 10, params);
	int pathNum = path.size();
	cm->paths = path;




	for (auto gene : cm->genes)
		if (gene)
			++objectVal;



		if (pathNum >  params.maximumPathNumber)
		pathVal = params.maximumPathNumber * params.pathContributeValue;
		
	else
		pathVal = pathNum * params.pathContributeValue;
	

	int fitness = objectVal + pathVal;
	cm->fitnessValue = fitness;
}

void Mutate(chromosome* ch, double chance)
{
	for (int i = 0; i < 100; i++)
	{
		if (chance <= (double)(std::rand() % 100) )
		{
			ch->genes[i] = !ch->genes[i]; //swap genes byte value
		}
	}
	
}

void Cross(chromosome* first, chromosome* second, double chance)
{
	if (chance <= (double)(std::rand() % 100 + 1))
	{
		//create random value from 1-100 to decide crosspoint
		int crossPoint = std::rand() % 100 + 1;

		for(int i = 0; i < crossPoint; ++i)
			std::swap(first->genes[i], second->genes[1]);
	}
}

void FindNextPair(chromosome* first, chromosome* second, population& pop)
{
	population tempPop = pop;

	//std::sort(pop.begin(), pop.end());


	first = nullptr;
	second = nullptr;



	std::random_device rd;
	std::mt19937 gen(rd());
	std::discrete_distribution<> randomSelectionWeightedTable;
	for(auto cm: pop)
		randomSelectionWeightedTable.probabilities().push_back(cm.fitnessValue);
	first = &pop[randomSelectionWeightedTable(gen)] ;
	int it = randomSelectionWeightedTable(gen);
	while (&pop[it] != first)
	{
		it = randomSelectionWeightedTable(gen);
		


	} 
	second = &pop[it];

	/*int overallFitness = -5;
	for (auto& cm : pop)
	{
		overallFitness += cm.fitnessValue;
	}
	int rand = std::rand() % overallFitness;
	for (int i = 0; i < pop.size(); i++) {
		if (rand < pop[i].fitnessValue)
		{
			pop.push_back(pop[i]);
			first = &pop[i];
			break;
		}
		rand -= pop[i].fitnessValue;
	}
	rand = std::rand() % overallFitness;
	for (int i = 0; i < pop.size(); i++) {
		if (rand < pop[i].fitnessValue && &pop[i] != first)
		{
			pop.push_back(pop[i]);
			second = &pop[i];
			break;
		}
		rand -= pop[i].fitnessValue;
	}*/


	assert(first != nullptr && second !=nullptr);


}


void GenerateNewGeneration(population* pop, const parameters& params, chromosome* best)
{
	//select two random
	chromosome first;
	chromosome second;

	FindNextPair(&first, &second, *pop);
	Cross(&first, &second, params.crossChance);
	Mutate(&first, params.mutateChance);
	Mutate(&second, params.mutateChance);
	FitnessTest(&first, params);
	FitnessTest(&second, params);

	pop->push_back(first);
	pop->push_back(second);
}

void RunBest(const parameters params)
{
	population currentPopulation;
	chromosome first, second;


	for (int i = 0; i < 100; ++i)
	{
		first.genes[i] = false;
		second.genes[i] = false;
	}

	FitnessTest(&first, params);
	FitnessTest(&second, params);

	currentPopulation.push_back(first);
	currentPopulation.push_back(second);

	int itterationCount = 0;

	static chromosome* best = nullptr;
	int bestVal = 0;
	while (itterationCount < 100000)
	{
		best = nullptr;

		for (auto& cm : currentPopulation)
			if (best == nullptr || best->fitnessValue < cm.fitnessValue)
			{
				best = &cm;
				bestVal = best->fitnessValue;
			}
		if (bestVal > 100)
			break;
		GenerateNewGeneration(&currentPopulation, params, best);
		++itterationCount;
	}

	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; j++)
		{
			if (best->genes[i * 10 + j] == false) //availiable
			{
				SetConsoleTextAttribute(hConsole, 7);
			}
			else
			{
				SetConsoleTextAttribute(hConsole, 12);
			}


			if (i < 1)
				std::cout << 0;
			std::cout << i * 10 + j << " ";
		}
	
		std::cout << std::endl;
	}

	std::cout << std::endl;
	std::cout << "Fitness level was :" << best->fitnessValue << " and total itterations taken was: " << itterationCount << std::endl;











}





void RunTestParameters()
{

	parameters params;
	params.crossChance = 50;
	params.mutateChance = 30;
	params.pathContributeValue = 40;

	for (int i = 0; i < 5; i++)
	{
		RunBest(params);

		std::cout << std::endl << std::endl << std::endl;
	}

}


int main()
{
	std::srand(time(nullptr));
	
	RunTestParameters();

	system("pause");

	return 0;
}
