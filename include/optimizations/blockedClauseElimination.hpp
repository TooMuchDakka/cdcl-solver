#ifndef BLOCKED_CLAUSE_ELIMINATION_HPP
#define BLOCKED_CLAUSE_ELIMINATION_HPP
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dimacs/problemDefinition.hpp"

namespace blockedClauseElimination
{
	class BaseBlockedClauseEliminator
	{
	public:
		using Formula = std::vector<dimacs::ProblemDefinition::Clause::constPtr>;
		virtual ~BaseBlockedClauseEliminator() {}

		explicit BaseBlockedClauseEliminator(std::shared_ptr<const Formula> formula)
			: formula(std::move(formula)) {}

		[[nodiscard]] virtual std::vector<std::size_t> determineCandidatesBasedOnHeuristic() const;
		[[nodiscard]] virtual std::optional<bool> isClauseBlocked(std::size_t idxOfClauseToCheck) const;
	protected:
		using SetOfExternalClauseLiterals = std::unordered_set<std::size_t>;
		using SetOfExternalClauseLiteralsReference = std::shared_ptr<SetOfExternalClauseLiterals>;

		std::vector<SetOfExternalClauseLiteralsReference> externalLiteralsPerClauseLookup;
		std::shared_ptr<const Formula> formula;
		
		[[nodiscard]] std::optional<SetOfExternalClauseLiteralsReference> getExternalLiteralsOfClause(std::size_t clauseIdxInFormula) const;
		[[nodiscard]] bool isClauseBlocked(std::size_t idxOfClauseToCheck, const SetOfExternalClauseLiterals& externalLiteralsOfClause) const;
		[[nodiscard]] static std::unordered_map<std::size_t, SetOfExternalClauseLiteralsReference> determineExternalLiteralsPerClauseOfFormula();
	};
};
#endif