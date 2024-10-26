#ifndef PROBLEM_DEFINITION_HPP
#define PROBLEM_DEFINITION_HPP

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

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
			this->numUserDeclaredVariables = numVariables;
			literalValues = std::vector<bool>((numVariables * 2) + 1, false);

			clauses = std::make_shared<std::vector<Clause>>();
			clauses->reserve(numClauses);
			for (std::size_t i = 0; i < numClauses; ++i)
			{
				clauses->emplace_back();
			}
		}

		[[maybe_unused]] bool addClause(std::size_t index, Clause clause) const
		{
			if (clauses->size() <= index)
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

		[[maybe_unused]] bool fixVariableAssignment(long literal)
		{
			if (variablesWithValuesDeterminedDuringPreprocessing.count(std::abs(literal)))
				return false;

			variablesWithValuesDeterminedDuringPreprocessing.emplace(std::abs(literal));
			setLiteral(literal, literal > 0);
			return true;
		}

		void setLiteral(long literal, bool assignedValue)
		{
			literalValues.at(numUserDeclaredVariables + literal) = assignedValue;
			literalValues.at(numUserDeclaredVariables - literal) = !assignedValue;
		}

		[[nodiscard]] std::vector<long> getVariablesWithValueDeterminedDuringPreprocessing() const
		{
			return { variablesWithValuesDeterminedDuringPreprocessing.cbegin(), variablesWithValuesDeterminedDuringPreprocessing.cend() };
		}

		[[nodiscard]] std::size_t getNumVariablesInFormula() const { return numUserDeclaredVariables; }
		[[nodiscard]] std::size_t getNumClauses() const { return clauses->size(); }

	protected:
		std::size_t numUserDeclaredVariables;
		std::unordered_set<long> variablesWithValuesDeterminedDuringPreprocessing;
		std::vector<bool> literalValues;
		std::shared_ptr<std::vector<Clause>> clauses;
	};
}
#endif