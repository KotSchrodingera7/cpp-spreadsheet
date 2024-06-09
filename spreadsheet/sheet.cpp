/* разместите здесь свой код */
#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if( pos.IsValid() ) {

        
        if( size_.cols < pos.col + 1 ) {
            size_.cols = pos.col + 1;
        }

        if( size_.rows < pos.row + 1 ) {
            size_.rows = pos.row + 1;
        }

        if( !sheet_.count(pos) ) {

            sheet_[pos] = std::make_unique<Cell>(*this);
        }
        
        sheet_[pos]->Set(text);
        
    } else {
        throw InvalidPositionException("Pos is invalid"s);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    
    
    if( pos.IsValid()  ) {
        if( sheet_.count(pos)) {
            return sheet_.at(pos).get();
        }
        
    } else {
        throw InvalidPositionException("Pos is invalid"s);
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if( pos.IsValid() ) {

        if( !GetCell(pos) ) {
            return;
        }

        sheet_.erase(pos);

        if(pos.row + 1 == size_.rows ||
            pos.col + 1 == size_.cols ) {
            
            Size new_size{-1, -1};

            for(const auto &[pos, cel] : sheet_) {

                if( pos.col > new_size.cols ) {
                    new_size.cols = pos.col;
                }

                if( pos.row > new_size.rows ) {
                    new_size.rows = pos.row;
                }
            }

            size_ = {new_size.rows + 1, new_size.cols + 1};
        }
    } else {
        throw InvalidPositionException("Pos is invalid"s);
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            auto cell = GetCell({i, j});
            Cell::Value ans;
            if (cell) {
                ans = cell->GetValue();
            }
            switch (ans.index()) {
                case 0: {
                    output << std::get<0>(ans) << (j + 1 == size_.cols ? "" : "\t");
                    break;
                }
                case 1: {
                    output << std::get<1>(ans) << (j + 1 == size_.cols ? "" : "\t");
                    break;
                }
                case 2: {
                    output << std::get<2>(ans) << (j + 1 == size_.cols ? "" : "\t");
                    break;
                }
                default:
                    break;
                    // assert(false);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            auto cell = GetCell({i, j});
            std::string ans = "";
            if (cell) {
                ans = cell->GetText();
            }
            output << ans << (j + 1 == size_.cols ? "" : "\t");
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}