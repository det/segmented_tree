# What is segmented_tree_seq?

The segmented_tree_seq container supports the interfaces of vector and deque while offering efficient random access insert and erase with a low memory overhead.

# When should I use segmented_tree_seq?

You should use segmented_tree_seq when you are frequently calling insert or erase in random positions of your container and you don't need stable iterators. A good example would be a text editor buffer.

# How is segmented_tree_seq implemented?

The segmented_tree_seq container is implemented using a counted [B+Tree](https://en.wikipedia.org/wiki/B%2B_tree). Internally the tree consists of 3 types of nodes: segments, leaves, and branches. The bottom level of the tree consists of segments. Segments are simply arrays of the container's value_type. They contain no metadata. Above this exists leaves. Leaves are index nodes and store a size and a pointer for each of its child segments. They also contain meta data of parent pointer, parent index and number of children. All levels above this are branches. Branches are the same as leaves but can contain either other branches or leaves. Because segments do not store metadata it is neccesary to store the their parent pointer, parent index and length in the iterators. As inserting/erasing/splitting/merging happens exponentially less often as you travel up the tree, removing the metadata from the bottom level has proved to be a worthwhile optimization by removing the cache misses associated with loading and storing of this data.

# How does segmented_tree_seq compare to similar data structures?

## Vs avl_array

Pros:

 * Significantly faster in nearly every operation.
 * Uses significantly less memory.
 * Provides constant time comparison of iterators.

Cons:

 * Does not provide stable iterators.

## Vs btree_seq

Pros:

 * Slightly faster single insert/erase.
 * Slightly faster iteration.
 * Supports the C++11 allocator model.
 * Provides eager iterators[1].
 * Uses memcpy/memmove for trivial types[2].
 * Automatically caluclates segment/leaf/branch lengths[3].

Cons:

 * The implementation of range insert/erase is significantly slower[4].
 * Requires the user to use the nth() member function for best performance[1].

[1] Iterators in btree_seq are lazy, which is not what you would expect when using a C++ container. Obtaining an iterator to the nth element is constant time and will only do the work of traversing the tree when dereferenced. The advantage of this is that calling begin() + offset as you would with a vector suffers no performance penalty. The iterator must also restart from the root of the tree when crossing leaf boundaries. I've observed that btree_seq is up to 20x slower when calling std::sort or using reverse_iterators.

[2] Using memcpy/memove is important when operating on 8 bit values as allows you to use optimal segment sizes which leads to greater performance.

[3] Rather than passing the sizes of nodes to segmented_tree_seq, you pass the target size in bytes and then the lengths are caluclated to fit inside that.

[4] Range insert/erase in segmented_tree_seq currently calls single insert/erase multiple times which performs a lot of duplicate work of moving data around and is very slow.

# Additional work

In the future the following things could be implemented: optimized range insert/erase, splice and merge from the std::list interface, and additional indexes.

# More information

[Class documentation](https://det.github.io/segmented_tree_seq/classboost_1_1container_1_1segmented__tree__seq.html)

[Benchmarks](https://github.com/det/segmented_tree_seq/tree/benchmarks)
