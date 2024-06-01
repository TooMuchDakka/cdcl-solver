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

				struct LiteralBounds
				{
					long smallestLiteral;
					long largestLiteral;

					explicit LiteralBounds(long smallestLiteral, long largestLiteral)
						: smallestLiteral(smallestLiteral), largestLiteral(largestLiteral) {}

					[[nodiscard]] bool operator==(const LiteralBounds& other) const
					{
						return std::tie(smallestLiteral, largestLiteral) == std::tie(other.smallestLiteral, other.largestLiteral);
					}

					[[nodiscard]] bool operator<(const LiteralBounds& other) const
					{
						return smallestLiteral < other.smallestLiteral;
					}

					[[nodiscard]] bool operator>(const LiteralBounds& other) const
					{
						return largestLiteral > other.largestLiteral;
					}

					[[nodiscard]] bool contains(long literal) const
					{
						return literal >= smallestLiteral && literal <= largestLiteral;
					}
				};

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
				[[nodiscard]] bool containsLiteral(long literal) const
				{
					if (literals.empty() || literals.front() > literal || literals.back() < literal)
						return false;

					std::size_t low = 0;
					std::size_t high = literals.size() - 1;
					while (low <= high)
					{
						std::size_t mid = low + ((high - low) / 2);
						const long literalAtMidPosition = literals.at(mid);
						if (literalAtMidPosition == literal)
							return true;
						if (literalAtMidPosition < literal)
							low = mid + 1;
						else
							high = mid - 1;
					}
					return false;
				}

				[[nodiscard]] bool containsLiteralSmallerThanGivenBound(LiteralBounds literalBounds) const
				{
					if (literals.empty() || literalBounds.smallestLiteral < literals.front())
						return false;

					constexpr std::size_t low = 0;
					std::size_t high = literals.size() - 1;
					while (high >= low)
					{
						const std::size_t mid = low + ((high - low) / 2);
						const long literalAtMidPosition = literals.at(mid);
						if (literalAtMidPosition < literalBounds.largestLiteral)
							return true;

						if (!high)
							return false;

						high = mid - 1;
					}
					return false;
				}

				[[nodiscard]] bool containsLiteralLargerThanGivenBound(LiteralBounds literalBounds) const
				{
					if (literals.empty() || literalBounds.largestLiteral > literals.back())
						return false;

					std::size_t low = 0;
					const std::size_t high = literals.size() - 1;
					while (low <= high)
					{
						const std::size_t mid = low + ((high - low) / 2);
						const long literalAtMidPosition = literals.at(mid);
						if (literalAtMidPosition > literalBounds.largestLiteral)
							return true;
						
						low = mid + 1;
					}
					return false;
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
