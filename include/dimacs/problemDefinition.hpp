#ifndef PROBLEM_DEFINITION_HPP
#define PROBLEM_DEFINITION_HPP

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

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
			explicit Clause(std::vector<long> literals) : literals(std::move(literals))
			{
				sortLiterals();
			}

			void sortLiterals()
			{
				std::stable_sort(literals.begin(), literals.end(), std::less_equal<>());
			}
			
			std::vector<long> literals;
		};

		ProblemDefinition() = delete;
		ProblemDefinition(std::size_t numVariables, std::size_t numClauses)
		{
			const std::optional<std::size_t> requiredContainerSizeForStorageOfLiterals = LiteralInContainerIndexLookup::getRequiredTotalSizeOfContainerToStoreRange(numVariables);
			if (!requiredContainerSizeForStorageOfLiterals.has_value())
				throw std::invalid_argument("Internal container is only able to store at most " + std::to_string(LiteralInContainerIndexLookup::getMaximumStorableNumberOfVariables()) + " variables");

			nVariables = numVariables;
			variableValueLookup = VariableValueLookup(*requiredContainerSizeForStorageOfLiterals - 1);

			clauses = std::make_shared<std::vector<Clause>>();
			clauses->resize(numClauses);

			literalOccurrenceLookup = LiteralOccurrenceLookup(*requiredContainerSizeForStorageOfLiterals);
		}

		[[maybe_unused]] bool addClause(std::size_t index, Clause clause);
		[[nodiscard]] std::shared_ptr<std::vector<Clause>> getClauses() const;
		[[nodiscard]] std::optional<const Clause*> getClauseByIndexInFormula(std::size_t idxOfClauseInFormula) const;
		[[nodiscard]] std::optional<Clause*> getClauseByIndexInFormula(std::size_t idxOfClauseInFormula);
		[[nodiscard]] std::size_t getNumVariablesInFormula() const;
		[[nodiscard]] std::size_t getNumClauses() const;
		[[nodiscard]] const LiteralOccurrenceLookup& getLiteralOccurrenceLookup() const;
		[[nodiscard]] std::optional<std::vector<long>> getClauseLiteralsOmittingAlreadyAssignedOnes(std::size_t idxOfClauseInFormula) const;
		[[maybe_unused]] PropagationResult propagate(long literal);

	protected:
		struct VariableValueLookup
		{
			enum VariableValue : char
			{
				Low = 0,
				High = 1,
				Unknown = -1
			};
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

				if (getLiteralValue(literal).value_or(VariableValue::Unknown) == VariableValue::Unknown)
					return true;

				variableValues[literalToVariable(literal)] = determineSatisfyingAssignmentForLiteral(literal);
				return true;
			}

			[[nodiscard]] bool isVariableWithinRange(std::size_t variable) const noexcept
			{
				return variable && variable < variableValues.size();
			}

			[[nodiscard]] static VariableValue determineSatisfyingAssignmentForLiteral(long literal) noexcept
			{
				return static_cast<VariableValue>(literal > 0);
			}

			[[nodiscard]] static std::size_t literalToVariable(long literal) noexcept
			{
				return std::abs(literal);
			}
		};

		std::size_t nVariables;
		std::shared_ptr<std::vector<Clause>> clauses;
		VariableValueLookup variableValueLookup;
		LiteralOccurrenceLookup literalOccurrenceLookup;
	};
}
#endif