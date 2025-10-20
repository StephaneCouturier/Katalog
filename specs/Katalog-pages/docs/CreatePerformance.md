# Katalog Performance Guide

## What Affects Scanning Speed?

### 1. Metadata Extraction (Biggest Impact: ~10× slowdown)
- Image metadata: ~2-3ms/file (reads header)
- Video metadata: ~5-15ms/file (seeks, parses container)
- Solution: Use "Media Basic" only, not "Full Extended"

### 2. Database Mode
- Memory mode: Faster, uses RAM
- SQLite File mode: Slower, I/O bound
- Recommendation: Use Memory for development

### 3. Storage Type
- SSD: ~100K files/min
- HDD: ~20-30K files/min (fragmentation matters)
- Network storage: Highly variable

### 4. Excluded Folders
- More exclusions = faster scanning
- Example: Exclude .cache, node_modules, etc.

### 5. System Load
- Parallel extraction uses 4-8 cores
- Other heavy processes will interfere

## Performance Benchmarks

| Files | Storage | Metadata | Time |
|-------|---------|----------|------|
| 5K | SSD | None | 10s |
| 5K | SSD | Basic | 50s |
| 95K | HDD | Basic | 47s (1st) / 10s (cached) |

## Tuning Tips

1. For first scan: Disable metadata
2. For rescans: Enable incremental scanning
3. For large drives: Use filter rules
4. Monitor: Settings > Diagnostics > Performance
