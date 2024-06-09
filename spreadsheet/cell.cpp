/* разместите здесь свой код */
#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы
Cell::Cell(SheetInterface &sheet) : sheet_(sheet), impl_(std::make_unique<EmptyImpl>()) {

}

Cell::~Cell() {

}

void Cell::Set(std::string text) {
    if( text.size() == 0 ) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    }

     if( text[0] == '=' && text.size() > 1 ) {
        ref_cells_.clear();
        auto new_impl = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        
        const auto new_ref_cells_ = new_impl->GetReferencedCells();
        if( new_ref_cells_.size() > 0 ) {
            std::unordered_set<const CellInterface*> visited_cell;
            CheckCycleGraph(new_ref_cells_, visited_cell);
            UpdateDeps(new_ref_cells_);
        }
        impl_ = std::move(new_impl);
    } else if( text.size() > 0 ) {
        impl_.reset();
        impl_ = std::make_unique<TextImpl>(text);
    }

    VolidateCache();
}

void Cell::Clear() {
    impl_.reset();
    cache_ = std::nullopt;
}

Cell::Value Cell::GetValue() const {

    if( !cache_.has_value() ) {
        cache_ = impl_->GetValue();
    }
    return cache_.value();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

void Cell::ClearCache() {
    cache_ = std::nullopt;
}


std::unordered_set<Cell *> Cell::GetDependCell() {
    return depend_cells_;
}
void Cell::UpdateDeps(const std::vector<Position> &ref_cell) {

    for( const auto &pos : ref_cell ) {
        auto cell = sheet_.GetCell(pos);
        if( !cell ) {
            sheet_.SetCell(pos, "");
        }

        cell = sheet_.GetCell(pos);

        ref_cells_.insert(dynamic_cast<Cell*>(cell));
        dynamic_cast<Cell*>(cell)->GetDependCell().insert(this);
    }
}

void Cell::VolidateCache() {
    ClearCache();
    for( auto &cell : ref_cells_ ) {
        cell->ClearCache();
    }
}

void Cell::CheckCycleGraph(const std::vector<Position> &ref_cell, std::unordered_set<const CellInterface *> &visited_cell) {

    for( auto &pos : ref_cell ) {
        auto cell = sheet_.GetCell(pos);
        if( cell == this ) {
            throw CircularDependencyException("found cycle");
        }

        if( cell ) {
            auto next_ref_cell = cell->GetReferencedCells();
            if( !next_ref_cell.empty() && !visited_cell.count(cell)) {
                CheckCycleGraph(next_ref_cell, visited_cell);
            }

            visited_cell.emplace(cell);
        }
    }
}