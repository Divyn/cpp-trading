#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <cmath>
#include <algorithm>

// Liquidity Position - represents liquidity provided in a price range
struct LiquidityPosition {
    int id;
    double price_lower;      // Lower bound of price range
    double price_upper;      // Upper bound of price range
    double amount_token0;   // Amount of token0 (e.g., USDC)
    double amount_token1;   // Amount of token1 (e.g., BTC)
    double fees_earned;      // Fees earned from trades in this range
    
    LiquidityPosition(int i, double lower, double upper, double token0, double token1)
        : id(i), price_lower(lower), price_upper(upper), 
          amount_token0(token0), amount_token1(token1), fees_earned(0.0) {}
};

// Trade event in the pool
struct Trade {
    double price;
    double amount;
    bool is_buy;  // true = buying token1, false = selling token1
    double fee_paid;
    
    Trade(double p, double amt, bool buy, double fee)
        : price(p), amount(amt), is_buy(buy), fee_paid(fee) {}
};

// DEX Market Maker - Provides liquidity in price bands (like Uniswap V3)
class DEXMarketMaker {
private:
    // Current price of the pool
    double current_price = 50000.0;
    
    // Liquidity positions (price bands)
    std::vector<LiquidityPosition> positions;
    
    // Fee rate (e.g., 0.003 = 0.3% for Uniswap V3 0.3% pool)
    double fee_rate = 0.003;
    
    // Total fees earned across all positions
    double total_fees_earned = 0.0;
    
    // Track trades for fee distribution
    std::vector<Trade> trade_history;
    
    // Check if a price is within a liquidity position's range
    bool isPriceInRange(double price, const LiquidityPosition& pos) const {
        return price >= pos.price_lower && price <= pos.price_upper;
    }
    
    // Calculate liquidity distribution in a price range
    // In Uniswap V3, liquidity is distributed based on the constant product formula
    // For simplicity, we'll use a linear distribution
    void calculateLiquidityDistribution(LiquidityPosition& pos, double price) {
        if (!isPriceInRange(price, pos)) {
            // Price outside range - all liquidity in one token
            if (price < pos.price_lower) {
                // Price below range - all in token1 (BTC)
                pos.amount_token0 = 0.0;
                pos.amount_token1 = pos.amount_token0 + pos.amount_token1 * pos.price_lower;
            } else {
                // Price above range - all in token0 (USDC)
                pos.amount_token1 = 0.0;
                pos.amount_token0 = pos.amount_token0 + pos.amount_token1 * pos.price_upper;
            }
        } else {
            // Price in range - liquidity in both tokens
            double range_size = pos.price_upper - pos.price_lower;
            double price_from_lower = price - pos.price_lower;
            double ratio = price_from_lower / range_size;
            
            // Simplified: distribute based on position in range
            double total_value = pos.amount_token0 + pos.amount_token1 * price;
            pos.amount_token0 = total_value * (1.0 - ratio);
            pos.amount_token1 = (total_value - pos.amount_token0) / price;
        }
    }
    
    // Distribute fees to liquidity providers in the active price range
    void distributeFees(double fee_amount, double price) {
        // Find all positions that contain this price
        for (auto& pos : positions) {
            if (isPriceInRange(price, pos)) {
                // Calculate this position's share of liquidity
                double position_liquidity = pos.amount_token0 + pos.amount_token1 * price;
                
                // For simplicity, distribute fees proportionally
                // In reality, Uniswap V3 uses more complex calculations
                double total_active_liquidity = 0.0;
                for (const auto& p : positions) {
                    if (isPriceInRange(price, p)) {
                        total_active_liquidity += p.amount_token0 + p.amount_token1 * price;
                    }
                }
                
                if (total_active_liquidity > 0) {
                    double share = position_liquidity / total_active_liquidity;
                    pos.fees_earned += fee_amount * share;
                    total_fees_earned += fee_amount * share;
                }
            }
        }
    }
    
public:
    // Add liquidity to a price band
    void addLiquidity(double price_lower, double price_upper, 
                     double amount_token0, double amount_token1) {
        int new_id = positions.size() + 1;
        LiquidityPosition pos(new_id, price_lower, price_upper, amount_token0, amount_token1);
        
        // Calculate initial liquidity distribution at current price
        calculateLiquidityDistribution(pos, current_price);
        
        positions.push_back(pos);
        
        std::cout << "  [ADD LIQUIDITY] Position #" << new_id 
                  << ": Range $" << std::fixed << std::setprecision(2) 
                  << price_lower << " - $" << price_upper
                  << " | Token0: " << amount_token0 
                  << " | Token1: " << amount_token1 << "\n";
    }
    
    // Remove liquidity from a position
    void removeLiquidity(int position_id) {
        auto it = std::remove_if(positions.begin(), positions.end(),
            [position_id](const LiquidityPosition& pos) {
                return pos.id == position_id;
            });
        
        if (it != positions.end()) {
            std::cout << "  [REMOVE LIQUIDITY] Position #" << position_id << " removed\n";
            positions.erase(it, positions.end());
        }
    }
    
    // Simulate a trade in the pool (updates price and distributes fees)
    void executeTrade(double amount, bool is_buy) {
        // Simple price impact model (in reality, uses constant product formula)
        double price_impact = amount * 0.0001;  // 0.01% per unit
        double new_price;
        
        if (is_buy) {
            // Buying token1 pushes price up
            new_price = current_price * (1.0 + price_impact);
        } else {
            // Selling token1 pushes price down
            new_price = current_price * (1.0 - price_impact);
        }
        
        // Calculate fee
        double trade_value = amount * current_price;
        double fee = trade_value * fee_rate;
        
        // Record trade
        Trade trade(current_price, amount, is_buy, fee);
        trade_history.push_back(trade);
        
        // Distribute fees to active liquidity positions (pass fee amount, not trade amount)
        distributeFees(fee, current_price);
        
        // Update price
        current_price = new_price;
        
        // Update liquidity distribution for all positions
        for (auto& pos : positions) {
            calculateLiquidityDistribution(pos, current_price);
        }
        
        std::cout << "  [TRADE] " << (is_buy ? "BUY" : "SELL") 
                  << " " << amount << " @ $" << std::fixed << std::setprecision(2)
                  << current_price << " | Fee: $" << fee << "\n";
    }
    
    // Update current price (from external market data)
    void updatePrice(double new_price) {
        current_price = new_price;
        
        // Recalculate liquidity distribution
        for (auto& pos : positions) {
            calculateLiquidityDistribution(pos, current_price);
        }
    }
    
    // Display all liquidity positions
    void printPositions() const {
        std::cout << "\n=== LIQUIDITY POSITIONS ===\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Current Pool Price: $" << current_price << "\n\n";
        
        if (positions.empty()) {
            std::cout << "No active positions\n";
            return;
        }
        
        for (const auto& pos : positions) {
            bool is_active = isPriceInRange(current_price, pos);
            
            std::cout << "Position #" << pos.id 
                      << " [" << (is_active ? "ACTIVE" : "INACTIVE") << "]\n";
            std::cout << "  Price Range: $" << pos.price_lower 
                      << " - $" << pos.price_upper << "\n";
            std::cout << "  Token0 (USDC): " << pos.amount_token0 << "\n";
            std::cout << "  Token1 (BTC): " << pos.amount_token1 << "\n";
            std::cout << "  Fees Earned: $" << pos.fees_earned << "\n";
            
            if (is_active) {
                double position_value = pos.amount_token0 + pos.amount_token1 * current_price;
                std::cout << "  Current Value: $" << position_value << "\n";
            }
            std::cout << "\n";
        }
    }
    
    // Display summary statistics
    void printSummary() const {
        std::cout << "\n=== MARKET MAKER SUMMARY ===\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Current Price: $" << current_price << "\n";
        std::cout << "Total Positions: " << positions.size() << "\n";
        std::cout << "Total Fees Earned: $" << total_fees_earned << "\n";
        std::cout << "Total Trades: " << trade_history.size() << "\n";
        
        // Count active positions
        int active_count = 0;
        for (const auto& pos : positions) {
            if (isPriceInRange(current_price, pos)) {
                active_count++;
            }
        }
        std::cout << "Active Positions: " << active_count << "\n";
    }
    
    double getCurrentPrice() const { return current_price; }
    int getPositionCount() const { return positions.size(); }
    double getTotalFees() const { return total_fees_earned; }
};

int main() {
    std::cout << "=== DEX MARKET MAKING SIMULATION ===\n";
    std::cout << "Simulating liquidity provision in price bands (Uniswap V3 style)\n\n";
    
    DEXMarketMaker mm;
    
    // Market maker adds liquidity at different price bands
    std::cout << "=== ADDING LIQUIDITY POSITIONS ===\n";
    
    // Position 1: Wide range around current price
    mm.addLiquidity(49000.0, 51000.0, 10000.0, 0.0);  // $49k-$51k, mostly USDC
    
    // Position 2: Narrow range below current price
    mm.addLiquidity(49500.0, 50000.0, 5000.0, 0.0);   // $49.5k-$50k
    
    // Position 3: Narrow range above current price
    mm.addLiquidity(50000.0, 50500.0, 0.0, 0.1);      // $50k-$50.5k, mostly BTC
    
    // Position 4: Very wide range (passive liquidity)
    mm.addLiquidity(45000.0, 55000.0, 20000.0, 0.0);  // $45k-$55k
    
    mm.printPositions();
    
    // Simulate trades
    std::cout << "\n=== SIMULATING TRADES ===\n";
    
    // Trade 1: Small buy
    std::cout << "\n[TRADE 1]\n";
    mm.executeTrade(0.05, true);  // Buy 0.05 BTC
    mm.printPositions();
    
    // Trade 2: Medium sell
    std::cout << "\n[TRADE 2]\n";
    mm.executeTrade(0.1, false);  // Sell 0.1 BTC
    mm.printPositions();
    
    // Trade 3: Large buy
    std::cout << "\n[TRADE 3]\n";
    mm.executeTrade(0.2, true);  // Buy 0.2 BTC
    mm.printPositions();
    
    // Simulate price movement (external market move)
    std::cout << "\n=== EXTERNAL PRICE MOVEMENT ===\n";
    std::cout << "Market price moves to $51,500\n";
    mm.updatePrice(51500.0);
    mm.printPositions();
    
    // More trades at new price level
    std::cout << "\n[TRADE 4]\n";
    mm.executeTrade(0.15, false);  // Sell 0.15 BTC
    mm.printPositions();
    
    // Final summary
    mm.printSummary();
    
    std::cout << "\n=== KEY CONCEPTS ===\n";
    std::cout << "1. Liquidity is provided in specific price bands/ranges\n";
    std::cout << "2. Only positions with current price in their range earn fees\n";
    std::cout << "3. As price moves, different positions become active/inactive\n";
    std::cout << "4. Fees are distributed proportionally to active liquidity\n";
    std::cout << "5. Narrower ranges = higher capital efficiency but more rebalancing\n";
    
    return 0;
}

