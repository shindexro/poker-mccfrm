# Poker MCCFRM

A high-performance implementation of Monte Carlo Counterfactual Regret Minimization (MCCFR) for multi-player poker, featuring information abstraction and parallel training capabilities.

## Overview

This project implements MCCFR with various optimizations for training competitive poker AI agents. It supports:
- 6-player poker games (configurable)
- Information abstraction using OCHS and EMD bucketing
- Multi-threaded training with Intel TBB
- Interactive gameplay against trained AI

## Features

- **Monte Carlo CFR Algorithm**: Efficient sampling-based CFR implementation with regret pruning
- **Information Abstraction**: 
  - Preflop: 169 isomorphic buckets
  - Postflop: 200 buckets per street using K-means clustering
  - OCHS (Opponent Cluster Hand Strength) for river
  - EMD (Earth Mover's Distance) for flop/turn
- **Optimizations**:
  - Hand isomorphism via indexed representation
  - Parallel hash maps for concurrent strategy updates
  - Buffered node map updates for performance
  - Thread-local deck instances
- **Cloud Deployment**: Terraform configuration for AWS training infrastructure

## Requirements

- C++20 compatible compiler
- CMake 3.14+
- Boost libraries (serialization, archive)
- Intel TBB
- Google Test (automatically fetched)

## Building

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Usage

### Training

Run the training executable to start MCCFR training:

```bash
./build/src/binary/main train
```

### Playing

To play interactively against the trained AI:

```bash
./build/src/binary/main play
```

## Project Structure

```
├── src/
│   ├── binary/         # Main executable
│   ├── abstraction/    # Game state abstractions
│   ├── algorithm/      # MCCFR implementation
│   ├── game/          # Poker game mechanics
│   ├── tables/        # Precomputed abstraction tables
│   ├── enums/         # Game enumerations
│   └── utils/         # Utility functions
├── tests/             # Unit tests
├── terraform/         # AWS deployment configuration
└── external/          # Third-party libraries
```

## Configuration

Game parameters can be modified in `src/abstraction/global.cpp`:
- Number of players: 6
- Starting chips: 10,000
- Big blind: 100
- Small blind: 50
- Rake: 2%

## Algorithm Details

The implementation uses:
- **Regret Matching**: With pruning threshold of -300,000,000
- **Strategy Updates**: Discounting factor applied periodically
- **Average Strategy**: Tracked only for preflop (other rounds use current strategy)
- **Sampling**: External sampling MCCFR variant

## Testing

Run unit tests:

```bash
./build/tests/unit_tests
```

## Cloud Training

Deploy training infrastructure on AWS:

```bash
cd terraform
terraform init
terraform apply
```

This sets up an r6a.xlarge EC2 instance optimized for memory-intensive MCCFR training.

## License

[Add your license information here]

## References

- Lanctot et al., "Monte Carlo Sampling for Regret Minimization in Extensive Games"
- Johanson et al., "Accelerating Best Response Calculation in Large Extensive Games"