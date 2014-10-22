mesa-mesa
=========

ATTENTION THIS IS A NON WORKING WIP

Repo with changes to build with Android-x86
Forked from unofficial mirror of freedesktop mesa repository which is updated daily.

In order to build mesa 10.4.0 with Android-x86: ATTENTION THIS IS A NON WORKING WIP

- replace external/drm with libdrm 2.4.58

- if you want to try experimental drm_gralloc_nouveau and add nouveau in device/generic/x86/BoardConfig.mk, replace hardware/drm_gralloc, using one of following gits:

https://github.com/evelikov/hardware_drm_gralloc or https://github.com/pstglia/drm_gralloc

- revert commits that introduced tr1/unordered_set, because STLport does not support all types in its own tr1/unordered_set implementation and gnu_stl, libc++ or boost libs integration not yet successful.

git revert b4df9d99604201d1d9924c91df9fbbcee269f828   [committed on 26-Sep-2014]

git revert ca9ab05d45ebf407485af2daa3742b897ff99162   [committed on 06-Sep-2014]

git revert c4bb436f7660c951cd27e52660cf825da68793e5   [committed on 01-Set-2014]

git revert a9b21015f5e3a6a37e53a8b3c755519f7b70479e   [committed on 08-Jul-2014]


Limitations:

- gallium r300, r600 can build and seem to be working on HD4xxx and HD5xxx, but still not extensively tested
- gallium radeonsi building not yet achieved for Android-x86
- gallium nouveau can be built, provided that tr1 code is reverted, because of STLport lack of supporting size_t
- drm_gralloc_nouveau is still unstable apparently due to buffer objects invalid addresses and/or GPU locking, missing GPU registers initialization code.
- intel i915, i965 classic drivers not working (GUI is there, but "rendered in black"). ilo working GUI, but to be tested with OpenGL ES, while i915g gallium driver is still veeery slow.
 
Any feedback/suggestion is appreciated expecially on tr1/unordered_set compiling support with current AOSP toolchain, or Android NDK latest GCC versions.

Mauro
