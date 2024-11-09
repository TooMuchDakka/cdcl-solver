#include "dimacs/problemDefinition.hpp"
#include <sstream>

using namespace dimacs;

[[maybe_unused]] bool ProblemDefinition::addClause(std::size_t index, Clause clause)
{
	if (clauses->count(index) || !literalOccurrenceLookup.recordClauseLiteralOccurrences(index, clause.literals))
		return false;

	clauses->emplace(index, std::move(clause));
	return true;
}

bool ProblemDefinition::removeClause(std::size_t index)
{
	const Clause* accessedClause = getClauseByIndexInFormula(index);
	if (!accessedClause)
		return false;

	for (const long literal : accessedClause->literals)
		literalOccurrenceLookup.removeLiteralFromClause(index, literal);

	if (clauses->count(index))
	{
		clauses->erase(index);
		return true;
	}
	return false;
}

bool ProblemDefinition::removeLiteralFromClausesOfFormula(long literal)
{
	const std::optional<std::vector<std::size_t>> indicesOfClausesContainingLiteral = literalOccurrenceLookup[literal];
	if (!indicesOfClausesContainingLiteral.has_value())
		return false;

	return std::all_of(
		indicesOfClausesContainingLiteral->cbegin(),
		indicesOfClausesContainingLiteral->cend(),
		[&](const std::size_t clauseIndex)
		{
			Clause* accessedClause = getClauseByIndexInFormula(clauseIndex);
			if (!accessedClause)
				return false;

			std::vector<long>& clauseLiterals = accessedClause->literals;
			clauseLiterals.erase(
				std::remove_if(clauseLiterals.begin(), clauseLiterals.end(), 
					[literal](long clauseLiteral) { return clauseLiteral == literal; }),
				clauseLiterals.end()
			);
			literalOccurrenceLookup.removeLiteralFromClause(clauseIndex, literal);
			return true;
		}
	);
}

[[nodiscard]] const ProblemDefinition::Clause* ProblemDefinition::getClauseByIndexInFormula(std::size_t idxOfClauseInFormula) const
{
	const auto& elementMatchingKey = clauses->find(idxOfClauseInFormula);
	return elementMatchingKey != clauses->end() ? &elementMatchingKey->second : nullptr;
}

[[nodiscard]] ProblemDefinition::Clause* ProblemDefinition::getClauseByIndexInFormula(std::size_t idxOfClauseInFormula)
{
	const auto& elementMatchingKey = clauses->find(idxOfClauseInFormula);
	return elementMatchingKey != clauses->end() ? &elementMatchingKey->second : nullptr;
}

std::vector<const ProblemDefinition::Clause*> ProblemDefinition::getClauses() const
{
	std::vector<const ProblemDefinition::Clause*> clausesContainer;
	clausesContainer.reserve(clauses->size());

	for (const auto& [key, _] : *clauses)
		clausesContainer.emplace_back(&clauses->at(key));
	return clausesContainer;
}

std::vector<std::size_t> ProblemDefinition::getIdentifiersOfClauses() const
{
	std::vector<std::size_t> identifierContainer;
	identifierContainer.reserve(clauses->size());

	for (const auto& [key, _] : *clauses)
		identifierContainer.emplace_back(key);
	return identifierContainer;
}


[[nodiscard]] std::size_t ProblemDefinition::getNumDeclaredVariablesOfFormula() const
{
	return nVariables;
}

[[nodiscard]] std::size_t ProblemDefinition::getNumDeclaredClausesOfFormula() const
{
	return nClauses;
}

std::size_t ProblemDefinition::getNumClausesAfterOptimizations() const
{
	return clauses ? clauses->size() : 0;
}

[[nodiscard]] const LiteralOccurrenceLookup& ProblemDefinition::getLiteralOccurrenceLookup() const
{
	return literalOccurrenceLookup;
}

std::optional<std::vector<long>> ProblemDefinition::getClauseLiteralsOmittingAlreadyAssignedOnes(std::size_t idxOfClauseInFormula) const
{
	const dimacs::ProblemDefinition::Clause* dataOfAccessedClause = getClauseByIndexInFormula(idxOfClauseInFormula);
	if (!dataOfAccessedClause)
		return std::nullopt;

	std::vector<long> unassignedClauseLiterals;
	for (const long literal : dataOfAccessedClause->literals)
	{
		if (variableValueLookup.getLiteralValue(literal).value_or(VariableValue::Unknown) == VariableValue::Unknown)
			unassignedClauseLiterals.emplace_back(literal);
	}
	return unassignedClauseLiterals;
}

ProblemDefinition::PropagationResult ProblemDefinition::propagate(long literal)
{
	if (variableValueLookup.getLiteralValue(literal).value_or(VariableValue::Unknown) != VariableValue::Unknown)
		return PropagationResult::Ok;

	const std::optional<bool> wouldAssignmentLeadToConflict = doesVariableAssignmentLeadToConflict(literal, determineSatisfyingAssignmentForLiteral(literal));
	if (!wouldAssignmentLeadToConflict.has_value())
		return PropagationResult::ErrorDuringPropagation;
	if (wouldAssignmentLeadToConflict.value())
		return PropagationResult::Conflict;

	if (!recordAssignment(PastAssignment({true, literal})))
		return PropagationResult::ErrorDuringPropagation;

	const std::optional<std::vector<std::size_t>> indicesOfClausesContainingLiteralContainer = literalOccurrenceLookup[literal];
	if (!indicesOfClausesContainingLiteralContainer.has_value())
		return PropagationResult::ErrorDuringPropagation;

	const std::vector<std::size_t>& indicesOfClausesContainingLiteral = *indicesOfClausesContainingLiteralContainer;
	for (std::size_t clauseIdx : indicesOfClausesContainingLiteral)
	{
		if (Clause* accessedClause = getClauseByIndexInFormula(clauseIdx); accessedClause)
			accessedClause->satisified = true;
		else
			return PropagationResult::ErrorDuringPropagation;
	}
		
	const std::optional<std::vector<std::size_t>> indicesOfClausesContainingNegatedLiteralContainer = literalOccurrenceLookup[-literal];
	if (!indicesOfClausesContainingNegatedLiteralContainer.has_value())
		return PropagationResult::ErrorDuringPropagation;

	const std::vector<std::size_t>& indicesOfClausesContainingNegatedLiteral = *indicesOfClausesContainingNegatedLiteralContainer;
	auto propagationResult = PropagationResult::Ok;
	for (std::size_t i = 0; propagationResult == PropagationResult::Ok && i < indicesOfClausesContainingNegatedLiteral.size(); ++i)
	{
		const std::optional<std::vector<long>> unassignedClauseLiteral = getClauseLiteralsOmittingAlreadyAssignedOnes(indicesOfClausesContainingNegatedLiteral.at(i));
		if (!unassignedClauseLiteral.has_value())
			return PropagationResult::ErrorDuringPropagation;
		if (unassignedClauseLiteral->size() == 1)
			propagationResult = propagate(unassignedClauseLiteral->front());
	}
	return propagationResult;
}

std::optional<ProblemDefinition::VariableValue> ProblemDefinition::getValueOfVariable(std::size_t variable) const
{
	return variableValueLookup.getVariableValue(variable);
}

std::string ProblemDefinition::stringify() const
{
	std::ostringstream stringificationContainer;
	stringificationContainer << "p cnf " << std::to_string(getNumDeclaredVariablesOfFormula()) << " " << std::to_string(getNumClausesAfterOptimizations());
	if (!clauses || clauses->empty())
		return stringificationContainer.str();

	for (const auto& [_, clause] : *clauses)
		stringificationContainer << "\n" << clause;

	return stringificationContainer.str();
}

std::optional<bool> ProblemDefinition::doesVariableAssignmentLeadToConflict(long literal, VariableValue chosenAssignment) const
{
	const std::optional<std::vector<std::size_t>>& indiciesOfClausesContainingNegatedLiteral = literalOccurrenceLookup[-literal];
	if (!indiciesOfClausesContainingNegatedLiteral.has_value())
		return std::nullopt;

	if (indiciesOfClausesContainingNegatedLiteral->empty() || chosenAssignment == VariableValue::Unknown)
		return false;

	bool foundConflict = false;
	for (std::size_t i = 0; i < indiciesOfClausesContainingNegatedLiteral->size() && !foundConflict; ++i)
	{
		const std::optional<std::vector<long>>& clauseLiterals = getClauseLiteralsOmittingAlreadyAssignedOnes(indiciesOfClausesContainingNegatedLiteral->at(i));
		if (!clauseLiterals.has_value())
			return std::nullopt;

		foundConflict = clauseLiterals->size() == 1 && clauseLiterals->front() == -literal && chosenAssignment == determineConflictingAssignmentForLiteral(literal);
	}
	return foundConflict;
}

const std::vector<ProblemDefinition::PastAssignment>& ProblemDefinition::getPastAssignments() const
{
	return pastAssignments;
}