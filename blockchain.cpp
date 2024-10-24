#include <bits/stdc++.h>
#include "hash.h" 

class User {
private:
    std::string name;
    std::string publicKey;
    double balance;

public:
    User(const std::string& name, const std::string& publicKey, double balance)
        : name(name), publicKey(publicKey), balance(balance) {}

    std::string getPublicKey() const { return publicKey; }
    double getBalance() const { return balance; }
    void setBalance(double newBalance) { balance = newBalance; }
    std::string getName() const { return name; }
};

class Transaction {
private:
    std::string transactionId;
    std::string senderKey;
    std::string receiverKey;
    double amount;
    std::chrono::system_clock::time_point timestamp;

public:
    Transaction(const std::string& sender, const std::string& receiver, double amount)
        : senderKey(sender), receiverKey(receiver), amount(amount) {
        timestamp = std::chrono::system_clock::now();
        generateTransactionId();
    }

    void generateTransactionId() {
        MyHash hasher;
        std::string data = senderKey + receiverKey + std::to_string(amount) + 
                          std::to_string(std::chrono::system_clock::to_time_t(timestamp));
        transactionId = hasher.generateHash(data);
    }

    std::string getId() const { return transactionId; }
    std::string getSender() const { return senderKey; }
    std::string getReceiver() const { return receiverKey; }
    double getAmount() const { return amount; }
};

public:
    MerkleTree() : root(nullptr) {}
    ~MerkleTree() { delete root; }

    void buildTree(const std::vector<Transaction>& transactions) {
        std::vector<std::string> txHashes;
        for (const auto& tx : transactions) {
            txHashes.push_back(tx.getId());
        }
        delete root;
        root = buildTree(txHashes);
    }

    std::string getRootHash() const {
        return root ? root->hash : "";
    }
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
        std::cout << "\n=== Block #" << blockHeight << " ===\n";
        std::cout << "Hash: " << blockHash << "\n";
        std::cout << "Previous Hash: " << previousHash << "\n";
        std::cout << "Merkle Root: " << merkleRoot << "\n";
        std::cout << "Nonce: " << nonce << "\n";
        std::cout << "Timestamp: " << std::chrono::system_clock::to_time_t(timestamp) << "\n";
        std::cout << "Transaction count: " << transactions.size() << "\n";
        
        for (const auto& tx : transactions) {
            std::cout << "\nTransaction ID: " << tx.getId() << "\n";
            std::cout << "From: " << tx.getSender() << "\n";
            std::cout << "To: " << tx.getReceiver() << "\n";
            std::cout << "Amount: " << tx.getAmount() << "\n";
        }
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
    
    std::string generatePublicKey() {
        MyHash hasher;
        static int counter = 0;
        return hasher.generateHash("user" + std::to_string(counter++));
    }

public:
    Blockchain(int diff = 4) : difficulty(diff) {
        // Create genesis block
        Block genesisBlock("0", 0);
        genesisBlock.mineBlock(difficulty);
        chain.push_back(genesisBlock);
    }

    void createUsers(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(100.0, 1000000.0);

        for (int i = 0; i < count; i++) {
            std::string name = "User" + std::to_string(i);
            std::string publicKey = generatePublicKey();
            double balance = distr(gen);
            users.push_back(User(name, publicKey, balance));
        }
        std::cout << count << " users created\n";
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
            if (users[senderIdx].getBalance() >= amount) {
                Transaction tx(users[senderIdx].getPublicKey(), 
                             users[receiverIdx].getPublicKey(), 
                             amount);
                pendingTransactions.push_back(tx);
            }
        }
        std::cout << pendingTransactions.size() << " transactions generated\n";
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
                auto senderIt = std::find_if(users.begin(), users.end(),
                    [&tx](const User& u) { return u.getPublicKey() == tx.getSender(); });
                auto receiverIt = std::find_if(users.begin(), users.end(),
                    [&tx](const User& u) { return u.getPublicKey() == tx.getReceiver(); });

                if (senderIt != users.end() && receiverIt != users.end()) {
                    senderIt->setBalance(senderIt->getBalance() - tx.getAmount());
                    receiverIt->setBalance(receiverIt->getBalance() + tx.getAmount());
                }
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
            std::cout << user.getName() << " (" << user.getPublicKey().substr(0, 8) << "...): " 
                     << std::fixed << std::setprecision(2) << user.getBalance() << "\n";
        }
    }
};

int main() {
    Blockchain blockchain(4); // Difficulty = 4 leading zeros

    // Generate initial users and transactions
    blockchain.createUsers(1000);
    blockchain.generateTransactions(10000);
    blockchain.printChainInfo();

    // Mine blocks until no pending transactions
    while (true) {
        std::string command;
        std::cout << "\nEnter command (mine/info/balances/exit): ";
        std::cin >> command;

        if (command == "mine") {
            blockchain.mineNextBlock();
        }
        else if (command == "info") {
            blockchain.printChainInfo();
        }
        else if (command == "balances") {
            blockchain.printUserBalances();
        }
        else if (command == "exit") {
            break;
        }
    }

    return 0;
}