

#include <vector>
#include "move.hpp"
#include "boardUI.hpp"
#include "positionBase.hpp"
#include "positionStructureBase.hpp"
#include "positionStructureBase.hpp"

template <typename PositionStructureType>
class MoveGeneratorBase {
    public:
        virtual std::vector<Move> getLegalMoves(PositionStructureType& position,  Color sideToMove, Square enPassantSquare, std::vector<bool> castlingRights) const {
            return {};
        }

        virtual bool isInCheck(const PositionStructureType& position, Color color) const {
            return false;
        }

        // --------------------- //
        // !-- Play / Unplay --! //
        // --------------------- //

        // in case I want pin tracking for instance, my cass needs to be updated every move!

        virtual void play(const Move& move, const PositionStructureType& position) {}
        virtual void unplay(const Move& move, const PositionStructureType& position) {}

        virtual ~MoveGeneratorBase() = default;
};

