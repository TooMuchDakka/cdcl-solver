#ifndef CLAUSE_CANDIDATE_SELECTOR_HPP
#define CLAUSE_CANDIDATE_SELECTOR_HPP

#include <dimacs/problemDefinition.hpp>

#include <optional>
#include <random>

namespace clauseCandidateSelection {
	class ClauseCandidateSelector {
	public:
		using ptr = std::unique_ptr<ClauseCandidateSelector>;

		enum CandidateSelectionHeuristic {
			Random,
			Sequential,
			MinimumClauseOverlap,
			MaximumClauseOverlap,
			MinimumClauseLength,
			MaximumClauseLength
		};

		struct ClauseLengthRestriction
		{
			std::size_t maxAllowedClauseLength;
		};

		explicit ClauseCandidateSelector(const std::size_t numCandidates, const CandidateSelectionHeuristic candidateSelectionHeuristic, std::optional<unsigned int> rngSeed, const std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction)
			: numUserRequestedCandiates(numCandidates), numGeneratableCandidates(0), candidateSelectionHeuristic(candidateSelectionHeuristic), optionalClauseLengthRestriction(optionalClauseLengthRestriction), lastChosenCandidateIndexInQueue(0) {
			if (candidateSelectionHeuristic != CandidateSelectionHeuristic::Random && rngSeed.has_value())
				throw std::invalid_argument("Rng seed can only be set when random candidate selection heuristic was chosen");

			if (candidateSelectionHeuristic == CandidateSelectionHeuristic::Random)
			{
				optionalRngEngine = std::default_random_engine{};
				if (rngSeed.has_value())
					optionalRngEngine->seed(*rngSeed);
			}
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingSequentialCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const std::size_t numCandidates, const std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(numCandidates, CandidateSelectionHeuristic::Sequential, std::nullopt, optionalClauseLengthRestriction))
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingRandomCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const std::size_t numCandidates, const unsigned int rngSeed, std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(numCandidates, CandidateSelectionHeuristic::Random, rngSeed, optionalClauseLengthRestriction))
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMinimalClauseOverlapForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MinimumClauseOverlap, std::nullopt, optionalClauseLengthRestriction); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMaximalClauseOverlapForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MaximumClauseOverlap, std::nullopt, optionalClauseLengthRestriction); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMinimumClauseLenghtForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction)
		{
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MinimumClauseLength, std::nullopt, optionalClauseLengthRestriction); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMaximumClauseLengthForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction)
		{
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MaximumClauseLength, std::nullopt, optionalClauseLengthRestriction); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		void initializeCandidateSequence(const dimacs::ProblemDefinition& problemDefinition);
		[[nodiscard]] std::optional<std::size_t> selectNextCandidate();
		[[nodiscard]] std::size_t getNumGeneratableCandidates() const noexcept;
	protected:
		std::size_t numUserRequestedCandiates;
		std::size_t numGeneratableCandidates;
		CandidateSelectionHeuristic candidateSelectionHeuristic;
		std::optional<std::default_random_engine> optionalRngEngine;
		std::optional<ClauseLengthRestriction> optionalClauseLengthRestriction;
		std::vector<std::size_t> candidateClauseIndexQueue;
		std::size_t lastChosenCandidateIndexInQueue;

		[[nodiscard]] static std::optional<std::size_t> determineNumberOfOverlapsBetweenClauses(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition);
		[[nodiscard]] static std::optional<std::size_t> determineLengthOfClause(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition);
		[[nodiscard]] static bool determineOrderingOfElementAccordingToHeuristic(std::size_t lElementIndex, std::size_t lElementHeuristicValue, std::size_t rElementIndex, std::size_t rElementHeuristicValue, bool sortedDescendingly)
		{
			if (lElementHeuristicValue == rElementHeuristicValue)
				return lElementIndex < rElementIndex;

			return sortedDescendingly ? (lElementHeuristicValue > rElementHeuristicValue) : (lElementHeuristicValue < rElementHeuristicValue);
		}
		[[nodiscard]] static std::unordered_map<std::size_t, std::size_t> buildOverlapCacheForClauses(const dimacs::ProblemDefinition& problemDefinition, bool usingMaxOverlapAsSelectionHeuristic);
		[[nodiscard]] static std::unordered_map<std::size_t, std::size_t> buildLengthCacheForClauses(const dimacs::ProblemDefinition& problemDefinition, bool usingMaxLengthAsSelectionHeuristic);
		[[nodiscard]] static std::optional<std::vector<std::size_t>> generateIndexSequence(std::size_t numElements, const dimacs::ProblemDefinition& problemDefinition, const std::optional<ClauseLengthRestriction>& optionalClauseLengthRestriction);
		static void filterClausesNotMatchingLengthRestriction(const dimacs::ProblemDefinition& problemDefinition, std::vector<std::size_t>& clauseIndices, ClauseLengthRestriction clauseLengthRestriction);
	};
}

#endif