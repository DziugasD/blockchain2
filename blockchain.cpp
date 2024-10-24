#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
#include "hash.h" 

class User {
private:
    std::string name;
    std::string publicKey;

public:
    User(const std::string& name, const std::string& publicKey)
        : name(name), publicKey(publicKey) {}

    std::string getPublicKey() const { return publicKey; }
    std::string getName() const { return name; }
};

class UTXO {
public:
    std::string transactionId;
    int outputIndex;
    double amount;
    std::string ownerKey;

    UTXO(const std::string& txId, int index, double amt, const std::string& owner)
        : transactionId(txId), outputIndex(index), amount(amt), ownerKey(owner) {}
};

class Transaction {
private:
    std::string transactionId;
    std::vector<UTXO> inputs;
    std::vector<UTXO> outputs;
    std::chrono::system_clock::time_point timestamp;

public:
    Transaction(const std::vector<UTXO>& inputs, const std::vector<UTXO>& outputs)
        : inputs(inputs), outputs(outputs) {
        timestamp = std::chrono::system_clock::now();
        generateTransactionId();
    }

    void generateTransactionId() {
        MyHash hasher;
        std::string data;
        // Combine all inputs
        for (const auto& input : inputs) {
            data += input.transactionId + std::to_string(input.outputIndex) + 
                   std::to_string(input.amount) + input.ownerKey;
        }
        // Combine all outputs
        for (const auto& output : outputs) {
            data += std::to_string(output.amount) + output.ownerKey;
        }
        data += std::to_string(std::chrono::system_clock::to_time_t(timestamp));
        transactionId = hasher.generateHash(data);
        
        // Update output transaction IDs
        for (auto& output : outputs) {
            output.transactionId = transactionId;
        }
    }

    std::string getId() const { return transactionId; }
    const std::vector<UTXO>& getInputs() const { return inputs; }
    const std::vector<UTXO>& getOutputs() const { return outputs; }
};

class Block {
private:
    std::string previousHash;
    std::vector<Transaction> transactions;
    std::string merkleRoot;
    int nonce;
    std::string blockHash;
    int blockHeight;
    std::chrono::system_clock::time_point timestamp;

public:
    Block(const std::string& prevHash, int height)
        : previousHash(prevHash), nonce(0), blockHeight(height) {
        timestamp = std::chrono::system_clock::now();
    }

    void addTransaction(const Transaction& tx) {
        transactions.push_back(tx);
        calculateMerkleRoot();
    }

    void calculateMerkleRoot() {
        MyHash hasher;
        std::string combinedTxIds;
        for (const auto& tx : transactions) {
            combinedTxIds += tx.getId();
        }
        merkleRoot = hasher.generateHash(combinedTxIds);
    }

    bool mineBlock(int difficulty) {
        MyHash hasher;
        std::string target(difficulty, '0');
        
        while (true) {
            std::string data = previousHash + merkleRoot + 
                             std::to_string(nonce) + 
                             std::to_string(std::chrono::system_clock::to_time_t(timestamp));
            
            blockHash = hasher.generateHash(data);
            
            if (blockHash.substr(0, difficulty) == target) {
                return true;
            }
            nonce++;
        }
    }

    void printBlock() const {
        std::stringstream buffer;
        buffer << "\n=== Block #" << blockHeight << " ===\n";
        buffer << "Hash: " << blockHash << "\n";
        buffer << "Previous Hash: " << previousHash << "\n";
        buffer << "Merkle Root: " << merkleRoot << "\n";
        buffer << "Nonce: " << nonce << "\n";
        buffer << "Timestamp: " << std::chrono::system_clock::to_time_t(timestamp) << "\n";
        buffer << "Transaction count: " << transactions.size() << "\n";
        
        for (const auto& tx : transactions) {
            buffer << "\nTransaction ID: " << tx.getId() << "\n";
            buffer << "Inputs:\n";
            for (const auto& input : tx.getInputs()) {
                buffer << "  From: " << input.ownerKey << ", Amount: " << input.amount << "\n";
            }
            buffer << "Outputs:\n";
            for (const auto& output : tx.getOutputs()) {
                buffer << "  To: " << output.ownerKey << ", Amount: " << output.amount << "\n";
            }
        }

        std::cout << buffer.str();
    }

    const std::vector<Transaction>& getTransactions() const { return transactions; }
    std::string getHash() const { return blockHash; }
};

class Blockchain {
private:
    std::vector<Block> chain;
    std::vector<Transaction> pendingTransactions;
    std::vector<User> users;
    int difficulty;
    std::vector<UTXO> utxoPool;
    
    std::string generatePublicKey() {
        MyHash hasher;
        static int counter = 0;
        return hasher.generateHash("user" + std::to_string(counter++));
    }

    double calculateUserBalance(const std::string& publicKey) const {
        double balance = 0.0;
        for (const auto& utxo : utxoPool) {
            if (utxo.ownerKey == publicKey) {
                balance += utxo.amount;
            }
        }
        return balance;
    }

public:
    Blockchain(int diff = 4) : difficulty(diff) {
        // Create genesis block
        Block genesisBlock("0", 0);
        genesisBlock.mineBlock(difficulty);
        chain.push_back(genesisBlock);
    }
    
    void createGenesisUTXOs() {
        for (const auto& user : users) {
            double initialBalance = calculateUserBalance(user.getPublicKey());
            UTXO genesisUtxo(chain[0].getHash(), utxoPool.size(), initialBalance, user.getPublicKey());
            utxoPool.push_back(genesisUtxo);
        }
    }

    void createUsers(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(100.0, 100000.0);

        for (int i = 0; i < count; i++) {
            std::string name = "User" + std::to_string(i);
            std::string publicKey = generatePublicKey();
            users.push_back(User(name, publicKey));

            // Create a genesis UTXO for each user's initial balance
            double initialBalance = distr(gen);
            UTXO genesisUtxo(chain[0].getHash(), utxoPool.size(), initialBalance, publicKey);
            utxoPool.push_back(genesisUtxo);
        }
        std::cout << count << " users created with initial UTXOs\n";
    }

        void generateTransactions(int count) {
        if (users.size() < 2) return;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> userDistr(0, users.size() - 1);
        std::uniform_real_distribution<> amountDistr(1.0, 1000.0);

        for (int i = 0; i < count; i++) {
            int senderIdx = userDistr(gen);
            int receiverIdx;
            do {
                receiverIdx = userDistr(gen);
            } while (receiverIdx == senderIdx);

            double amount = amountDistr(gen);
            std::vector<UTXO> availableUtxos;
            double totalAvailable = 0.0;

            // Collect ALL available UTXOs for the sender first
            for (const auto& utxo : utxoPool) {
                if (utxo.ownerKey == users[senderIdx].getPublicKey()) {
                    availableUtxos.push_back(utxo);
                    totalAvailable += utxo.amount;
                }
            }

            // Sort UTXOs by amount (smallest first) for better UTXO selection
            std::sort(availableUtxos.begin(), availableUtxos.end(),
                     [](const UTXO& a, const UTXO& b) { return a.amount < b.amount; });

            // Check if sender has enough funds
            if (totalAvailable >= amount) {
                std::vector<UTXO> selectedInputs;
                double totalInput = 0.0;
                int outputIndex = 0;

                // Select UTXOs using a more sophisticated selection strategy
                // Try to find a single UTXO that closely matches the amount first
                bool foundSingleUtxo = false;
                for (const auto& utxo : availableUtxos) {
                    if (utxo.amount >= amount && utxo.amount <= amount * 1.5) {
                        selectedInputs.push_back(utxo);
                        totalInput = utxo.amount;
                        foundSingleUtxo = true;
                        break;
                    }
                }

                // If no suitable single UTXO found, use multiple UTXOs
                if (!foundSingleUtxo) {
                    // Start with smallest UTXOs
                    for (const auto& utxo : availableUtxos) {
                        if (totalInput < amount) {
                            selectedInputs.push_back(utxo);
                            totalInput += utxo.amount;
                        } else {
                            break;
                        }
                    }
                }

                // Create outputs
                std::vector<UTXO> outputs;
                
                // Payment to receiver
                outputs.emplace_back("", outputIndex++, amount, users[receiverIdx].getPublicKey());
                
                // Change back to sender if necessary
                // Only create change UTXO if it's worth it (e.g., more than 1.0)
                double change = totalInput - amount;
                if (change >= 1.0) {
                    outputs.emplace_back("", outputIndex++, change, users[senderIdx].getPublicKey());
                }

                // Create and add transaction
                Transaction tx(selectedInputs, outputs);
                pendingTransactions.push_back(tx);

                // Remove spent UTXOs from pool
                for (const auto& input : selectedInputs) {
                    utxoPool.erase(
                        std::remove_if(utxoPool.begin(), utxoPool.end(),
                            [&input](const UTXO& utxo) {
                                return utxo.transactionId == input.transactionId && 
                                       utxo.outputIndex == input.outputIndex;
                            }), 
                        utxoPool.end());
                }
            }
        }
        std::cout << pendingTransactions.size() << " transactions generated\n";
    }
    
    void updateUTXOPool(const Transaction& tx) {
        // Remove spent UTXOs
        for (const auto& input : tx.getInputs()) {
            utxoPool.erase(
                std::remove_if(utxoPool.begin(), utxoPool.end(),
                    [&input](const UTXO& utxo) {
                        return utxo.transactionId == input.transactionId && 
                               utxo.outputIndex == input.outputIndex;
                    }), 
                utxoPool.end());
        }
        // Add new UTXOs
        utxoPool.insert(utxoPool.end(), tx.getOutputs().begin(), tx.getOutputs().end());
    }

    void mineNextBlock() {
        if (pendingTransactions.empty()) return;

        // Select random 100 transactions
        std::vector<Transaction> blockTxs;
        int txCount = std::min(100, static_cast<int>(pendingTransactions.size()));
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::vector<int> indices(pendingTransactions.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), gen);

        Block newBlock(chain.back().getHash(), chain.size());
        
        for (int i = 0; i < txCount; i++) {
            newBlock.addTransaction(pendingTransactions[indices[i]]);
        }

        std::cout << "Mining block " << chain.size() << "...\n";
        if (newBlock.mineBlock(difficulty)) {
            // Update balances
            const auto& transactions = newBlock.getTransactions();
            for (const auto& tx : transactions) {
                updateUTXOPool(tx);
            }

            // Remove processed transactions
            std::vector<Transaction> remainingTxs;
            for (size_t i = 0; i < pendingTransactions.size(); i++) {
                if (std::find(indices.begin(), indices.begin() + txCount, i) == indices.begin() + txCount) {
                    remainingTxs.push_back(pendingTransactions[i]);
                }
            }
            pendingTransactions = remainingTxs;

            chain.push_back(newBlock);
            newBlock.printBlock();
        }
    }

    void printUTXOPoolInfo() const {
        std::cout << "\n=== UTXO Pool Info ===\n";
        std::cout << "Total UTXOs: " << utxoPool.size() << "\n";
        double totalValue = 0.0;
        std::map<std::string, double> balances;
        
        for (const auto& utxo : utxoPool) {
            totalValue += utxo.amount;
            balances[utxo.ownerKey] += utxo.amount;
        }
        
        std::cout << "Total value in UTXO pool: " << totalValue << "\n";
        std::cout << "\nUser balances from UTXO pool:\n";
        for (const auto& user : users) {
            std::cout << user.getName() << ": " << balances[user.getPublicKey()] << "\n";
        }
    }

    void printChainInfo() const {
        std::cout << "\n=== Blockchain Info ===\n";
        std::cout << "Number of blocks: " << chain.size() << "\n";
        std::cout << "Pending transactions: " << pendingTransactions.size() << "\n";
        std::cout << "Number of users: " << users.size() << "\n";
        std::cout << "Mining difficulty: " << difficulty << "\n";
    }

    void printUserBalances() const {
        std::cout << "\n=== User Balances ===\n";
        for (const auto& user : users) {
            double balance = calculateUserBalance(user.getPublicKey());
            std::cout << user.getName() << " (" << user.getPublicKey().substr(0, 8) << "...): " 
                      << std::fixed << std::setprecision(2) << balance << "\n";
        }
    }

    const std::vector<Transaction>& getPendingTransactions() const {
        return pendingTransactions;
    }
};

int main() {
    // Optimize I/O operations
    std::ios::sync_with_stdio(false);
    // std::cin.tie(nullptr);

    Blockchain blockchain(4); 

    // Generate initial users and transactions
    blockchain.createUsers(1000);
    blockchain.generateTransactions(10000);
    blockchain.printChainInfo();

    // Mine blocks until no pending transactions
    while (true) {
        std::string command;
        std::cout << "\nEnter command (mine/mine_all/info/balances/new_user <number>/new_transaction <number>/exit): ";
        std::cin >> command;

        if (command == "mine") {
            blockchain.mineNextBlock();
        }
        else if (command == "mine_all") {
            while (!blockchain.getPendingTransactions().empty()) {
                blockchain.mineNextBlock();
            }
        }
        else if (command == "info") {
            blockchain.printChainInfo();
        }
        else if (command == "balances") {
            blockchain.printUserBalances();
        }
        else if (command == "new_user") {
            int number;
            std::cin >> number;
            blockchain.createUsers(number);
        }
        else if (command == "new_transaction") {
            int number;
            std::cin >> number;
            blockchain.generateTransactions(number);
        }
        else if (command == "utxos") {
            blockchain.printUTXOPoolInfo();
        }   
        else if (command == "exit") {
            break;
        }
    }

    return 0;
}
