# magewell-capture-rs

This is a Rust crate for interacting with Magewell capture cards. It is a wrapper around the Magewell SDK, which is vendored in this repo and linked statically.

## Dependencies

The default build of this crate requires the following libraries to be installed:

- udev
- asound
- v4l2

Alternatively, you can enable the `no-deps` feature to compile non-functional stub versions of these dependencies into the library. This results in a binary that has no additional runtime dependencies on shared libraries, but will not be able to perform certain functions such as interact with USB devices.
