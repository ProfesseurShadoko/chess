
#include <tintoretto.hpp>
#include "move.hpp"
#include "boardUI.hpp"
#include <string>
#include "ttableBase.hpp"

int main() {
    Message("Testing file ttableBase.hpp");

    // --------------------------- //
    // !-- Test Initialization --! //
    // --------------------------- //

    Test test("TTableBase Initialization");
    uint32_t requestedsize_in_mb = 128;
    TTableBase ttable(requestedsize_in_mb); // 128 MB

    // display size in number of entries
    uint32_t num_entries = ttable.getSize();
    float num_entries_in_million = num_entries / 1000000.0f;
    Message::print("Transposition table size: " + std::to_string(num_entries_in_million) + " million entries");

    // check size is power of 2
    uint32_t size = ttable.getSize();
    bool is_power_of_2 = (size & (size - 1)) == 0;
    test.check("Size is power of 2", is_power_of_2);

    // check size is approximately correct
    uint32_t actual_size_in_bytes = size * sizeof(TTEntry);
    float actual_size_in_mb = actual_size_in_bytes / (1024.0f * 1024.0f);
    float size_difference = std::abs(actual_size_in_mb - requestedsize_in_mb) / static_cast<float>(requestedsize_in_mb);

    Message::print("Requested size: " + std::to_string(requestedsize_in_mb) + " MB");
    Message::print("Actual size: " + std::to_string(actual_size_in_mb) + " MB");
    Message::print("Size difference: " + std::to_string(size_difference * 100.0f) + " %");
    test.check("Size is approximately correct", size_difference < 0.5f); // allow 50% difference
    test.complete();

    // --------------------------------- //
    // !-- Check Storing and Probing --! //
    // --------------------------------- //

    test = Test("TTableBase Storing and Probing");
    ttable.clear();
    uint64_t test_key = 0x123456789ABCDEF0;
    int test_depth = 5;
    int test_score = 42;
    EntryType test_type = EXACT;
    uint32_t test_bestMove = Move(makeSquare("e2"), makeSquare("e4"), makePiece('P')).move;
    
    ttable.store(test_key, test_depth, test_score, test_type, test_bestMove);
    int retrieved_score;
    EntryType retrieved_type;
    uint32_t retrieved_bestMove;

    bool found = ttable.probe(test_key, test_depth, retrieved_score, retrieved_type, retrieved_bestMove);
    test.check("Entry found in table", found);
    test.check("Retrieved score matches stored score", retrieved_score == test_score);
    test.check("Retrieved type matches stored type", retrieved_type == test_type);
    test.check("Retrieved best move matches stored best move", retrieved_bestMove == test_bestMove);
    test.complete();


    // ------------------------- //
    // !-- Check Load Factor --! //
    // ------------------------- //
    
    test = Test("TTableBase Load Factor");
    float load_factor = ttable.getLoadFactor();
    Message::print("Load factor after one insertion: " + std::to_string(load_factor));
    test.check("Load factor is low after one insertion", load_factor < 0.01f);
    test.check("Load factor is non-zero after one insertion", load_factor > 0.0f);
    ttable.clear();
    load_factor = ttable.getLoadFactor();
    Message::print("Load factor after clearing: " + std::to_string(load_factor));
    test.check("Load factor is zero after clearing", load_factor == 0.0f);
    test.complete();

    return 0;
}