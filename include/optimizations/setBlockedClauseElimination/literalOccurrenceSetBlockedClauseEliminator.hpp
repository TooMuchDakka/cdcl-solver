#ifndef CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <stdexcept>

#include "baseSetBlockedClauseEliminator.hpp"
#include <dimacs/literalOccurrenceLookup.hpp>

namespace setBlockedClauseElimination {
	class LiteralOccurrenceSetBlockedClauseEliminator : public BaseSetBlockedClauseEliminator {
	public:
		LiteralOccurrenceSetBlockedClauseEliminator() = delete;
		explicit LiteralOccurrenceSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition, std::unique_ptr<BaseSetBlockedClauseEliminator> blockingSetCandidateGenerator)
			: BaseSetBlockedClauseEliminator(std::move(problemDefinition), std::move(blockingSetCandidateGenerator))
		{
			if (!problemDefinition)
				throw std::invalid_argument("Problem definition cannot be null");

			literalOccurrenceLookup = std::make_unique<dimacs::LiteralOccurrenceLookup>(*this->problemDefinition);
		}

		[[nodiscard]] std::optional<std::vector<long>> determineBlockingSet(std::size_t clauseIdxInFormula) const override;
		void ignoreSetBlockedClause(std::size_t clauseIdxInFormula) override;
		
	protected:
		std::unique_ptr<dimacs::LiteralOccurrenceLookup> literalOccurrenceLookup;

	};
}
#endif