#ifndef CLAUSE_CANDIDATE_SELECTOR_HPP
#define CLAUSE_CANDIDATE_SELECTOR_HPP

#include <dimacs/literalOccurrenceLookup.hpp>
#include <dimacs/problemDefinition.hpp>

#include <optional>
#include <random>
#include <string>

namespace clauseCandidateSelection {
	class ClauseCandidateSelector {
	public:
		using ptr = std::unique_ptr<ClauseCandidateSelector>;

		enum CandidateSelectionHeuristic {
			Random,
			Sequential,
			MinimumClauseOverlap,
			MaximumClauseOverlap
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

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMinimalClauseOverlapForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MinimumClauseOverlap, std::nullopt); instance)
			{
				instance->initializeCandidateSequence(problemDefinition, literalOccurrenceLookup);
				return instance;
			}
			return nullptr;
		}

		[[nodiscard]] static ClauseCandidateSelector::ptr initUsingMaximalClauseOverlapForCandidateSelection(const dimacs::ProblemDefinition& problemDefinition, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup) {
			if (auto instance = std::make_unique<ClauseCandidateSelector>(problemDefinition.getNumDeclaredClausesOfFormula(), CandidateSelectionHeuristic::MaximumClauseOverlap, std::nullopt); instance)
			{
				instance->initializeCandidateSequence(problemDefinition, literalOccurrenceLookup);
				return instance;
			}
			return nullptr;
		}

		void initializeCandidateSequence(const dimacs::ProblemDefinition& problemDefinition, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup);
		[[nodiscard]] std::optional<std::size_t> selectNextCandidate();
	protected:
		CandidateSelectionHeuristic candidateSelectionHeuristic;
		std::vector<std::size_t> candidateClauseIndexQueue;
		std::size_t lastChosenCandidateIndexInQueue;

		[[nodiscard]] static std::optional<std::size_t> determineNumberOfOverlapsBetweenClauses(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup);
	};
}

#endif