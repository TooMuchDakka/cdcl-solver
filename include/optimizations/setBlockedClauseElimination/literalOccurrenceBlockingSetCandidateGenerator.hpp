#ifndef LITERAL_OCCURRENCE_BLOCKING_SET_CANDIDATE_GENERATOR_HPP
#define LITERAL_OCCURRENCE_BLOCKING_SET_CANDIDATE_GENERATOR_HPP

#include <random>

#include "dimacs/literalOccurrenceLookup.hpp"
#include "dimacs/problemDefinition.hpp"
#include "optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp"

namespace setBlockedClauseElimination {
	class LiteralOccurrenceBlockingSetCandidateGenerator : public BaseBlockingSetCandidateGenerator {
	public:
		using BaseBlockingSetCandidateGenerator::init;
		enum LiteralClauseOverlapCountSortOrder
		{
			Ascending,
			Descending
		};

		explicit LiteralOccurrenceBlockingSetCandidateGenerator()
			: literalSelectionHeuristic(LiteralSelectionHeuristic::Sequential), optionalRng(std::nullopt), optionalSortOrder(std::nullopt) {}

		explicit LiteralOccurrenceBlockingSetCandidateGenerator(unsigned int rngSeed)
			: LiteralOccurrenceBlockingSetCandidateGenerator()
		{
			literalSelectionHeuristic = LiteralSelectionHeuristic::Random;
			optionalRng = std::default_random_engine{};
			optionalRng->seed(rngSeed);
		}

		explicit LiteralOccurrenceBlockingSetCandidateGenerator(const LiteralClauseOverlapCountSortOrder literalClauseOverlapCountSortOrder)
			: LiteralOccurrenceBlockingSetCandidateGenerator()
		{
			literalSelectionHeuristic = literalClauseOverlapCountSortOrder == LiteralClauseOverlapCountSortOrder::Ascending ? LiteralSelectionHeuristic::MinimalClauseOverlap : LiteralSelectionHeuristic::MaximumClauseOverlap;
			optionalSortOrder = literalClauseOverlapCountSortOrder;
		}

		using ptr = std::unique_ptr<LiteralOccurrenceBlockingSetCandidateGenerator>;

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingRandomLiteralSelectionHeuristic(unsigned int rngSeed)
		{
			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(rngSeed);
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingSequentialLiteralSelectionHeuristic()
		{
			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>();
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingMinimumClauseOverlapForLiteralSelection()
		{
			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(LiteralClauseOverlapCountSortOrder::Ascending);
		}

		[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr usingMaximumClauseOverlapForLiteralSelection()
		{
			return std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(LiteralClauseOverlapCountSortOrder::Descending);
		}

		[[nodiscard]] std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> generateNextCandidate() override;
		void init(std::vector<long> candidateClauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction) override;

	protected:
		constexpr static std::size_t INITIAL_INDEX_VALUE = SIZE_MAX;
		enum LiteralSelectionHeuristic
		{
			Sequential,
			Random,
			MinimalClauseOverlap,
			MaximumClauseOverlap
		};

		LiteralSelectionHeuristic literalSelectionHeuristic;
		std::optional<std::default_random_engine> optionalRng;
		std::optional<LiteralClauseOverlapCountSortOrder> optionalSortOrder;

		BaseBlockingSetCandidateGenerator::BlockingSetCandidate lastGeneratedCandidate;
		std::vector<std::size_t> candidateLiteralIndices;
		std::vector<std::size_t> requiredWrapAroundBeforeCandidateResize;
		bool userDefinedMinimumSizeMatchesMaximumPossibleSizeOneTimeFlag;
		bool usingNoneDefaultInitialCandidateSizeOneTimeFlag;

		[[nodiscard]] std::size_t getLastIncrementableIndexForPosition(std::size_t indexInCandidateVector) const noexcept
		{
			return (clauseLiterals.size() - candidateLiteralIndices.size()) + indexInCandidateVector;
		}

		[[nodiscard]] long getClauseLiteral(const std::size_t clauseLiteralIdx) const
		{
			return clauseLiterals[candidateLiteralIndices[clauseLiteralIdx]];
		}

		[[nodiscard]] std::size_t determineRequiredNumberOfWrapAroundsForIndex(std::size_t indexInCandidateVector) const noexcept
		{
			const std::size_t indexInClauseLiterals = candidateLiteralIndices[indexInCandidateVector];
			std::size_t sizeOfRemainingCandiate = candidateLiteralIndices.size() - indexInCandidateVector;
			return (clauseLiterals.size() - indexInClauseLiterals) - sizeOfRemainingCandiate;
		}

		[[nodiscard]] bool canGenerateMoreCandidates() const noexcept
		{
			return candidateLiteralIndices.size() <= candidateSizeRestriction.maxAllowedSize;
		}

		[[nodiscard]] bool handleCandidateGenerationOfSizeOne();

		void incrementCandidateSize();
		void setInternalInitialStateAfterCandidateResize();
		static void filterNoneOverlappingLiteralsFromClause(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup);
		static void orderLiteralsAccordingToHeuristic(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, bool orderAscendingly);
		[[nodiscard]] static bool getAndResetOneTimeFlagValueIfSet(bool& oneTimeFlagValue)
		{
			if (oneTimeFlagValue)
			{
				oneTimeFlagValue = false;
				return true;
			}
			return false;
		}
	};
}

#endif