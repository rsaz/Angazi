#pragma once

#include "Config.h"
#include "Genome.h"
#include "InnovationContainer.h"
#include "Species.h"

namespace Angazi::AI::NEAT
{
	class Population
	{
	public:
		Population(size_t input, size_t output, size_t bias = 1);

		void NewGeneration();

		size_t Generation() const { return mGenerationNumber; }

		MutationConfig mutationConfig;
		SpeciatingConfig speciatingConfig;
		NeuralNetConfig neuralNetConfig;

		std::list<Species> species;

	private:
		Genome MakeFirstGenome();

		// Evolutionary methods
		Genome Crossover(const Genome& g1, const Genome& g2);
		void MutateWeight(Genome& g);
		void MutateEnableDisable(Genome& g, bool enable);
		void MutateLink(Genome& g, bool force_bias);
		void MutateNode(Genome& g);
		void Mutate(Genome& g);

		double Disjoint(const Genome& g1, const Genome& g2);
		double Weights(const Genome& g1, const Genome& g2);
		bool IsSameSpecies(const Genome& g1, const Genome& g2);

		// Species Ranking
		void RankGlobally();
		void CalculateAverageFitness(Species& s);
		size_t TotalAverageFitness()const;

		// Evolution
		void CullSpecies(bool cut_to_one);
		Genome BreedChild(Species& s);
		void RemoveStaleSpecies();
		void RemoveWeakSpecies();
		void AddToSpecies(const Genome& child);

	private:
		InnovationContainer mInnovation;

		size_t mGenerationNumber = 1;
	};
}