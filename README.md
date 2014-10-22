mesa-mesa
=========

Repo with changes to build with Android-x86
Forked from unofficial mirror of freedesktop mesa repository, which is updated daily.

There are two commits in the github repo:

1) Gallium nouveu source corrections and main format_info.h header file
Explanation: I could not build format.c because of format_info.c (generated fragment of code), because Android buildchain wants to compile the fragment, which is not possible, I simply renamed the generated fragment as format_info.h and changed the #include in format.c file accordingly. NOTE: C by the book suggestion is to name code fragments as .h as best practice.

2) Android makefiles changes to build 10.3.1
Explanation: mainly Android make files includes paths and generation of format_info.h instead of format_info.c accordingly to previous commit.

In order to build mesa 10.3.1 with Android-x86:

- replace external/drm with libdrm 2.4.58

- if you want to try experimental drm_gralloc_nouveau and add nouveau in device/generic/x86/BoardConfig.mk, replace hardware/drm_gralloc, using one of following gits:

https://github.com/evelikov/hardware_drm_gralloc or https://github.com/pstglia/drm_gralloc

- revert commits that introduced tr1/unordered_set, because STLport does not support all types in its own tr1/unordered_set implementation and gnu_stl, libc++ or boost libs integration not yet successful.

git revert fb4e23626f9f27c62aa1cbea05a1c8b088fcf49a [committed on 27-Set-2014]

git revert bb06f2cd93e8e16798bd59b458abd8d33a8e4658 [committed on 08-Sep-2014]

git revert eaa9e14ce5a3e69c6aa303f2e5f039c1cc0b8de1 [committed on 05-Sep-2014]

git revert a9b21015f5e3a6a37e53a8b3c755519f7b70479e [committed on 08-Jul-2014]


Limitations:

- gallium r300, r600 can build and seem to be working on HD4xxx and HD5xxx, but still not extensively tested
- gallium radeonsi building not yet achieved for Android-x86
- gallium nouveau can be built, provided that tr1 code is reverted, because of STLport lack of supporting size_t
- drm_gralloc_nouveau is still unstable apparently due to buffer objects invalid addresses and/or GPU locking, missing GPU registers initialization code.
- intel i915, i965 classic drivers not working (GUI is there, but "rendered in black"). ilo working GUI, but to be tested with OpenGL ES, while i915g gallium driver is still veeery slow.
 
Any feedback/suggestion is appreciated expecially on tr1/unordered_set compiling with current AOSP toolchain or Android NDK latest GCC versions.

Mauro
