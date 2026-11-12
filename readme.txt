TIPE (L2) class project, 2024.

The goal is to find the contracting map of the image space whose attractor is as
close as possible to the desired image.

To do this, the image is segmented into a partition of blocks of side length n
then, for each block in the partition, we search for the block of side length 2n
at an arbitrary position in the image that most closely resembles it after
decimation and linear regression.

The mapping constructed by the algorithm then consists of replacing each of
these “small blocks” with its decimated and renormalized “large block.” Since
the mapping is constructed to preserve the image as much as possible while
remaining contracting, its attractor must be close to the initial image.

It is then sufficient to reapply the transformation while retaining each of its
coefficients to achieve compression; decompression consists of iterating from
any image.

The code does not rely on any libraries for educational purposes and is very
ugly.

