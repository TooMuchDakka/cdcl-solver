#ifndef BLOCKED_CLAUSE_ELIMINATION_HPP
#define BLOCKED_CLAUSE_ELIMINATION_HPP

#include <optional>
#include <vector>

#include "dimacs/problemDefinition.hpp"

namespace blockedClauseElimination
{
	class BaseBlockedClauseEliminator
	{
	public:
		virtual ~BaseBlockedClauseEliminator() = default;

		explicit BaseBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: problemDefinition(std::move(problemDefinition)) {}

		[[nodiscard]] virtual std::vector<std::size_t> determineCandidatesBasedOnHeuristic() const;
		[[nodiscard]] virtual bool canCandidateBeSelectedBasedOnHeuristic(std::size_t potentialCandidateClauseIdxInFormula) const;
		[[nodiscard]] virtual std::optional<bool> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) const;
	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;

		[[nodiscard]] bool isClauseLiteralBlocked(std::size_t idxOfClauseToCheckInFormula, long literal) const;
	};
};
#endif