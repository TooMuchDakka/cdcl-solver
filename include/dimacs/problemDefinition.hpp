#ifndef PROBLEM_DEFINITION_HPP
#define PROBLEM_DEFINITION_HPP

#include <memory>
#include <vector>

namespace dimacs
{
	class ProblemDefinition
	{
		public:
			using ptr = std::unique_ptr<ProblemDefinition>;

			class Clause
			{
			public:
				std::vector<std::size_t> literals;
			};

			ProblemDefinition() = delete;
			ProblemDefinition(std::size_t numLiterals, std::size_t numClauses)
			{
				literalValues = std::vector<bool>(numLiterals, 0);
				clauses = std::vector<Clause>(numClauses, Clause());
			}

			[[maybe_unused]] bool addClause(std::size_t index, Clause clause)
			{
				if (clauses.size() <= index)
					return false;
				clauses.at(index) = std::move(clause);
				return true;
			}

		protected:
			std::vector<bool> literalValues;
			std::vector<Clause> clauses;
	};
}
#endif
