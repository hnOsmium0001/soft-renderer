# Design Decisions
## Use of primitives over wrapper objects
You will find that conventions like `Vector3f` and `Color` objects are rarely
used within the code (especially in lower level constructs like in the triangle rasterizer).
This is to keep the heap allocation overhead low, and more importantly, reduce indirections
which could severely reduce performance.

Note, however, that in tests you probably will find using objects or not doesn't make a
significant difference in cache locality, because the "active working area" will be small enough
for our example tests such that most of the data fit into the cache even if we allocate small objects.