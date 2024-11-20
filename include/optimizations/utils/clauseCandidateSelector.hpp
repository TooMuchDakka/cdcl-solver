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

		explicit ClauseCandidateSelector(const std::size_t numCandidates, const CandidateSelectionHeuristic candidateSelectionHeuristic, std::optional<unsigned int> rngSeed)
			: candidateSelectionHeuristic(candidateSelectionHeuristic), lastChosenCandidateIndexInQueue(0) {
			if (candidateSelectionHeuristic != CandidateSelectionHeuristic::Random && rngSeed.has_value())
				throw std::invalid_argument("Rng seed can only be set when random candidate selection heuristic was chosen");

			candidateClauseIndexQueue.resize(numCandidates);
			for (std::size_t i = 1; i < candidateClauseIndexQueue.size(); ++i)
				candidateClauseIndexQueue[i] = i;

			if (candidateSelectionHeuristic == CandidateSelectionHeuristic::Random)
			{
				auto optionalRng = std::default_random_engine{};
				optionalRng.seed(*rngSeed);

				std::shuffle(candidateClauseIndexQueue.begin(), candidateClauseIndexQueue.end(), optionalRng);
			}
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingSequentialCandidateSelection(const std::size_t numCandidates) {
			return std::make_unique<ClauseCandidateSelector>(numCandidates, CandidateSelectionHeuristic::Sequential, std::nullopt);
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingRandomCandidateSelection(const std::size_t numCandidates, const unsigned int rngSeed) {
			return std::make_unique<ClauseCandidateSelector>(numCandidates, CandidateSelectionHeuristic::Random, rngSeed);
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMinimalClauseOverlapForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MinimumClauseOverlap, std::nullopt); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMaximalClauseOverlapForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MaximumClauseOverlap, std::nullopt); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMinimumClauseLenghtForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition)
		{
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MinimumClauseLength, std::nullopt); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMaximumClauseLengthForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition)
		{
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MaximumClauseLength, std::nullopt); instance)
			{
				instance->initializeCandidateSequence(problemDefinition);
				return instance;
			}
			return nullptr;
		}

		void initializeCandidateSequence(const dimacs::ProblemDefinition& problemDefinition);
		[[nodiscard]] std::optional<std::size_t> selectNextCandidate();
	protected:
		CandidateSelectionHeuristic candidateSelectionHeuristic;
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
	};
}

#endif