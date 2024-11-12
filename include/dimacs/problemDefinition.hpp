#ifndef PROBLEM_DEFINITION_HPP
#define PROBLEM_DEFINITION_HPP

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>
#include <ostream>
#include <unordered_map>

#include "literalOccurrenceLookup.hpp"

namespace dimacs
{
	class ProblemDefinition
	{
	public:
		using ptr = std::shared_ptr<ProblemDefinition>;
		using constPtr = std::shared_ptr<const ProblemDefinition>;

		enum PropagationResult : char
		{
			Ok,
			ErrorDuringPropagation,
			Conflict
		};

		class Clause
		{
		public:
			using ptr = std::shared_ptr<Clause>;
			using constPtr = std::shared_ptr<const Clause>;

			Clause() = default;
			explicit Clause(std::vector<long> literals)
				: literals(std::move(literals)), satisified(false)
			{
				sortLiteralsAscendingly();
			}

			void sortLiteralsAscendingly();
			[[nodiscard]] bool doesClauseContainLiteral(long literal) const;
			[[nodiscard]] bool isTautology() const;
			[[nodiscard]] std::optional<long> getSmallestLiteralOfClause() const;
			[[nodiscard]] std::optional<long> getLargestLiteralOfClause() const;

			std::vector<long> literals;
			bool satisified;
		};

		enum VariableValue : char
		{
			Low = 0,
			High = 1,
			Unknown = -1
		};

		struct PastAssignment
		{
			bool isPropagation;
			long assignedLiteral;
		};

		ProblemDefinition() = delete;
		ProblemDefinition(std::size_t numVariables, std::size_t numClauses)
		{
			const std::optional<std::size_t> requiredContainerSizeForStorageOfLiterals = LiteralInContainerIndexLookup::getRequiredTotalSizeOfContainerToStoreRange(numVariables);
			if (!requiredContainerSizeForStorageOfLiterals.has_value())
				throw std::invalid_argument("Internal container is only able to store at most " + std::to_string(LiteralInContainerIndexLookup::getMaximumStorableNumberOfVariables()) + " variables");

			nVariables = numVariables;
			nClauses = numClauses;
			variableValueLookup = VariableValueLookup(*requiredContainerSizeForStorageOfLiterals - 1);

			clauses = std::make_shared<std::unordered_map<std::size_t, Clause>>();
			literalOccurrenceLookup = LiteralOccurrenceLookup(numVariables);
		}

		[[maybe_unused]] bool addClause(std::size_t index, Clause clause);
		[[maybe_unused]] bool removeClause(std::size_t index);
		[[maybe_unused]] bool removeLiteralFromClausesOfFormula(long literal);


 		[[nodiscard]] const Clause* getClauseByIndexInFormula(std::size_t idxOfClauseInFormula) const;
		[[nodiscard]] Clause* getClauseByIndexInFormula(std::size_t idxOfClauseInFormula);
		[[nodiscard]] std::vector<const Clause*> getClauses() const;
		[[nodiscard]] std::vector<std::size_t> getIdentifiersOfClauses() const;
		[[nodiscard]] std::size_t getNumDeclaredVariablesOfFormula() const;
		[[nodiscard]] std::size_t getNumDeclaredClausesOfFormula() const;
		[[nodiscard]] std::size_t getNumClausesAfterOptimizations() const;
		[[nodiscard]] const LiteralOccurrenceLookup& getLiteralOccurrenceLookup() const;
		[[nodiscard]] std::optional<std::vector<long>> getClauseLiteralsOmittingAlreadyAssignedOnes(std::size_t idxOfClauseInFormula) const;
		[[maybe_unused]] PropagationResult propagate(long literal);
		[[nodiscard]] std::optional<VariableValue> getValueOfVariable(std::size_t variable) const;
		[[nodiscard]] std::string stringify() const;
		[[nodiscard]] std::optional<bool> doesVariableAssignmentLeadToConflict(long literal, VariableValue chosenAssignment) const;
		[[nodiscard]] const std::vector<PastAssignment>& getPastAssignments() const;

		[[nodiscard]] static VariableValue determineSatisfyingAssignmentForLiteral(long literal) noexcept
		{
			return static_cast<VariableValue>(literal > 0);
		}

		[[nodiscard]] static VariableValue determineConflictingAssignmentForLiteral(long literal) noexcept
		{
			return static_cast<VariableValue>(literal < 0);
		}
	protected:
		struct VariableValueLookup
		{
			std::vector<VariableValue> variableValues;

			VariableValueLookup() = default;
			VariableValueLookup(std::size_t nVariables)
				: variableValues(std::vector(nVariables + 1, VariableValue::Unknown)) {}

			[[nodiscard]] std::optional<VariableValue> getVariableValue(std::size_t variable) const
			{
				if (!isVariableWithinRange(variable))
					return std::nullopt;
				return variableValues[variable];
			}

			[[nodiscard]] std::optional<VariableValue> getLiteralValue(long literal) const
			{
				return getVariableValue(literalToVariable(literal));
			}

			[[maybe_unused]] bool recordSatisfyingLiteralAssignment(long literal)
			{
				if (!isVariableWithinRange(literalToVariable(literal)))
					return false;

				if (getLiteralValue(literal).value_or(VariableValue::Unknown) != VariableValue::Unknown)
					return true;

				variableValues[literalToVariable(literal)] = determineSatisfyingAssignmentForLiteral(literal);
				return true;
			}

			[[nodiscard]] bool isVariableWithinRange(std::size_t variable) const noexcept
			{
				return variable && variable < variableValues.size();
			}

			[[nodiscard]] static std::size_t literalToVariable(long literal) noexcept
			{
				return std::abs(literal);
			}
		};

		[[maybe_unused]] bool recordAssignment(PastAssignment variableAssignment)
		{
			pastAssignments.emplace_back(variableAssignment);
			return variableValueLookup.recordSatisfyingLiteralAssignment(variableAssignment.assignedLiteral);
		}

		std::size_t nVariables;
		std::size_t nClauses;
		std::shared_ptr<std::unordered_map<std::size_t, Clause>> clauses;
		VariableValueLookup variableValueLookup;
		LiteralOccurrenceLookup literalOccurrenceLookup;
		std::vector<PastAssignment> pastAssignments;
	};

	inline std::ostream& operator<<(std::ostream& os, const dimacs::ProblemDefinition::Clause& clause)
	{
		if (clause.literals.empty())
			return os;

		if (clause.literals.size() > 1)
		{
			for (auto literalIterator = clause.literals.begin(); literalIterator != std::prev(clause.literals.end(), 2); ++literalIterator)
				os << std::to_string(*literalIterator) + " ";
		}
		os << std::to_string(clause.literals.back()) + " 0";
		return os;
	}
}
#endif