Bounding Half-Space Hierarchy
=============================

The Bounding Volume Hierarchy (BVH) is a popular spatial data structure. It is a tree of volumes, each node of which fully encloses the volumes of its child nodes. If you would like to know which objects in a BVH might intersect some volume of interest, you can do an intersection test against a BVH node, and if that test fails, there is no need to perform tests against the children of that node, since their intersection tests are guaranteed to fail.

BVH has many nice properties and is commonly used in ray tracing and real-time computer graphics, such as in video games.

One property that BVH does not have, however, is that it is not possible to express a BVH as the ordering of objects in an array.

That is to say that, if you have an array of objects, and you wish to sort them for sublinear search, BVH can not do this, unless there is also an auxiliary data structure, other than the array of objects itself.

There is, however, a spatial data structure that can sort an array of objects for sublinear search without requiring any auxiliary data structure, and it is called Bounding Half-Space Hierarchy (BHH).

In other words, with BHH it is possible to “sort” a 1D array of 3D objects, “in 3D,” for efficient search “in 3D.”

A K-D Tree of Points is a BHH
-----------------------------

A K-D Tree of points is equivalent to a BHH for objects with zero dimensions (i.e. points.) It is well known that an array of points can be sorted as a K-D Tree, and can be searched in sublinear time, without requiring any data structure other than the array itself. It is not well-known that an array of 3D objects can likewise be sorted as a BHH.

Half-Space
----------

A closed half-space is a plane plus all of the space on one side of the plane. A bounding box is the intersection of six half-spaces, and a bounding tetrahedron is the intersection of four. For any set of half-spaces whose intersection encloses space, and for any array of objects, if there is a function f that produces the minimum value occupied by the object in each half-space, then we can use BHH to sort the array of objects for sublinear search:

Let’s work in two dimensions, since it’s simpler and extends trivially to higher dimensions. 

How to Construct a BHH
----------------------

![triangles](/triangles.png)

Above we have an array of 15 triangle structures [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14] where each structure contains the three values [minX, minY, -(maxX+maxY)]. In 2D any 3 or more axes will do*, but we chose these for convenience.

It is possible to sort the entire array by increasing value of minX. Then, we can consider the median triangle (#7) to be done. 

Next, we sort each of the two arrays [0,1,2,3,4,5,6] and [8,9,10,11,12,13,14] by minY. Then, we can consider their median triangles (#3 and #11) to be done. 

Next, we can sort each of the four arrays [0,1,2] and [4,5,6] and [8,9,10] and [12,13,14] by -(maxX+maxY). Then we can consider their median triangles (#1 and #5 and #9 and #13) to be done. 

If we try to sort the arrays on either side of the newly-done medians, we find that each contains only one item, and so there is no more sorting to do.

How to Search with a BHH
------------------------

To search the above BHH, we need a triangle structure with the three values [maxX, maxY, -(minX+minY)]. This must be the [dual](https://en.wikipedia.org/wiki/Dual_polyhedron)** of the triangles we sorted.

![triangles](/triangle.png)

First, compare the search maxX with the minX of triangle #7. If the search maxX is smaller than triangle #7’s minX, then triangles [7,8,9,10,11,12,13,14] can’t possibly intersect the search triangle, because all of their minX are guaranteed to be >= the minX of triangle #7. 

Then compare the search maxY with the minY of triangle #3. If the search maxX is smaller than triangle #3’s minX, then triangles [3,4,5,6] can’t possibly intersect the search triangle, because all of their minY are guaranteed to be >= the minY of triangle #3. 

Then compare the search -(minX+minY) with the -(maxX+maxY) of triangle #1. If the search -(minX+minY) is smaller than triangle #1’s -(maxX+maxY), then triangles [1,2] can’t possibly intersect the search triangle, because all of their -(maxX+maxY) are guaranteed to be >= the -(maxX+maxY) of triangle #1.

Possible To Sort Arrays of Circles, Spheres, Et Cetera
------------------------------------------------------

The array of objects to sort can contain any shape, as long as you can cheaply derive half-spaces from it.

For example, if you had an array of circles, each with the structure [x, y, radius]:

![circles](/circles.png)

It is possible to cheaply derive bounding triangles from them on the fly:

```
minX = x-radius
maxX = x+radius
minY = y-radius
maxY = y+radius

{minX, minY, -(maxX+maxY)} // for each circle in the array
{maxX, maxY, -(minX+minY)} // for the one circle you test them against
```

And perform the same sort & search algorithm as above, despite the fact that the array contains only circles, and the various minX minY values are never stored in memory.

---

* As long as all semicircles contain at least one axis
* Not exactly a dual polyhedron. What is it called when one convex polytope's planes are all the opposite from anothers'?
