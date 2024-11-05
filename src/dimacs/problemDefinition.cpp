#include "dimacs/problemDefinition.hpp"

using namespace dimacs;

[[maybe_unused]] bool ProblemDefinition::addClause(std::size_t index, Clause clause)
{
	if (index >= clauses->size() || !literalOccurrenceLookup.recordClauseLiteralOccurrences(index, clause.literals))
		return false;

	clauses->at(index) = std::move(clause);
	return true;
}

[[nodiscard]] std::shared_ptr<std::vector<ProblemDefinition::Clause>> ProblemDefinition::getClauses() const
{
	return clauses;
}

[[nodiscard]] std::optional<const dimacs::ProblemDefinition::Clause*> ProblemDefinition::getClauseByIndexInFormula(std::size_t idxOfClauseInFormula) const
{
	if (idxOfClauseInFormula >= clauses->size())
		return std::nullopt;

	return &clauses->at(idxOfClauseInFormula);
}

[[nodiscard]] std::optional<ProblemDefinition::Clause*> ProblemDefinition::getClauseByIndexInFormula(std::size_t idxOfClauseInFormula)
{
	if (idxOfClauseInFormula >= clauses->size())
		return std::nullopt;

	return &clauses->at(idxOfClauseInFormula);
}

[[nodiscard]] std::size_t ProblemDefinition::getNumVariablesInFormula() const
{
	return nVariables;
}

[[nodiscard]] std::size_t ProblemDefinition::getNumClauses() const
{
	return clauses->size();
}

[[nodiscard]] const LiteralOccurrenceLookup& ProblemDefinition::getLiteralOccurrenceLookup() const
{
	return literalOccurrenceLookup;
}

std::optional<std::vector<long>> ProblemDefinition::getClauseLiteralsOmittingAlreadyAssignedOnes(std::size_t idxOfClauseInFormula) const
{
	const std::optional<const dimacs::ProblemDefinition::Clause*> dataOfAccessedClause = getClauseByIndexInFormula(idxOfClauseInFormula);
	if (!dataOfAccessedClause.has_value())
		return std::nullopt;

	std::vector<long> unassignedClauseLiterals;
	for (const long literal : dataOfAccessedClause.value()->literals)
	{
		if (variableValueLookup.getLiteralValue(literal).value_or(VariableValueLookup::Unknown) == VariableValueLookup::Unknown)
			unassignedClauseLiterals.emplace_back(literal);
	}
	return unassignedClauseLiterals;
}

[[maybe_unused]] ProblemDefinition::PropagationResult ProblemDefinition::propagate(long literal)
{
	if (variableValueLookup.getLiteralValue(literal).has_value())
		return PropagationResult::Ok;

	const std::optional<std::vector<std::size_t>>& indiciesOfClausesContainingNegatedLiteral = literalOccurrenceLookup[-literal];
	auto propagationResult = indiciesOfClausesContainingNegatedLiteral.has_value() ? PropagationResult::Ok : PropagationResult::ErrorDuringPropagation;

	for (std::size_t i = 0; propagationResult == PropagationResult::Ok && i < indiciesOfClausesContainingNegatedLiteral->size(); ++i)
	{
		const std::optional<std::vector<long>>& unassignedLiteralsOfClause = getClauseLiteralsOmittingAlreadyAssignedOnes(indiciesOfClausesContainingNegatedLiteral->at(i));
		if (!unassignedLiteralsOfClause.has_value())
			propagationResult = PropagationResult::ErrorDuringPropagation;
		else if (unassignedLiteralsOfClause->size() == 1)
			propagationResult = PropagationResult::Conflict;
	}

	if (!variableValueLookup.recordSatisfyingLiteralAssignment(literal))
		return PropagationResult::ErrorDuringPropagation;

	const std::optional<std::vector<std::size_t>>& indiciesOfClausesContainingLiteral = literalOccurrenceLookup[literal];
	propagationResult = indiciesOfClausesContainingLiteral.has_value() ? PropagationResult::Ok : PropagationResult::ErrorDuringPropagation;

	for (std::size_t i = 0; propagationResult == PropagationResult::Ok && i < indiciesOfClausesContainingLiteral->size(); ++i)
	{
		const std::optional<std::vector<long>>& unassignedLiteralsOfClause = getClauseLiteralsOmittingAlreadyAssignedOnes(indiciesOfClausesContainingLiteral->at(i));
		if (!unassignedLiteralsOfClause.has_value())
			propagationResult = PropagationResult::ErrorDuringPropagation;
		else if (unassignedLiteralsOfClause->size() == 1)
			propagationResult = propagate(unassignedLiteralsOfClause->front());
	}
	return propagationResult;
}