

#ifndef TTABLEBASE_HPP
#define TTABLEBASE_HPP

#include <tintoretto.hpp>
#include <vector>
#include "move.hpp"


enum EntryType {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};


/**
 * @brief Represents an entry in the transposition table.
 * 
 * Stores the Zobrist hash key, depth of the search, evaluation score,
 * type of the entry (exact, lowerbound, upperbound), and the best move found
 * from this position.
 */
struct TTEntry {
    uint64_t key;
    int depth;
    int score; // evaluation score in centipawns
    EntryType type; // exact, lowerbound, upperbound
    uint32_t bestMove; // best move (see Move class)
    // int age; // for replacement schemes // only used for very fancy engines
};


class TTableBase {

    private:
        uint32_t size = 0; // number of entries
        std::vector<TTEntry> table;

    public:
        TTableBase(uint32_t sizeInMB = 64) {
            // check that we do not overflow uint_32_t
            if (sizeInMB > (UINT32_MAX / (1024 * 1024 / sizeof(TTEntry)))) {
                throw std::invalid_argument("Transposition table size is too large.");
            }
            // set the size
            setSize(sizeInMB);
        }

        /**
         * @brief Get the size of the transposition table in number of entries.
         * @return The size of the transposition table (a power of 2).
         */
        uint32_t getSize() const {
            return size;
        }

        /**
         * @brief Clears the transposition table by resetting all entries.
         */
        void clear() {
            for (auto& entry : table) {
                entry = TTEntry{};
            }
        }



        // --------------- //
        // !-- Storing --! //
        // --------------- //
    
        /**
         * @brief Adds an entry to the transposition table. If an entry with the same key already exists,
         * decides whether to replace it based on the depth (if the new evaluation is deeper, it replaces the old one).
         * @param entry The TTEntry object to store.
         */
        void store(const TTEntry& entry) {
            size_t index = entry.key & (size - 1);
            TTEntry& currentEntry = table[index];

            // simple replacement scheme: replace if the new depth is greater or the age is newer
            if (currentEntry.key != entry.key || entry.depth >= currentEntry.depth) {
                currentEntry = entry;
            }
        }

        /**
         * @brief Adds an entry to the transposition table. If an entry with the same key already exists,
         * decides whether to replace it based on the depth (if the new evaluation is deeper, it replaces the old one).
         * @param key The Zobrist hash key of the position.
         * @param depth The depth of the search that produced this evaluation.
         * @param score The evaluation score in centipawns.
         * @param type The type of the entry (EXACT, LOWERBOUND, UPPERBOUND).
         * @param bestMove The best move found from this position.
         */
        void store(uint64_t key, int depth, int score, EntryType type, uint32_t bestMove) {
            TTEntry entry;
            entry.key = key;
            entry.depth = depth;
            entry.score = score;
            entry.type = type;
            entry.bestMove = bestMove;
            store(entry);
        }

        

        // ------------------ //
        // !-- Retrieving --! //
        // ------------------ //

        /**
         * @brief Probes the transposition table for an entry with the given key and depth.
         * If found, fills the provided TTEntry object with the stored data.
         * @param entry A TTEntry object with the key and depth to probe. If found,
         * it will be filled with the stored data (score, type, bestMove).
         * @return True if an entry was found and filled, false otherwise.
         */
        bool probe(TTEntry& entry) const {
            size_t index = entry.key & (size - 1);
            const TTEntry& currentEntry = table[index];

            if (currentEntry.key == entry.key && currentEntry.depth >= entry.depth) {
                entry.score = currentEntry.score;
                entry.type = currentEntry.type;
                entry.bestMove = currentEntry.bestMove;
                return true;
            }
            return false;
        }

        /**
         * @brief Probes the transposition table for an entry with the given key and depth.
         * If found, fills the provided references with the stored data.
         * @param key The Zobrist hash key of the position to probe.
         * @param depth The depth of the search to probe.
         * @param score Reference to store the evaluation score if found.
         * @param type Reference to store the entry type if found.
         * @param bestMove Reference to store the best move if found.
         * @return True if an entry was found and filled, false otherwise.
         */
        bool probe(uint64_t key, int depth, int& score, EntryType& type, uint32_t& bestMove) const {
            TTEntry entry;
            entry.key = key;
            entry.depth = depth;
            if (probe(entry)) {
                score = entry.score;
                type = entry.type;
                bestMove = entry.bestMove;
                return true;
            }
            return false;
        }


        // ------------- //
        // !-- Debug --! //
        // ------------- //

        float getLoadFactor() const {
            uint32_t filledEntries = 0;
            for (const TTEntry entry : table) {
                if (entry.key != 0) {
                    filledEntries++;
                }
            }
            return static_cast<float>(filledEntries) / static_cast<float>(size);
        }




    private:

        /**
         * @brief Defines the length of the table in number of entries, based
         * on a target size in megabytes.
         */
        void setSize(uint32_t sizeInMB) {
            size = (sizeInMB * 1024 * 1024) / sizeof(TTEntry);
            // I actually want a power of 2 for more convinient indexing
            // round down to the nearest power of 2
            uint32_t power = 1;
            while (power * 2 < size) {
                power *= 2;
            }
            size = power;
            table.resize(size);
        }

        
};



#endif // TTABLEBASE_HPP