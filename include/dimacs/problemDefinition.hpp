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
			literalValues = std::vector(*requiredContainerSizeForStorageOfLiterals, false);

			clauses = std::make_shared<std::vector<Clause>>();
			clauses->resize(numClauses);

			literalOccurrenceLookup = LiteralOccurrenceLookup(*requiredContainerSizeForStorageOfLiterals);
		}

		[[maybe_unused]] bool addClause(std::size_t index, Clause clause)
		{
			if (index >= clauses->size() || !literalOccurrenceLookup.recordClauseLiteralOccurrences(index, clause.literals))
				return false;

			clauses->at(index) = std::move(clause);
			return true;
		}

		[[nodiscard]] std::shared_ptr<std::vector<Clause>> getClauses() const
		{
			return clauses;
		}

		[[nodiscard]] std::optional<const Clause*> getClauseByIndexInFormula(std::size_t idxOfClauseInFormula) const
		{
			if (idxOfClauseInFormula >= clauses->size())
				return std::nullopt;

			return &clauses->at(idxOfClauseInFormula);
		}

		[[nodiscard]] std::optional<Clause*> getClauseByIndexInFormula(std::size_t idxOfClauseInFormula)
		{
			if (idxOfClauseInFormula >= clauses->size())
				return std::nullopt;

			return &clauses->at(idxOfClauseInFormula);
		}

		[[nodiscard]] std::size_t getNumVariablesInFormula() const { return nVariables; }
		[[nodiscard]] std::size_t getNumClauses() const { return clauses->size(); }
		[[nodiscard]] const LiteralOccurrenceLookup& getLiteralOccurrenceLookup() const
		{
			return literalOccurrenceLookup;
		}

	protected:
		std::size_t nVariables;
		std::unordered_set<long> variablesWithValuesDeterminedDuringPreprocessing;
		std::vector<bool> literalValues;
		std::shared_ptr<std::vector<Clause>> clauses;
		LiteralOccurrenceLookup literalOccurrenceLookup;
	};
}
#endif