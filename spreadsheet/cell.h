#pragma once

#include "common.h"
#include "formula.h"
#include <optional>
#include <unordered_set>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override {
        return impl_->GetReferencedCells();  
    };

private:
    //Field of cache in the private zone
    void ClearCache();

    void CheckCycleGraph(const std::vector<Position> &ref_cell, std::unordered_set<const CellInterface *> &visited_cell);

    void UpdateDeps(const std::vector<Position> &ref_cell);
    void VolidateCache();

    std::unordered_set<Cell *> GetDependCell();
private:
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const {
            return {};
        };
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        CellInterface::Value GetValue() const override {
            return 0.0;
        }

        std::string GetText() const override {
            return {};
        }
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string str) : str_(str) {}

        CellInterface::Value GetValue() const override {
            if( str_.size() > 0 && str_[0] == '\'' ) {
                return str_.substr(1);
            }
            return str_;
        }
        std::string GetText() const override {
            return str_;
        }
    private:
        std::string str_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string formula, const SheetInterface& sheet) : formula_(ParseFormula(formula)), sheet_(sheet) {}

        CellInterface::Value GetValue() const override {
            FormulaInterface::Value val = formula_->Evaluate(sheet_);
            if (std::holds_alternative<double>(val)) {
                return std::get<double>(val);
            }
            return std::get<FormulaError>(val);
        }

        std::string GetText() const override {
            return '=' + formula_->GetExpression();
        }

        std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }
    private:
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& sheet_;
        
    };

private:
    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
    mutable std::optional<CellInterface::Value> cache_;
    std::unordered_set<Cell *> ref_cells_;
    std::unordered_set<Cell *> depend_cells_;

};