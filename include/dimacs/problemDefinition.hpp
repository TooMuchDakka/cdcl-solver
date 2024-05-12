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

				struct LiteralBounds
				{
					long smallestLiteral;
					long largestLiteral;

					explicit LiteralBounds(long smallestLiteral, long largestLiteral)
						: smallestLiteral(smallestLiteral), largestLiteral(largestLiteral) {}
				};

				std::vector<long> literals;
				[[nodiscard]] LiteralBounds getLiteralBounds() const
				{
					if (literals.size() == 1)
						return LiteralBounds(literals.front(), literals.front());
					return LiteralBounds(literals.front(), literals.back());
				}

				void sortLiterals()
				{
					std::stable_sort(literals.begin(), literals.end(), std::less_equal<>());
				}
			};

			ProblemDefinition() = delete;
			ProblemDefinition(std::size_t numVariables, std::size_t numClauses)
			{
				this->numUserDeclaredVariables = numVariables;
				literalValues = std::vector<bool>((numVariables * 2) + 1, false);

				clauses = std::make_shared<std::vector<Clause::ptr>>();
				clauses->reserve(numClauses);
				for (std::size_t i = 0; i < numClauses; ++i)
				{
					clauses->push_back(std::make_shared<Clause>());
				}
			}

			[[maybe_unused]] bool addClause(std::size_t index, Clause::ptr clause) const
			{
				if (clauses->size() <= index)
					return false;

				clauses->at(index) = std::move(clause);
				return true;
			}

			[[nodiscard]] std::shared_ptr<std::vector<Clause::ptr>> getClauses() const
			{
				return clauses;
			}

			[[nodiscard]] std::optional<Clause::ptr> getClauseByIndexInFormula(std::size_t idxOfClauseInFormula) const
			{
				if (idxOfClauseInFormula >= clauses->size())
					return std::nullopt;
				return clauses->at(idxOfClauseInFormula);
			}

			[[maybe_unused]] bool fixVariableAssignment(long literal)
			{
				if (fixVariableValues.count(std::abs(literal)))
					return false;

				fixVariableValues.emplace(std::abs(literal));
				setLiteral(literal, literal > 0 ? true : false);
				return true;
			}

			void setLiteral(long literal, bool assignedValue)
			{
				literalValues.at(numUserDeclaredVariables + literal) = assignedValue;
				literalValues.at(numUserDeclaredVariables - literal) = !assignedValue;
			}

		protected:
			std::size_t numUserDeclaredVariables;
			std::unordered_set<long> fixVariableValues;
			std::vector<bool> literalValues;
			std::shared_ptr<std::vector<Clause::ptr>> clauses;
	};
}
#endif
