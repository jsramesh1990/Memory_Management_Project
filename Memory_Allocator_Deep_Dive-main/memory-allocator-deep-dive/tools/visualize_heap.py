#!/usr/bin/env python3
"""
Heap visualization tool for memory allocator
Parses allocator_dump output and creates visual representation
"""

import sys
import re

def parse_heap_dump(filename):
    """Parse heap dump output from allocator_dump_heap()"""
    blocks = []
    with open(filename, 'r') as f:
        for line in f:
            match = re.search(r'Block \d+: addr=([^,]+), size=(\d+), (\w+), magic=0x([0-9A-F]+)', line)
            if match:
                blocks.append({
                    'addr': match.group(1),
                    'size': int(match.group(2)),
                    'state': match.group(3),
                    'magic': match.group(4)
                })
    return blocks

def visualize(blocks, width=80):
    """Create ASCII visualization of heap"""
    if not blocks:
        return
    
    total_size = sum(b['size'] for b in blocks)
    scale = width / total_size if total_size > 0 else 1
    
    print("\n" + "=" * width)
    print("HEAP MEMORY MAP")
    print("=" * width)
    
    for block in blocks:
        block_width = max(1, int(block['size'] * scale))
        char = '#' if block['state'] == 'ALLOCATED' else '.'
        state = block['state'][0]
        print(f"{char * block_width}  [{block['state']}] {block['size']} bytes")
    
    print("=" * width)
    print("Legend: # = Allocated, . = Free")
    print(f"Total: {total_size} bytes")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: visualize_heap.py <heap_dump_file>")
        sys.exit(1)
    
    blocks = parse_heap_dump(sys.argv[1])
    visualize(blocks)
