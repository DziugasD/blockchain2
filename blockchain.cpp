#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
#include "hash.h" 

struct MerkleNode {
    std::string hash;
    MerkleNode *left, *right;
    
    MerkleNode(const std::string& hash) : hash(hash), left(nullptr), right(nullptr) {}
    ~MerkleNode() {
        delete left;
        delete right;
    }
};

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
    bool verifyTransaction(const std::vector<UTXO>& utxoPool) const {
        for (const auto& input : inputs) {
            auto it = std::find_if(utxoPool.begin(), utxoPool.end(),
                [&input](const UTXO& utxo) {
                    return utxo.transactionId == input.transactionId &&
                           utxo.outputIndex == input.outputIndex &&
                           utxo.amount == input.amount &&
                           utxo.ownerKey == input.ownerKey;
                });
            if (it == utxoPool.end()) return false;
        }

        double inputSum = 0, outputSum = 0;
        for (const auto& input : inputs) inputSum += input.amount;
        for (const auto& output : outputs) outputSum += output.amount;
        if (inputSum < outputSum) return false;

        MyHash hasher;
        std::string data;
        for (const auto& input : inputs) {
            data += input.transactionId + std::to_string(input.outputIndex) + 
                   std::to_string(input.amount) + input.ownerKey;
        }
        for (const auto& output : outputs) {
            data += std::to_string(output.amount) + output.ownerKey;
        }
        data += std::to_string(std::chrono::system_clock::to_time_t(timestamp));
        return hasher.generateHash(data) == transactionId;
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

    void printInfo() const {
        std::cout << "Transaction ID: " << transactionId << "\n";
        std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
        std::cout << "Timestamp: " << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "\n";
        
        std::cout << "Inputs:\n";
        for (const auto& input : inputs) {
            std::cout << "  Transaction ID: " << input.transactionId << "\n";
            std::cout << "  Output Index: " << input.outputIndex << "\n";
            std::cout << "  Amount: " << input.amount << "\n";
            std::cout << "  Owner Key: " << input.ownerKey << "\n";
        }

        std::cout << "Outputs:\n";
        for (const auto& output : outputs) {
            std::cout << "  Transaction ID: " << output.transactionId << "\n";
            std::cout << "  Output Index: " << output.outputIndex << "\n";
            std::cout << "  Amount: " << output.amount << "\n";
            std::cout << "  Owner Key: " << output.ownerKey << "\n";
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

    MerkleNode* buildMerkleTree(const std::vector<std::string>& leaves) {
        if (leaves.empty()) return nullptr;
        
        std::vector<MerkleNode*> nodes;
        for (const auto& leaf : leaves) {
            nodes.push_back(new MerkleNode(leaf));
        }
        
        while (nodes.size() > 1) {
            std::vector<MerkleNode*> newLevel;
            for (size_t i = 0; i < nodes.size(); i += 2) {
                MerkleNode* left = nodes[i];
                MerkleNode* right = (i + 1 < nodes.size()) ? nodes[i + 1] : nodes[i];
                
                MyHash hasher;
                std::string combinedHash = hasher.generateHash(left->hash + right->hash);
                MerkleNode* parent = new MerkleNode(combinedHash);
                parent->left = left;
                parent->right = right;
                newLevel.push_back(parent);
            }
            nodes = newLevel;
        }
        
        return nodes[0];
    }

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
        std::vector<std::string> txHashes;
        for (const auto& tx : transactions) {
            txHashes.push_back(tx.getId());
        }
        
        MerkleNode* root = buildMerkleTree(txHashes);
        if (root) {
            merkleRoot = root->hash;
            delete root;
        } else {
            merkleRoot = "";
        }
    }

    bool mineBlock(int difficulty, int timeLimit) {
        MyHash hasher;
        std::string target(difficulty, '0');
        auto startTime = std::chrono::steady_clock::now();
        bool found = false;
        int nonceCounter = 0;
        std::atomic<bool> shouldExit{false};  // Atomic flag for coordinating thread exit
        
        #pragma omp parallel
        {
            std::string localBlockHash;
            int localNonce;
            
            // Each thread gets its own nonce range
            #pragma omp critical
            {
                localNonce = nonceCounter;
                nonceCounter += 1000000;  // Increment by a large step to give each thread its own range
            }
            
            while (true) {
                // Check time limit
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(
                    currentTime - startTime).count();
                
                if (elapsedTime >= timeLimit) {
                    shouldExit = true;
                    break;
                }
                if (shouldExit) {
                    break;
                }
                if (found) {
                    break;
                }
                // Create block data with current nonce
                std::string data = previousHash + merkleRoot +
                                std::to_string(localNonce) +
                                std::to_string(std::chrono::system_clock::to_time_t(timestamp));
                
                // Generate and check hash
                localBlockHash = hasher.generateHash(data);
                if (localBlockHash.substr(0, difficulty) == target) {
                    #pragma omp critical
                    {
                        if (!found) {
                            found = true;
                            nonce = localNonce;
                            blockHash = localBlockHash;
                            shouldExit = true;  // Signal other threads to exit
                        }
                    }
                    break;
                }
                
                localNonce++;
                
                if (localNonce % 1000000 == 0) {
                    #pragma omp critical
                    {
                        localNonce = nonceCounter;
                        nonceCounter += 1000000;
                    }
                }
            }
            // auto endTime = std::chrono::steady_clock::now();
            // auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
            // std::cout << elapsedTime ;
        }

        return found;
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
    Blockchain(int diff = 5) : difficulty(diff) {
        // Create genesis block
        Block genesisBlock("0", 0);
        genesisBlock.mineBlock(difficulty, 1);
        chain.push_back(genesisBlock);
    }

    void createUsers(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(100.0, 10000.0);

        for (int i = 0; i < count; i++) {
            std::string name = "User" + std::to_string(i);
            std::string publicKey = generatePublicKey();
            users.push_back(User(name, publicKey));

            for (int j = 0; j < 15; j++) {
                double initialBalance = distr(gen);
                UTXO genesisUtxo(chain[0].getHash(), utxoPool.size(), initialBalance, publicKey);
                utxoPool.push_back(genesisUtxo);
            }
        }
        std::cout << count << " users created with initial UTXOs\n";
    }

    void generateTransactions(int count) {
        if (users.size() < 2) return;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> userDistr(0, users.size() - 1);
        std::uniform_real_distribution<> amountDistr(1.0, 1000.0);

        std::unordered_map<std::string, std::vector<UTXO>> availableUtxos;

        for (const auto& utxo : utxoPool) {
            availableUtxos[utxo.ownerKey].push_back(utxo);
        }
        

        for (int i = 0; i < count; i++) {
            int senderIdx = userDistr(gen);
            while (availableUtxos[users[senderIdx].getPublicKey()].empty()) {
                senderIdx = userDistr(gen);
            }
            int receiverIdx = userDistr(gen);
            while (receiverIdx == senderIdx) receiverIdx = userDistr(gen);

            double amount = amountDistr(gen);
            const std::string& senderKey = users[senderIdx].getPublicKey();
            const std::string& receiverKey = users[receiverIdx].getPublicKey();

            double totalAvailable = 0.0;
            for (const auto& utxo : availableUtxos[senderKey]) {
                totalAvailable += utxo.amount;
            }

            if (totalAvailable >= amount) {
                std::vector<UTXO> selectedInputs;
                double totalInput = 0.0;
                int outputIndex = 0;
  
                auto& senderUtxos = availableUtxos[senderKey];
                for (auto it = senderUtxos.begin(); it != senderUtxos.end() && totalInput < amount; ) {
                    selectedInputs.push_back(*it);
                    totalInput += it->amount;
                    it = senderUtxos.erase(it);
                }
                
                std::vector<UTXO> outputs;
                outputs.emplace_back("", outputIndex++, amount, receiverKey);
                
                double change = totalInput - amount;
                if (change > 0) {
                    outputs.emplace_back("", outputIndex++, change, senderKey);
                }
                
                Transaction tx(selectedInputs, outputs);
                pendingTransactions.push_back(tx);
            }
            else if (isAvailableUtxos(availableUtxos)) i--;
        }
        std::cout << pendingTransactions.size() << " transactions generated\n";
    }
    
    bool isAvailableUtxos(std::unordered_map<std::string, std::vector<UTXO>>& availableUtxos){
        int count = 0, unavailable = 0 ;
        for (const auto& utxo : availableUtxos) {
            count++;
            if (utxo.second.empty()) {
                unavailable++;
            }
        }
        return count == unavailable ? false : true;
    }

    void updateUTXOPool(const Transaction& tx) {
        for (const auto& input : tx.getInputs()) {
            utxoPool.erase(
                std::remove_if(utxoPool.begin(), utxoPool.end(),
                    [&input](const UTXO& utxo) {
                        return utxo.transactionId == input.transactionId && 
                               utxo.outputIndex == input.outputIndex;
                    }), 
                utxoPool.end());
        }
        utxoPool.insert(utxoPool.end(), tx.getOutputs().begin(), tx.getOutputs().end());
    }

    void mineNextBlock() {
        if (pendingTransactions.empty()) return;

        // Create 5 candidate blocks
        std::vector<Block> candidates;
        std::random_device rd;
        std::mt19937 gen(rd());

        for (int i = 0; i < 5; i++) {
            Block candidate(chain.back().getHash(), chain.size());
            
            // Select ~100 random transactions
            std::vector<int> indices(pendingTransactions.size());
            std::iota(indices.begin(), indices.end(), 0);
            std::shuffle(indices.begin(), indices.end(), gen);
            
            int txCount = std::min(100, static_cast<int>(pendingTransactions.size()));
            for (int j = 0; j < txCount; j++) {
                // cout << j << " " << indices[j] << endl;
                const Transaction& tx = pendingTransactions[indices[j]];
                if (tx.verifyTransaction(utxoPool)) {
                    candidate.addTransaction(tx);
                }
            }
            
            candidates.push_back(std::move(candidate));
        }
        // Try mining each candidate
        for (auto& candidate : candidates) {
            std::cout << "Attempting to mine candidate block...\n";
            if (candidate.mineBlock(difficulty, 5)) {
                // Update UTXO pool with the mined transactions
                const auto& transactions = candidate.getTransactions();
                for (const auto& tx : transactions) {
                    updateUTXOPool(tx);
                }

                // Remove mined transactions from pending pool
                for (const auto& tx : transactions) {
                    pendingTransactions.erase(
                        std::remove_if(pendingTransactions.begin(), pendingTransactions.end(),
                            [&tx](const Transaction& t) { return t.getId() == tx.getId(); }),
                        pendingTransactions.end());
                }

                chain.push_back(candidate);
                candidate.printBlock();
                std::cout << "Block successfully mined!\n";
                return;
            }
        }
        
        std::cout << "Failed to mine any candidate blocks. Increasing mining time...\n";
        for (auto& candidate : candidates) {
            if (candidate.mineBlock(difficulty, 10)) {
                chain.push_back(candidate);
                candidate.printBlock();
                std::cout << "Block successfully mined with increased time!\n";
                return;
            }
        }
        
        std::cout << "Failed to mine block even with increased time.\n";
    }

    void printUTXOPoolInfo() const {
        std::cout << "\n=== UTXO Pool Info ===\n";
        std::cout << "Total UTXOs: " << utxoPool.size() << "\n";
        double totalValue = 0.0;
        std::map<std::string, double> balances;
        
        for (const auto& utxo : utxoPool) {
            totalValue += utxo.amount;
        }
        
        std::cout << "Total value in UTXO pool: " << totalValue << "\n";
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

    void printTransactionInfo(const std::string& transactionId) const {
        for (const auto& block : chain) {
            for (const auto& transaction : block.getTransactions()) {
                if (transaction.getId() == transactionId) {
                    transaction.printInfo();
                    return;
                }
            }
        }
        std::cout << "Transaction not found.\n";
    }

    void printBlockInfo(int blockIndex) const {
        if (blockIndex < 0 || blockIndex >= chain.size()) {
            std::cout << "Block index out of range.\n";
            return;
        }
        chain[blockIndex].printBlock();
    }
};

int main() {
    std::ios::sync_with_stdio(false);

    Blockchain blockchain(5); 

    // Generate initial users and transactions
    blockchain.createUsers(1000);
    blockchain.generateTransactions(10000);
    blockchain.printChainInfo();

    while (true) {
        std::string command;
        std::cout << "\nEnter command (mine/mine_all/info/balances/utxo/new_user <number>/new_transaction <number>/transaction <transactionID>/block <blockIdx>/exit): ";
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
        else if (command == "utxo") {
            blockchain.printUTXOPoolInfo();
        }
        else if (command == "transaction") {
            std::string transactionId;
            std::cin >> transactionId;
            blockchain.printTransactionInfo(transactionId);
        }
        else if (command == "block") {
            int blockIndex;
            std::cin >> blockIndex;
            blockchain.printBlockInfo(blockIndex);
        }
        else if (command == "exit") {
            break;
        }
    }

    return 0;
}
