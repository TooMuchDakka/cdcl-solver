#ifndef LOCAL_CLAUSE_LITERAL_REMOVER_HPP
#define LOCAL_CLAUSE_LITERAL_REMOVER_HPP
#include "problemDefinition.hpp"

namespace localClauseLiteralRemoval
{
	class LocalClauseLiteralRemover
	{
	public:
		LocalClauseLiteralRemover(std::size_t numLiterals) : perLiteralUsageLookup(initializeLiteralUsageLookup(numLiterals + 1)) {}

		void recordLiteralUsageInClause(long literal, std::size_t clauseIdx);
		[[maybe_unused]] std::vector<long> removeLocalLiteralsFromClause(dimacs::ProblemDefinition::Clause& clause) const;
		void manuallyMarkVariableAsRemoved(long literal);
	protected:
		struct LiteralUsageRecord
		{
			enum UsageState
			{
				None,
				Once,
				MoreThanOnce,
				MarkedForRemoval
			};
			std::size_t lastUsedInClauseIdx;
			UsageState usageState;
			
			LiteralUsageRecord() : lastUsedInClauseIdx(0), usageState(UsageState::None) {}
			void recordUsageInClause(std::size_t clauseIdx);
		};

		std::vector<LiteralUsageRecord> perLiteralUsageLookup;

		[[nodiscard]] static std::vector<LiteralUsageRecord> initializeLiteralUsageLookup(std::size_t numLiterals);
	};
}
#endif