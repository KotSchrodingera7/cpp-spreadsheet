#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:


    struct HashPos {
        size_t operator()(const Position &pos) const {
            return pos.row + pos.col * 37;
        }
    };

    std::unordered_map<Position, std::unique_ptr<Cell>, HashPos> sheet_;
    Size size_;
};