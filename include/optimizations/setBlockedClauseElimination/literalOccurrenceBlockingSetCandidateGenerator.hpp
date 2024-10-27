#ifndef LITERAL_OCCURRENCE_BLOCKING_SET_CANDIDATE_GENERATOR_HPP
#define LITERAL_OCCURRENCE_BLOCKING_SET_CANDIDATE_GENERATOR_HPP

#include <random>

#include "dimacs/literalOccurrenceLookup.hpp"
#include "dimacs/problemDefinition.hpp"
#include "optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp"

namespace setBlockedClauseElimination {
	class LiteralOccurrenceBlockingSetCandidateGenerator : public BaseBlockingSetCandidateGenerator {
	public:
		using ptr = std::unique_ptr<LiteralOccurrenceBlockingSetCandidateGenerator>;

		explicit LiteralOccurrenceBlockingSetCandidateGenerator(std::vector<long> clauseLiterals)
			: BaseBlockingSetCandidateGenerator(std::move(clauseLiterals))
		{
			if (!this->clauseLiterals.empty())
			{
				candidateLiteralIndices = { 0, 0 };
				lastGeneratedCandidate = { getClauseLiteral(0) };
				requiredWrapAroundBeforeCandidateResize = { determineRequiredNumberOfWrapAroundsForIndex(0) };
			}
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingSequentialLiteralSelection(std::vector<long> clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			filterNoneOverlappingLiteralsFromClause(clauseLiterals, literalOccurrenceLookup);
			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(clauseLiterals);
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingRandomLiteralSelection(std::vector<long> clauseLiterals, unsigned int rngSeed, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			filterNoneOverlappingLiteralsFromClause(clauseLiterals, literalOccurrenceLookup);
			if (!clauseLiterals.empty())
			{
				auto rng = std::default_random_engine{};
				rng.seed(rngSeed);

				std::shuffle(clauseLiterals.begin(), clauseLiterals.end(), rng);
			}
			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(clauseLiterals);
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingMinimumClauseOverlapForLiteralSelection(std::vector<long> clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			filterNoneOverlappingLiteralsFromClause(clauseLiterals, literalOccurrenceLookup);
			if (!clauseLiterals.empty())
				orderLiteralsAccordingToHeuristic(clauseLiterals, literalOccurrenceLookup, true);

			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(clauseLiterals);
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingMaximumClauseOverlapForLiteralSelection(std::vector<long> clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			filterNoneOverlappingLiteralsFromClause(clauseLiterals, literalOccurrenceLookup);
			if (!clauseLiterals.empty())
				orderLiteralsAccordingToHeuristic(clauseLiterals, literalOccurrenceLookup, false);

			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(clauseLiterals);
		}

		[[nodiscard]] std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> generateNextCandidate() override;

	protected:
		BaseBlockingSetCandidateGenerator::BlockingSetCandidate lastGeneratedCandidate;
		std::vector<std::size_t> candidateLiteralIndices;
		std::vector<std::size_t> requiredWrapAroundBeforeCandidateResize;

		static void orderLiteralsAccordingToHeuristic(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, bool orderAscendingly);

		[[nodiscard]] std::size_t getLastIncrementableIndexForPosition(std::size_t indexInCandidateVector) const noexcept
		{
			return (clauseLiterals.size() - candidateLiteralIndices.size()) + indexInCandidateVector;
		}

		[[nodiscard]] long getClauseLiteral(const std::size_t clauseLiteralIdx) const
		{
			return clauseLiterals[candidateLiteralIndices[clauseLiteralIdx]];
		}

		[[nodiscard]] std::size_t determineRequiredNumberOfWrapAroundsForIndex(const std::size_t clauseLiteralIdx) const noexcept
		{
			return (clauseLiterals.size() - (candidateLiteralIndices.size() + clauseLiteralIdx)) + 1;
		}

		[[nodiscard]] bool canGenerateMoreCandidates() const noexcept
		{
			return candidateLiteralIndices.size() <= clauseLiterals.size();
		}

		static void filterNoneOverlappingLiteralsFromClause(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			clauseLiterals.erase(
				std::remove_if(
					clauseLiterals.begin(),
					clauseLiterals.end(),
					[&literalOccurrenceLookup](const long literal) { return literalOccurrenceLookup.getNumberOfOccurrencesOfLiteral(-literal) == 0; }),
				clauseLiterals.end()
			);
		}

		void incrementCandidateSize();
	};
}

#endif