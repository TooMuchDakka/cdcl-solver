#ifndef BLOCKING_LITERAL_GENERATOR_HPP
#define BLOCKING_LITERAL_GENERATOR_HPP

#include <dimacs/literalOccurrenceLookup.hpp>

#include <optional>
#include <random>
#include <vector>

namespace blockedClauseElimination
{
	class BlockingLiteralGenerator {
	public:
		using ptr = std::unique_ptr<BlockingLiteralGenerator>;
		enum BlockingLiteralSelectionHeuristic
		{
			Sequential,
			Random,
			MinClauseOverlap,
			MaxClauseOverlap
		};

		BlockingLiteralGenerator(BlockingLiteralSelectionHeuristic candidateSelectionHeuristic, std::optional<unsigned int> rngSeed)
			: candidateSelectionHeuristic(candidateSelectionHeuristic), lastSelectedCandidateIndex(0)
		{
			if (candidateSelectionHeuristic == BlockingLiteralSelectionHeuristic::Random)
			{
				optionalRng = std::default_random_engine{};
				if (rngSeed.has_value())
					optionalRng->seed(*rngSeed);
			}
		}

		[[nodiscard]] static BlockingLiteralGenerator::ptr usingRandomLiteralSelectionHeuristic(unsigned int rngSeed)
		{
			return std::make_unique<BlockingLiteralGenerator>(BlockingLiteralSelectionHeuristic::Random, rngSeed);
		}

		[[nodiscard]] static BlockingLiteralGenerator::ptr usingSequentialLiteralSelectionHeuristic()
		{
			return std::make_unique<BlockingLiteralGenerator>(BlockingLiteralSelectionHeuristic::Sequential, std::nullopt);
		}

		[[nodiscard]] static BlockingLiteralGenerator::ptr usingMinimumClauseOverlapForLiteralSelection()
		{
			return std::make_unique<BlockingLiteralGenerator>(BlockingLiteralSelectionHeuristic::MinClauseOverlap, std::nullopt);
		}

		[[nodiscard]] static BlockingLiteralGenerator::ptr usingMaximumClauseOverlapForLiteralSelection()
		{
			return std::make_unique<BlockingLiteralGenerator>(BlockingLiteralSelectionHeuristic::MaxClauseOverlap, std::nullopt);
		}

		void init(std::vector<long> literals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup);
		[[nodiscard]] std::optional<long> getNextCandiate();

	protected:
		static void orderLiteralsAccordingToClauseOverlap(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, bool orderAscendingly);

		BlockingLiteralSelectionHeuristic candidateSelectionHeuristic;
		std::size_t lastSelectedCandidateIndex;
		std::vector<long> candidateLiterals;
		std::optional<std::default_random_engine> optionalRng;
		
	};
}

#endif