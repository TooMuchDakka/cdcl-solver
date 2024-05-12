#include "localClauseLiteralRemover.hpp"

using namespace localClauseLiteralRemoval;
void LocalClauseLiteralRemover::recordLiteralUsageInClause(long literal, std::size_t clauseIdx)
{
	if (!literal)
		return;
	perLiteralUsageLookup.at(std::abs(literal)).recordUsageInClause(clauseIdx);
}

std::vector<long> LocalClauseLiteralRemover::removeLocalLiteralsFromClause(dimacs::ProblemDefinition::Clause& clause) const
{
	std::vector<long> removedLiterals;
	for (auto clauseLiteralIterator = clause.literals.begin(); clauseLiteralIterator != clause.literals.end();)
	{
		if (perLiteralUsageLookup.at(std::abs(*clauseLiteralIterator)).usageState != LiteralUsageRecord::UsageState::MoreThanOnce)
		{
			removedLiterals.emplace_back(*clauseLiteralIterator);
			clauseLiteralIterator = clause.literals.erase(clauseLiteralIterator);
		}
		else
		{
			++clauseLiteralIterator;
		}
	}
	return removedLiterals;
}

void LocalClauseLiteralRemover::manuallyMarkVariableAsRemoved(long literal)
{
	perLiteralUsageLookup.at(std::abs(literal)).usageState = LiteralUsageRecord::MarkedForRemoval;
}

// BEING OF NON-PUBLIC FUNCTIONALITY
std::vector<LocalClauseLiteralRemover::LiteralUsageRecord> LocalClauseLiteralRemover::initializeLiteralUsageLookup(std::size_t numLiterals)
{
	return { numLiterals + 1, LocalClauseLiteralRemover::LiteralUsageRecord() };
}

void LocalClauseLiteralRemover::LiteralUsageRecord::recordUsageInClause(std::size_t clauseIdx)
{
	if (usageState == UsageState::MoreThanOnce || usageState == UsageState::MarkedForRemoval)
		return;

	if (usageState == UsageState::None)
	{
		lastUsedInClauseIdx = clauseIdx;
		usageState = UsageState::Once;
		return;
	}

	if (usageState == UsageState::Once)
	{
		if (lastUsedInClauseIdx == clauseIdx)
			return;

		lastUsedInClauseIdx = clauseIdx;
		usageState = UsageState::MoreThanOnce;
	}
}