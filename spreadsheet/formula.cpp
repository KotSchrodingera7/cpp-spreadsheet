/* разместите здесь свой код */
#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}
    Value Evaluate(const SheetInterface& sheet) const override {

        Value result;
        try {
            result = ast_.Execute([&sheet](const Position &pos) {

                auto cell = sheet.GetCell(pos);
                if( cell == nullptr ) {
                    return 0.0;
                }

                auto value = cell->GetValue();

                if( std::get_if<FormulaError>(&value)) {
                    throw value;
                }

                if( std::get_if<double>(&value) ) {
                    return std::get<double>(value);
                }

                if( std::get_if<std::string>(&value) ) {
                    const std::string str = std::get<std::string>(value);
                    size_t idx = 0;
                    try {
                        double res = std::stod(str.c_str(), &idx);
                        if (idx < str.size()) {
                            throw FormulaError(FormulaError::Category::Value);
                        }
                        return res;
                    } catch (...) {
                    }
                }
                
                throw FormulaError(FormulaError::Category::Value);
            });
        } catch(FormulaError &e) {
            result = e;
        }

        return result;
    }
    std::string GetExpression() const override {
        std::ostringstream result;
        
        ast_.PrintFormula(result);

        return result.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        auto cells_ = ast_.GetCells();
        std::set set_(cells_.begin(), cells_.end());

        return std::vector<Position>(set_.begin(), set_.end());
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(expression);
    } catch (...) {
        throw FormulaException("Parsing formula from expression was failure"s);
    }
}