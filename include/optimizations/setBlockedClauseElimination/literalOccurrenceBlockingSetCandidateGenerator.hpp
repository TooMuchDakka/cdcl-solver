#ifndef LITERAL_OCCURRENCE_BLOCKING_SET_CANDIDATE_GENERATOR_HPP
#define LITERAL_OCCURRENCE_BLOCKING_SET_CANDIDATE_GENERATOR_HPP

#include "dimacs/literalOccurrenceLookup.hpp"
#include "dimacs/problemDefinition.hpp"
#include "optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp"

namespace setBlockedClauseElimination {
	class LiteralOccurrenceBlockingSetCandidateGenerator : public BaseBlockingSetCandidateGenerator {
	public:
		enum CandidateSelectionHeuristic
		{
			MinClauseOverlap,
			MaxClauseOverlap,
			RandomSelection
		};

		explicit LiteralOccurrenceBlockingSetCandidateGenerator(std::vector<long> clauseLiterals, const CandidateSelectionHeuristic candidateSelectionHeuristic, const dimacs::LiteralOccurrenceLookup* literalOccurrenceLookup)
			: BaseBlockingSetCandidateGenerator(std::move(clauseLiterals)), candidateSelectionHeuristic(candidateSelectionHeuristic)
		{
			if (!literalOccurrenceLookup && candidateSelectionHeuristic != CandidateSelectionHeuristic::RandomSelection)
				throw std::invalid_argument("None random candidate selection heuristic requires literal occurrence lookup to operate correctly!");

			orderLiteralsAccordingToHeuristic(*literalOccurrenceLookup);
			candidateLiteralIndices = { 0, 0 };
			requiredWrapAroundBeforeCandidateResize = { determineRequiredNumberOfWrapAroundsForIndex(0) };
		}

		[[nodiscard]] std::optional<BlockingSetCandidate> generateNextCandidate() override;
	protected:
		CandidateSelectionHeuristic candidateSelectionHeuristic;
		BaseBlockingSetCandidateGenerator::BlockingSetCandidate lastGeneratedCandidate;
		std::vector<std::size_t> candidateLiteralIndices;
		std::vector<std::size_t> requiredWrapAroundBeforeCandidateResize;

		void orderLiteralsAccordingToHeuristic(const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup);


		[[nodiscard]] std::size_t getLastIncrementableIndexForPosition(std::size_t indexInCandidateVector) const noexcept
		{
			return clauseLiterals.size() - indexInCandidateVector - 1;
		}

		[[nodiscard]] long getClauseLiteral(std::size_t clauseLiteralIdx) const
		{
			return clauseLiterals[candidateLiteralIndices[clauseLiteralIdx]];
		}

		[[nodiscard]] std::size_t determineRequiredNumberOfWrapAroundsForIndex(std::size_t clauseLiteralIdx) const noexcept
		{
			return this->clauseLiterals.size() - (2 + clauseLiteralIdx);
		}

		void incrementCandidateSize();
	};
}

#endif