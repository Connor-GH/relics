# Goals
This file talks about the goals of the project. Above all, this project hopes to
achieve a performant kernel and thin userspace.

Multiple approaches are in
consideration currently, but I would like to experiment with process isolation
in the kernel, rather than relying on a userspace, as the performance gains
would be extreme.

Additionally, I would like to implement a kernel-level garbage
collector for the libraryOSes to use. Memory allocation will be fast, albeit a
little ballooned in terms of size, but memory safety will be easier for both the
kernel developer and userspace developer. For those who do not want to use a
garbage collector, an alternate backend will be developed that fits more of the
traditional "malloc-free" idiom.
